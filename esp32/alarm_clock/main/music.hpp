#pragma once

#ifndef _MUSIC_HPP_
#define _MUSIC_HPP_


namespace music {


static const char* TAG = "music";


void init() {
    ESP_LOGI(TAG, "Music initialized");
}


void main(void* args) {
    while (true) {
        ESP_LOGI(TAG, "Music playing");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


} // namespace music


#endif // !_MUSIC_HPP_
