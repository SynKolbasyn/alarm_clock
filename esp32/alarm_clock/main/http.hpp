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

#include "channels.hpp"


namespace http {


enum http_error {
  dns_lookup_failed,
  allocate_socket_failed,
  socket_connect_failed,
  socket_send_failed,
  set_socket_recv_timeout_failed
};


addrinfo* dns_lookup(const std::string& server_address, const std::string& server_port);
int create_socket(addrinfo* dns_result);
bool socket_connect(int sock, addrinfo* dns_result);
std::string create_request(const std::string& host, const std::vector<std::uint8_t>& data);
bool socket_write(int sock, const std::string& host, const std::vector<std::uint8_t>& request);
bool set_socket_timeout(int sock);
std::string socket_read(int sock);
std::expected<std::string, http_error> send_request(const std::string& server_address, const std::string& server_port, const std::vector<std::uint8_t>& data);
void send_result(std::string result);


const char* tag = "http";


void main(void* arg) {
  const std::string server_address = "192.168.133.231";
  const std::string server_port = "8080";

  while (true) {
    channels::image_t image;
    if (xQueueReceive(channels::image_channel, static_cast<void*>(&image), portMAX_DELAY) != pdTRUE) continue;
    std::vector<std::uint8_t> data(image.buffer, image.buffer + image.size);
    delete[] image.buffer;

    std::expected<std::string, http_error> request_result = send_request(server_address, server_port, data);
    
    if (request_result.has_value()) {
      ESP_LOGI(tag, "request result size: \n%zu\n\n%s", request_result->size(), request_result->c_str());
      send_result(request_result->c_str());
    }
  }
}


void send_result(std::string result) {
  int32_t result_code = 404;
  if (result.contains("200")) result_code = 200;
  xQueueSend(channels::server_status_channel, static_cast<void*>(&result_code), 100 / portTICK_PERIOD_MS);
}


std::expected<std::string, http_error> send_request(const std::string& server_address, const std::string& server_port, const std::vector<std::uint8_t>& data) {
  addrinfo* dns_result = dns_lookup(server_address, server_port);
  if (dns_result == nullptr) return std::unexpected(dns_lookup_failed);

  int sock = create_socket(dns_result);
  if (sock < 0) return std::unexpected(allocate_socket_failed);

  if (!socket_connect(sock, dns_result)) return std::unexpected(socket_connect_failed);

  if (!socket_write(sock, server_address, data)) return std::unexpected(socket_send_failed);

  if (!set_socket_timeout(sock)) return std::unexpected(set_socket_recv_timeout_failed);

  return socket_read(sock);
}


addrinfo* dns_lookup(const std::string& server_address, const std::string& server_port) {
  addrinfo* dns_result;
  const addrinfo hints = {
    .ai_family = AF_INET,
    .ai_socktype = SOCK_STREAM,
  };
  int dns_return = getaddrinfo(server_address.c_str(), server_port.c_str(), &hints, &dns_result);

  if(dns_return != 0 || dns_result == NULL) {
    ESP_LOGE(tag, "DNS lookup failed | err: %d res: %p", dns_return, dns_result);
    return nullptr;
  }
  in_addr* addr = &((sockaddr_in*)(dns_result)->ai_addr)->sin_addr;
  ESP_LOGI(tag, "DNS lookup succeeded | IP: %s", inet_ntoa(*addr));
  return dns_result;
}


int create_socket(addrinfo* dns_result) {
  int sock = socket(dns_result->ai_family, dns_result->ai_socktype, 0);
  if(sock < 0) {
    ESP_LOGE(tag, "failed to allocate socket");
    freeaddrinfo(dns_result);
    return sock;
  }
  ESP_LOGI(tag, "allocated socket");
  return sock;
}


bool socket_connect(int sock, addrinfo* dns_result) {
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


std::string create_request(const std::string& host, const std::vector<std::uint8_t>& data) {
  std::string result = "POST / HTTP/1.1\r\n"
                       "Host: " + host + "\r\n"
                       "Content-Type: text/plain\r\n"
                       "Content-Type: image/jpeg\r\n"
                       "Content-Length: " + std::to_string(data.size()) + "\r\n"
                       "\r\n" + std::string(data.begin(), data.end());
  return result;
}


bool socket_write(int sock, const std::string& host, const std::vector<std::uint8_t>& request) {
  std::string http_request = create_request(host, request);
  if (write(sock, static_cast<const void*>(http_request.c_str()), http_request.length()) < 0) {
    ESP_LOGE(tag, "socket send failed");
    close(sock);
    return false;
  }
  ESP_LOGI(tag, "socket send success");
  return true;
}


bool set_socket_timeout(int sock) {
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


std::string socket_read(int sock) {
  std::string result;

  constexpr std::uint16_t buffer_size = 2048;
  std::uint8_t buffer[buffer_size];
  int read_size = read(sock, buffer, buffer_size);

  if (read_size > 0) result = std::string(buffer, buffer + read_size);
  else ESP_LOGE(tag, "can't read server response");

  ESP_LOGI(tag, "done reading from socket | return: %d errno: %d", result.size(), errno);
  close(sock);

  return result;
}


} // namespace http


#endif // !HTTP_HPP
