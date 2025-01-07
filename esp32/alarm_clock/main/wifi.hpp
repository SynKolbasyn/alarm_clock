#pragma once


#ifndef WIFI_HPP
#define WIFI_HPP


#include <string.h>

#include "esp_log.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"


namespace wifi {


bool is_our_netif(const char* prefix, esp_netif_t* netif);
static void handler_on_wifi_disconnect(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void handler_on_wifi_connect(void* esp_netif, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void handler_on_sta_got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void start();
void stop();
esp_err_t sta_do_connect(wifi_config_t wifi_config, bool wait);
esp_err_t sta_do_disconnect();
esp_err_t connect();
void shutdown();


static const char* tag = "wifi";
static esp_netif_t *s_example_sta_netif = NULL;
static SemaphoreHandle_t s_semph_get_ip_addrs = NULL;
static int s_retry_num = 0;


void init() {
  while (connect() != ESP_OK) ESP_LOGE(tag, "CAN'T CONNECT TO WIFI, TRYING AGAIN");
  ESP_LOGI(tag, "SUCCSESSFULY WIFI CONNECTION");
}


esp_err_t connect() {
  ESP_LOGI(tag, "Start example_connect.");
  start();
  wifi_scan_threshold_t threshold = {
    .rssi = CONFIG_EXAMPLE_WIFI_SCAN_RSSI_THRESHOLD,
    .authmode = WIFI_AUTH_OPEN,
  };
  wifi_config_t wifi_config = {
    .sta = {
      .ssid = CONFIG_EXAMPLE_WIFI_SSID,
      .password = CONFIG_EXAMPLE_WIFI_PASSWORD,
      .scan_method = WIFI_ALL_CHANNEL_SCAN,
      .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
      .threshold = threshold,
    },
  };

  return sta_do_connect(wifi_config, true);
}


void shutdown() {
  sta_do_disconnect();
  stop();
}


void start() {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
  esp_netif_config.if_desc = "example_netif_sta";
  esp_netif_config.route_prio = 128;
  s_example_sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
  esp_wifi_set_default_wifi_sta_handlers();

  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
}


void stop() {
  esp_err_t err = esp_wifi_stop();
  if (err == ESP_ERR_WIFI_NOT_INIT) {
    return;
  }

  ESP_ERROR_CHECK(err);
  ESP_ERROR_CHECK(esp_wifi_deinit());
  ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(s_example_sta_netif));
  esp_netif_destroy(s_example_sta_netif);
  s_example_sta_netif = NULL;
}


esp_err_t sta_do_connect(wifi_config_t wifi_config, bool wait) {
  if (wait) {
    s_semph_get_ip_addrs = xSemaphoreCreateBinary();
    if (s_semph_get_ip_addrs == NULL) {
      return ESP_ERR_NO_MEM;
    }
  }

  s_retry_num = 0;
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &handler_on_wifi_disconnect, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &handler_on_sta_got_ip, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &handler_on_wifi_connect, s_example_sta_netif));

  ESP_LOGI(tag, "Connecting to %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  esp_err_t ret = esp_wifi_connect();

  if (ret != ESP_OK) {
    ESP_LOGE(tag, "WiFi connect failed! ret:%x", ret);
    return ret;
  }

  if (wait) {
    ESP_LOGI(tag, "Waiting for IP(s)");
    xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
    if (s_retry_num > CONFIG_EXAMPLE_WIFI_CONN_MAX_RETRY) {
      return ESP_FAIL;
    }
  }

  return ESP_OK;
}

esp_err_t sta_do_disconnect() {
  ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &handler_on_wifi_disconnect));
  ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &handler_on_sta_got_ip));
  ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &handler_on_wifi_connect));
  
  if (s_semph_get_ip_addrs) {
    vSemaphoreDelete(s_semph_get_ip_addrs);
  }

  return esp_wifi_disconnect();
}


static void handler_on_wifi_disconnect(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  s_retry_num++;
  if (s_retry_num > CONFIG_EXAMPLE_WIFI_CONN_MAX_RETRY) {
    ESP_LOGI(tag, "WiFi Connect failed %d times, stop reconnect.", s_retry_num);
    if (s_semph_get_ip_addrs) {
      xSemaphoreGive(s_semph_get_ip_addrs);
    }
    sta_do_disconnect();
    return;
  }

  wifi_event_sta_disconnected_t* disconn = static_cast<wifi_event_sta_disconnected_t*>(event_data);
  if (disconn->reason == WIFI_REASON_ROAMING) {
    ESP_LOGD(tag, "station roaming, do nothing");
    return;
  }

  ESP_LOGI(tag, "Wi-Fi disconnected %d, trying to reconnect...", disconn->reason);
  esp_err_t err = esp_wifi_connect();
  if (err == ESP_ERR_WIFI_NOT_STARTED) {
    return;
  }

  ESP_ERROR_CHECK(err);
}

static void handler_on_wifi_connect(void* esp_netif, esp_event_base_t event_base, int32_t event_id, void* event_data) {

}

static void handler_on_sta_got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  s_retry_num = 0;
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
  if (!is_our_netif("example_netif_sta", event->esp_netif)) {
    return;
  }

  ESP_LOGI(tag, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
  if (s_semph_get_ip_addrs) {
    xSemaphoreGive(s_semph_get_ip_addrs);
  }
  else {
    ESP_LOGI(tag, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
  }
}


bool is_our_netif(const char* prefix, esp_netif_t* netif) {
  return strncmp(prefix, esp_netif_get_desc(netif), strlen(prefix) - 1) == 0;
}


} // namespace wifi


#endif // WIFI_HPP
