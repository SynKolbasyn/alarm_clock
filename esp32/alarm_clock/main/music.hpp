#pragma once

#ifndef _MUSIC_HPP_
#define _MUSIC_HPP_


#include "driver/ledc.h"
#include "esp_err.h"

#define LEDC_TIMER              LEDC_TIMER_1
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (42) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_1
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz


namespace music {


static const char* TAG = "music";
constexpr uint32_t freqs[] = {147, 147, 147, 147, 147, 147, 147, 147, 147, 147, 147, 147, 147, 147, 147, 147, 220, 262, 294, 294, 294, 330, 349, 349, 349, 392, 330, 330, 294, 262, 262, 294, 220, 262, 294, 294, 294, 330, 349, 349, 349, 392, 330, 330, 294, 262, 294, 220, 262, 294, 294, 294, 349, 392, 392, 392, 440, 466, 466, 440, 392, 440, 294, 294, 330, 349, 349, 392, 440, 294, 294, 349, 330, 330, 349, 294, 330};
constexpr int32_t durs[] = {429, 214, 429, 214, 429, 214, 214, 214, 214, 429, 214, 429, 214, 429, 214, 214, 214, 214, 429, 429, 214, 214, 429, 429, 214, 214, 429, 429, 214, 214, 214, 429, 214, 214, 429, 429, 214, 214, 429, 429, 214, 214, 429, 429, 214, 214, 857, 214, 214, 429, 429, 214, 214, 429, 429, 214, 214, 429, 429, 214, 214, 214, 643, 214, 214, 429, 429, 429, 214, 429, 214, 214, 429, 429, 214, 214, 643};
constexpr int32_t music_size = 383;
bool flag = false;


void play_note(uint32_t freq) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num       = LEDC_TIMER,
        .freq_hz         = freq,  // Set output frequency at 4 kHz
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}


void init() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num       = LEDC_TIMER,
        .freq_hz         = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num   = LEDC_OUTPUT_IO,
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER,
        .duty       = 0,  // Установить скважность 0%
        .hpoint     = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

    ESP_LOGI(TAG, "Music initialized");
}


void main(void* args) {
    while (true) {
        for (int32_t i = 0; i < music_size; ++i) {
            if (!flag) {
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
                break;
            }
            ESP_LOGI(TAG, "Music playing");
            play_note(freqs[i]);
            vTaskDelay(pdMS_TO_TICKS(durs[i]));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void start() {
    flag = true;
}


void stop() {
    flag = false;
}


} // namespace music


#endif // !_MUSIC_HPP_
