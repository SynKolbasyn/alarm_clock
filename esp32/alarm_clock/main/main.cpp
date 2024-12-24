#include <string>
#include <cstdint>
#include <memory>
#include <limits>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "http.hpp"
#include "camera.hpp"


extern "C" void app_main(void) {
  const char* tag = "main";

  std::shared_ptr<http::Requests> requests(new http::Requests);

  TaskHandle_t http_task_handle;
  TaskHandle_t cam_task_handle;
  
  xTaskCreate(http::main, "http::main", 4096, static_cast<void*>(&requests), 1, &http_task_handle);
  ESP_LOGI(tag, "Created http task");

  xTaskCreate(cam::main, "cam::main", 8192, static_cast<void*>(&requests), 1, &cam_task_handle);
  ESP_LOGI(tag, "Created camera task");
  
  while (true) vTaskDelay(std::numeric_limits<std::uint32_t>::max());

  vTaskDelete(http_task_handle);
  vTaskDelete(cam_task_handle);
  vTaskDelete(nullptr);
}
