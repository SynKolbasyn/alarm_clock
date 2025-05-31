#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "storage.hpp"
#include "wifi.hpp"
#include "http.hpp"
#include "camera.hpp"
#include "bluetooth.hpp"
#include "sntp.hpp"
#include "channels.hpp"
#include "logic.hpp"
#include "music.hpp"


extern "C" void app_main(void) {
  const char* tag = "main";

  TaskHandle_t http_task_handle;
  TaskHandle_t cam_task_handle;
  TaskHandle_t ble_task_handle;
  TaskHandle_t logic_task_handle;
  TaskHandle_t music_task_handle;

  storage::init();
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  xTaskCreate(ble::main, "ble::main", 8096, nullptr, 1, &ble_task_handle);
  ESP_LOGI(tag, "Created bluetooth task");

  wifi::init();
  sntp::init();
  channels::init();
  music::init();
  
  xTaskCreate(http::main, "http::main", 8096, nullptr, 1, &http_task_handle);
  ESP_LOGI(tag, "Created http task");

  xTaskCreate(logic::main, "logic::main", 8096, nullptr, 1, &logic_task_handle);
  ESP_LOGI(tag, "Created logic task");

  xTaskCreate(cam::main, "cam::main", 8096, nullptr, 1, &cam_task_handle);
  ESP_LOGI(tag, "Created camera task");

  xTaskCreate(music::main, "music::main", 2048, nullptr, 1, &music_task_handle);
  ESP_LOGI(tag, "Created music task");
  
  while (true) vTaskDelay(portMAX_DELAY);

  vTaskDelete(logic_task_handle);
  vTaskDelete(http_task_handle);
  vTaskDelete(cam_task_handle);
  vTaskDelete(ble_task_handle);
  vTaskDelete(music_task_handle);
  vTaskDelete(nullptr);
}
