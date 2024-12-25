#include <vector>
#include <cstdint>
#include <memory>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "http.hpp"
#include "camera.hpp"


extern "C" void app_main(void) {
  const char* tag = "main";

  QueueHandle_t requests_queue = xQueueCreate(10, sizeof(std::vector<std::uint8_t>));

  TaskHandle_t http_task_handle;
  TaskHandle_t cam_task_handle;
  
  xTaskCreate(http::main, "http::main", 4096, static_cast<void*>(&requests_queue), 1, &http_task_handle);
  ESP_LOGI(tag, "Created http task");

  xTaskCreate(cam::main, "cam::main", 8192, static_cast<void*>(&requests_queue), 1, &cam_task_handle);
  ESP_LOGI(tag, "Created camera task");
  
  while (true) vTaskDelay(portMAX_DELAY);
  
  vQueueDelete(requests_queue);

  vTaskDelete(http_task_handle);
  vTaskDelete(cam_task_handle);
  vTaskDelete(nullptr);
}
