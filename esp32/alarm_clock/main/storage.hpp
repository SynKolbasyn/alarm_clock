#pragma once


#ifndef STORAGE_HPP
#define STORAGE_HPP


#include <expected>

#include "nvs_flash.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "esp_system.h"


namespace storage {


enum StorageError {
  WRITE_OK,
  FAILED_TO_READ_VALUE,
  FAILED_TO_WRITE_VALUE,
};


static const char* tag = "storage";
static std::unique_ptr<nvs::NVSHandle> handle;


void init() {
  ESP_LOGI(tag, "Initializing storage");

  esp_err_t err = nvs_flash_init();
  if ((err == ESP_ERR_NVS_NO_FREE_PAGES) || (err == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  handle = nvs::open_nvs_handle("storage", NVS_READWRITE, &err);
  if (err != ESP_OK) {
    ESP_LOGE(tag, "Storage init failed: %s", esp_err_to_name(err));
    ESP_ERROR_CHECK(err);
  }

  ESP_LOGI(tag, "Storage initialization complete");
}


template<typename T> std::expected<T, StorageError> load(const std::string& key) {
  T value;
  esp_err_t res = handle->get_item(key.c_str(), value);
  if (res != ESP_OK) return std::unexpected(FAILED_TO_READ_VALUE);
  return value;
}


template<typename T> std::expected<T, StorageError> load(const std::string&& key) {
  T value;
  esp_err_t res = handle->get_item(key.c_str(), value);
  if (res != ESP_OK) return std::unexpected(FAILED_TO_READ_VALUE);
  return value;
}


template<typename T> StorageError save(const std::string& key, T& value) {
  esp_err_t res = handle->set_item(key.c_str(), value);
  if (res != ESP_OK) return FAILED_TO_WRITE_VALUE;
  return WRITE_OK;
}


template<typename T> StorageError save(const std::string&& key, T& value) {
  esp_err_t res = handle->set_item(key.c_str(), value);
  if (res != ESP_OK) return FAILED_TO_WRITE_VALUE;
  return WRITE_OK;
}


}


#endif // !STORAGE_HPP
