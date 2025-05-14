#pragma once

#ifndef _MUSIC_HPP_
#define _MUSIC_HPP_


namespace music {


static const char* TAG = "music";


void init() {
    ESP_LOGI(TAG, "Music initialized");
}


void main(void* args) {
    ESP_LOGI(TAG, "Music playing");
    while (true) vTaskDelay(pdMS_TO_TICKS(1000));
}


} // namespace music


#endif // !_MUSIC_HPP_
