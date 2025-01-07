#pragma once


#ifndef CHANNELS_HPP
#define CHANNELS_HPP


#include <cstdint>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


namespace channels {


typedef struct {
  std::uint8_t* buffer;
  std::uint64_t size;
} image_t;
constexpr UBaseType_t image_channel_len = 10;
QueueHandle_t image_channel;


void init() {
  image_channel = xQueueCreate(image_channel_len, sizeof(image_t));
}


}


#endif
