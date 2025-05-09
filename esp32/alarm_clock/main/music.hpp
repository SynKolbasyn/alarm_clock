#pragma once

#ifndef _MUSIC_HPP_
#define _MUSIC_HPP_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2s_std.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"
#include "driver/sdmmc_host.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"


namespace music {


static const char* TAG = "music";

// SD Card (SPI)
#define MOUNT_POINT "/sdcard"
#define SPI_HOST    SPI2_HOST
#define SPI_CS      GPIO_NUM_5
#define SPI_MOSI    GPIO_NUM_23
#define SPI_MISO    GPIO_NUM_19
#define SPI_SCLK    GPIO_NUM_18

// I2S
#define I2S_MCLK    GPIO_NUM_0
#define I2S_BCLK    GPIO_NUM_1
#define I2S_WS      GPIO_NUM_2
#define I2S_DOUT    GPIO_NUM_3


struct WAVHeader {
    uint32_t sample_rate;
    uint16_t bits_per_sample;
    uint16_t num_channels;
    uint32_t data_offset;
};


esp_err_t mount_sdcard() {
    ESP_LOGI(TAG, "Initializing SD card over SPI");

    // Конфигурация SPI шины
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MISO,
        .sclk_io_num = SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = 4092,
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .intr_flags = 0
    };

    // Инициализация SPI шины
    esp_err_t ret = spi_bus_initialize(SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // Конфигурация SD-карты
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SPI_CS;
    slot_config.host_id = SPI_HOST;

    // Настройка подтяжки CS
    gpio_set_pull_mode(SPI_CS, GPIO_PULLUP_ONLY);

    // Параметры монтирования
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 3,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t* card;
    ret = esp_vfs_fat_sdspi_mount(
        MOUNT_POINT,
        &host,
        &slot_config,
        &mount_config,
        &card
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
        spi_bus_free(SPI_HOST);
    }
    return ret;
}


esp_err_t parse_wav_header(FILE* f, WAVHeader* header)
{
    uint8_t buf[44];
    if (fread(buf, 1, sizeof(buf), f) != sizeof(buf)) {
        ESP_LOGE(TAG, "Failed to read WAV header");
        return ESP_FAIL;
    }

    if (memcmp(buf, "RIFF", 4) != 0 || memcmp(&buf[8], "WAVE", 4) != 0) {
        ESP_LOGE(TAG, "Not a valid WAV file");
        return ESP_FAIL;
    }

    header->num_channels     = buf[22] | (buf[23] << 8);
    header->sample_rate      = buf[24] | (buf[25] << 8) | (buf[26] << 16) | (buf[27] << 24);
    header->bits_per_sample  = buf[34] | (buf[35] << 8);
    header->data_offset = 44; // Обычно так

    ESP_LOGI(TAG, "WAV: %ld Hz, %d bit, %d channels",
             header->sample_rate, header->bits_per_sample, header->num_channels);
    return ESP_OK;
}

esp_err_t init_i2s(const WAVHeader& header, i2s_chan_handle_t* out_tx_chan)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, out_tx_chan, NULL));

    i2s_data_bit_width_t bit_width;
    switch (header.bits_per_sample) {
        case 16: bit_width = I2S_DATA_BIT_WIDTH_16BIT; break;
        case 24: bit_width = I2S_DATA_BIT_WIDTH_24BIT; break;
        case 32: bit_width = I2S_DATA_BIT_WIDTH_32BIT; break;
        default:
            ESP_LOGE(TAG, "Unsupported bit depth");
            return ESP_ERR_NOT_SUPPORTED;
    }

    i2s_slot_mode_t mode = (header.num_channels == 1)
        ? I2S_SLOT_MODE_MONO
        : I2S_SLOT_MODE_STEREO;

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(header.sample_rate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(bit_width, mode),
        .gpio_cfg = {
            .mclk = I2S_MCLK,
            .bclk = I2S_BCLK,
            .ws   = I2S_WS,
            .dout = I2S_DOUT,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            }
        }
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(*out_tx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(*out_tx_chan));
    return ESP_OK;
}


void play_wav_file(const char* path, const WAVHeader& header, i2s_chan_handle_t tx_chan)
{
    FILE* f = fopen(path, "rb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file: %s", path);
        return;
    }

    fseek(f, header.data_offset, SEEK_SET);

    uint8_t buffer[512];
    size_t bytes_read, bytes_written;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        i2s_channel_write(tx_chan, buffer, bytes_read, &bytes_written, portMAX_DELAY);
    }

    fclose(f);
    ESP_LOGI(TAG, "Playback finished.");
}


/*
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting WAV Player");

    if (mount_sdcard() != ESP_OK) return;

    const char* wav_path = MOUNT_POINT "/music.wav";
    FILE* f = fopen(wav_path, "rb");
    if (!f) {
        ESP_LOGE(TAG, "Cannot open file: %s", wav_path);
        return;
    }

    WAVHeader header;
    if (parse_wav_header(f, &header) != ESP_OK) {
        fclose(f);
        return;
    }
    fclose(f);

    i2s_chan_handle_t tx_chan;
    if (init_i2s(header, &tx_chan) != ESP_OK) return;

    play_wav_file(wav_path, header, tx_chan);

    i2s_channel_disable(tx_chan);
    i2s_del_channel(tx_chan);
}
*/

void init() {
    while (mount_sdcard() != ESP_OK) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void main(void* args) {
    const char* wav_path = MOUNT_POINT "/music.wav";
    FILE* f = fopen(wav_path, "rb");
    if (!f) {
        ESP_LOGE(TAG, "Cannot open file: %s", wav_path);
        return;
    }

    WAVHeader header;
    if (parse_wav_header(f, &header) != ESP_OK) {
        fclose(f);
        return;
    }
    fclose(f);

    i2s_chan_handle_t tx_chan;
    if (init_i2s(header, &tx_chan) != ESP_OK) return;

    play_wav_file(wav_path, header, tx_chan);

    i2s_channel_disable(tx_chan);
    i2s_del_channel(tx_chan);
}


} // namespace music


#endif // !_MUSIC_HPP_
