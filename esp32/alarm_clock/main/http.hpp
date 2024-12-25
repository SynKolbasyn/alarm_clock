#pragma once


#ifndef HTTP_HPP
#define HTTP_HPP


#include <memory>
#include <shared_mutex>
#include <queue>
#include <cstdint>
#include <string>
#include <vector>
#include <expected>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "protocol_examples_common.h"


namespace http {


enum http_error {
  dns_lookup_failed,
  allocate_socket_failed,
  socket_connect_failed,
  socket_send_failed,
  set_socket_recv_timeout_failed
};


in_addr* dns_lookup(const char* tag, addrinfo** dns_result, const std::string& server_address, const std::string& server_port);
int create_socket(const char* tag, addrinfo* dns_result);
bool socket_connect(const char* tag, int sock, addrinfo* dns_result);
bool socket_write(const char* tag, int sock, const std::vector<std::uint8_t>& request);
bool set_socket_timeout(const char* tag, int sock);
std::vector<std::uint8_t> socket_read(const char* tag, int sock);
std::expected<std::vector<std::uint8_t>, http_error> send_request(const char* tag, const std::string& server_address, const std::string& server_port, const std::vector<std::uint8_t>& data);


void main(void* arg) {
  QueueHandle_t* requests_queue = static_cast<QueueHandle_t*>(arg);

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());

  const char* tag = "http";

  const std::string server_address = "192.168.1.17";
  const std::string server_port = "80";

  while (true) {
    std::vector<std::uint8_t> data;
    if (xQueueReceive(*requests_queue, static_cast<void*>(&data), portMAX_DELAY) != pdTRUE) {
      ESP_LOGE(tag, "can't receive item from requests queue");
      continue;
    }
    std::expected<std::vector<std::uint8_t>, http_error> request_result = send_request(tag, server_address, server_port, data);
    if (request_result.has_value()) ESP_LOGI(tag, "request result size: \n%zu", request_result->size());
  }
}


std::expected<std::vector<std::uint8_t>, http_error> send_request(const char* tag, const std::string& server_address, const std::string& server_port, const std::vector<std::uint8_t>& data) {
  addrinfo* dns_result;
  in_addr* addr = dns_lookup(tag, &dns_result, server_address, server_port);
  if (addr == nullptr) return std::unexpected(dns_lookup_failed);

  int sock = create_socket(tag, dns_result);
  if (sock < 0) return std::unexpected(allocate_socket_failed);

  if (!socket_connect(tag, sock, dns_result)) return std::unexpected(socket_connect_failed);

  if (!socket_write(tag, sock, data)) return std::unexpected(socket_send_failed);

  if (!set_socket_timeout(tag, sock)) return std::unexpected(set_socket_recv_timeout_failed);

  return socket_read(tag, sock);
}


in_addr* dns_lookup(const char* tag, addrinfo** dns_result, const std::string& server_address, const std::string& server_port) {
  const addrinfo hints = {
    .ai_family = AF_INET,
    .ai_socktype = SOCK_STREAM,
  };
  int dns_return = getaddrinfo(server_address.c_str(), server_port.c_str(), &hints, &*dns_result);

  if(dns_return != 0 || *dns_result == NULL) {
    ESP_LOGE(tag, "DNS lookup failed | err: %d res: %p", dns_return, *dns_result);
    return nullptr;
  }
  in_addr* addr = &((sockaddr_in*)(*dns_result)->ai_addr)->sin_addr;
  ESP_LOGI(tag, "DNS lookup succeeded | IP: %s", inet_ntoa(*addr));
  return addr;
}


int create_socket(const char* tag, addrinfo* dns_result) {
  int sock = socket(dns_result->ai_family, dns_result->ai_socktype, 0);
  if(sock < 0) {
    ESP_LOGE(tag, "failed to allocate socket");
    freeaddrinfo(dns_result);
    return sock;
  }
  ESP_LOGI(tag, "allocated socket");
  return sock;
}


bool socket_connect(const char* tag, int sock, addrinfo* dns_result) {
  if(connect(sock, dns_result->ai_addr, dns_result->ai_addrlen) != 0) {
    ESP_LOGE(tag, "socket connect failed | errno: %d", errno);
    close(sock);
    freeaddrinfo(dns_result);
    return false;
  }

  ESP_LOGI(tag, "connected");
  freeaddrinfo(dns_result);

  return true;
}


bool socket_write(const char* tag, int sock, const std::vector<std::uint8_t>& request) {
  if (write(sock, static_cast<const void*>(&request.front()), request.size()) < 0) {
    ESP_LOGE(tag, "socket send failed");
    close(sock);
    return false;
  }
  ESP_LOGI(tag, "socket send success");
  return true;
}


bool set_socket_timeout(const char* tag, int sock) {
  timeval receiving_timeout;
  receiving_timeout.tv_sec = 5;
  receiving_timeout.tv_usec = 0;
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0) {
    ESP_LOGE(tag, "failed to set socket receiving timeout");
    close(sock);
    return false;
  }
  ESP_LOGI(tag, "set socket receiving timeout success");
  return true;
}


std::vector<std::uint8_t> socket_read(const char* tag, int sock) {
  std::vector<std::uint8_t> result;

  while (true) {
    constexpr std::uint16_t buffer_size = 2048;
    std::uint8_t buffer[buffer_size];
    int read_size = read(sock, buffer, buffer_size);

    if ((0 < read_size) && (read_size <= buffer_size)) {
      result = std::vector<std::uint8_t>(buffer, buffer + read_size);
      break;
    }
    else ESP_LOGE(tag, "buffer overflowed");
  }

  ESP_LOGI(tag, "done reading from socket | return: %d errno: %d", result.size(), errno);
  close(sock);

  return result;
}


} // namespace http


#endif // !HTTP_HPP
