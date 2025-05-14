#pragma once


#ifndef LOGIC_HPP
#define LOGIC_HPP


#include <cstdint>
#include <time.h>
#include <sys/time.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "channels.hpp"
#include "camera.hpp"
#include "music.hpp"


namespace logic {


bool is_pose_correct();


static uint8_t hours = 0;
static uint8_t minutes = 0;
const int32_t delay_time_sec = 10;
const char* tag = "logic";


void main(void* arg) {
  bool flag = false;
  TaskHandle_t music_task_handle = nullptr;

  while (true) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    if ((timeinfo.tm_hour != hours) || (timeinfo.tm_min != minutes)) {
      flag = false;
      ESP_LOGI(tag, "Time is %d:%d", timeinfo.tm_hour, timeinfo.tm_min);
      vTaskDelay(delay_time_sec * 1000 / portTICK_PERIOD_MS);
      
      int32_t code = 0;
      while (xQueueReceive(channels::server_status_channel, static_cast<void*>(&code), 100 / portTICK_PERIOD_MS) == pdTRUE);
    }

    if (!flag && (timeinfo.tm_hour == hours) && (timeinfo.tm_min == minutes)) {
      ESP_LOGI(tag, "Start taking photo");
      flag = true;
      cam::start_photo();
      xTaskCreate(music::main, "music::main", 2048, nullptr, 1, &music_task_handle);
    }

    if (is_pose_correct()) {
      ESP_LOGI(tag, "Stop taking photo");
      cam::stop_photo();
      if (music_task_handle != nullptr) {
        vTaskDelete(music_task_handle);
      }
      music_task_handle = nullptr;
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


void change_time(uint8_t new_hours, uint8_t new_minutes) {
  hours = new_hours;
  minutes = new_minutes;
}


bool is_pose_correct() {
  int code = 0;
  xQueueReceive(channels::server_status_channel, static_cast<void*>(&code), 100 / portTICK_PERIOD_MS);
  return code == 200;
}


}


#endif // !LOGIC_HPP
