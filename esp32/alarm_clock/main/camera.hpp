#pragma once


#ifndef CAMERA_HPP
#define CAMERA_HPP


#include <vector>
#include <memory>

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>

#include "esp_camera.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "http.hpp"


namespace cam {


esp_err_t init_camera();


void main(void* arg) {
  http::Requests* requests_queue = static_cast<http::Requests*>(arg);

  const char* tag = "cam";

  while (init_camera() != ESP_OK) {
    ESP_LOGE(tag, "Camera init failed");
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }

  while (true) {
    camera_fb_t* pic = esp_camera_fb_get();

    ESP_LOGI(tag, "Picture maked | (%d, %d): %zu bytes", pic->width, pic->height, pic->len);
    std::vector<std::uint8_t> data(pic->buf, pic->buf + pic->len);
    requests_queue->push(data);

    esp_camera_fb_return(pic);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


esp_err_t init_camera() {
  constexpr std::int8_t cam_pin_pwdn = 38;
  constexpr std::int8_t cam_pin_reset = -1;
  constexpr std::int8_t cam_pin_vsync = 6;
  constexpr std::int8_t cam_pin_href = 7;
  constexpr std::int8_t cam_pin_pclk = 13;
  constexpr std::int8_t cam_pin_xclk = 15;
  constexpr std::int8_t cam_pin_siod = 4;
  constexpr std::int8_t cam_pin_sioc = 5;
  constexpr std::int8_t cam_pin_d0 = 11;
  constexpr std::int8_t cam_pin_d1 = 9;
  constexpr std::int8_t cam_pin_d2 = 8;
  constexpr std::int8_t cam_pin_d3 = 10;
  constexpr std::int8_t cam_pin_d4 = 12;
  constexpr std::int8_t cam_pin_d5 = 18;
  constexpr std::int8_t cam_pin_d6 = 17;
  constexpr std::int8_t cam_pin_d7 = 16;

  camera_config_t camera_config = {
    .pin_pwdn = cam_pin_pwdn,
    .pin_reset = cam_pin_reset,
    .pin_xclk = cam_pin_xclk,
    .pin_sccb_sda = cam_pin_siod,
    .pin_sccb_scl = cam_pin_sioc,

    .pin_d7 = cam_pin_d7,
    .pin_d6 = cam_pin_d6,
    .pin_d5 = cam_pin_d5,
    .pin_d4 = cam_pin_d4,
    .pin_d3 = cam_pin_d3,
    .pin_d2 = cam_pin_d2,
    .pin_d1 = cam_pin_d1,
    .pin_d0 = cam_pin_d0,
    .pin_vsync = cam_pin_vsync,
    .pin_href = cam_pin_href,
    .pin_pclk = cam_pin_pclk,

    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QVGA,

    .jpeg_quality = 6,
    .fb_count = 2,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
  };

  return esp_camera_init(&camera_config);
}


}


#endif // !CAMERA_HPP
