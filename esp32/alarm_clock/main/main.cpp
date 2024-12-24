#include <string>
#include <cstdint>
#include <memory>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "http.hpp"


extern "C" void app_main(void) {
  const char* tag = "main";

  std::shared_ptr<http::Requests> requests(new http::Requests);

  TaskHandle_t http_task_handle;
  xTaskCreate(http::main, "http::main", 4096, static_cast<void*>(&requests), 1, &http_task_handle);

  std::uint64_t count = 0;
  while (true) {
    requests->push(std::to_string(count++));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  vTaskDelete(http_task_handle);
  vTaskDelete(nullptr);
}
