#include <vector>
#include <cstdint>
#include <memory>

#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "http.hpp"
#include "camera.hpp"
#include "bluetooth.hpp"
#include "sntp.hpp"


extern "C" void app_main(void) {
  const char* tag = "main";

  constexpr std::uint64_t requests_queue_size = 10;
  http::Requests requests_queue(requests_queue_size);

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());

  sntp::main();

  TaskHandle_t http_task_handle;
  TaskHandle_t cam_task_handle;
  TaskHandle_t ble_task_handle;
  TaskHandle_t sntp_task_handle;
  
  xTaskCreate(http::main, "http::main", 8096, static_cast<void*>(&requests_queue), 1, &http_task_handle);
  ESP_LOGI(tag, "Created http task");

  xTaskCreate(cam::main, "cam::main", 8096, static_cast<void*>(&requests_queue), 1, &cam_task_handle);
  ESP_LOGI(tag, "Created camera task");

  xTaskCreate(ble::main, "ble::main", 4096, nullptr, 1, &ble_task_handle);
  ESP_LOGI(tag, "Created bluetooth task");
  
  while (true) vTaskDelay(portMAX_DELAY);

  vTaskDelete(http_task_handle);
  vTaskDelete(cam_task_handle);
  vTaskDelete(ble_task_handle);
  vTaskDelete(sntp_task_handle);
  vTaskDelete(nullptr);
}
