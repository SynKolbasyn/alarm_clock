#pragma once


#ifndef SNTP_HPP
#define SNTP_HPP


namespace sntp {


#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"

#include "esp_netif_sntp.h"
#include "lwip/ip_addr.h"
#include "esp_sntp.h"

static const char *TAG = "example";


static void obtain_time();


void time_sync_notification_cb(struct timeval *tv) {
  ESP_LOGI(TAG, "Notification of a time synchronization event");
}


void init() {
  while (true) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900)) {
      ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
      obtain_time();
      time(&now);
    }
    else break;

    char strftime_buf[64];

    setenv("TZ", "RDT-3", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Moscow is: %s", strftime_buf);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


static void print_servers() {
  ESP_LOGI(TAG, "List of configured NTP servers:");

  for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i){
    if (esp_sntp_getservername(i)){
      ESP_LOGI(TAG, "server %d: %s", i, esp_sntp_getservername(i));
    }
    else {
      char buff[INET6_ADDRSTRLEN];
      ip_addr_t const *ip = esp_sntp_getserver(i);
      if (ipaddr_ntoa_r(ip, buff, INET6_ADDRSTRLEN) != NULL) {
        ESP_LOGI(TAG, "server %d: %s", i, buff);
      }
    }
  }
}


static void obtain_time() {
  ESP_LOGI(TAG, "Initializing and starting SNTP");
  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
  config.sync_cb = time_sync_notification_cb;

  esp_netif_sntp_init(&config);

  print_servers();

  time_t now = 0;
  struct tm timeinfo = { 0 };
  int retry = 0;
  const int retry_count = 15;
  while (esp_netif_sntp_sync_wait(5000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry <= retry_count) {
    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
  }
  time(&now);
  localtime_r(&now, &timeinfo);

  esp_netif_sntp_deinit();
}


} // namespace sntp


#endif // !SNTP_HPP
