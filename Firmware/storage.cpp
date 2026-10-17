#include "storage.hpp"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <sys/time.h>

static const char* TAG = "NVS";

struct PackedVitals {
    uint8_t hunger;
    uint8_t happiness;
    uint8_t energy;
    uint32_t age_seconds;
    int64_t last_saved_epoch;
};

esp_err_t Storage::init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

esp_err_t Storage::saveVitals(const Vitals& vitals) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    PackedVitals packed = {
        .hunger = vitals.hunger,
        .happiness = vitals.happiness,
        .energy = vitals.energy,
        .age_seconds = vitals.age_seconds, 
        .last_saved_epoch = tv.tv_sec
    };

    err = nvs_set_blob(handle, VITALS_KEY, &packed, sizeof(packed));
    if (err == ESP_OK) {
        err = nvs_commmit(handle);
        ESP_LOGI(TAG, "Pet stats persisted to NVS successfully.");
    }
    nvs_close(handle);
    return err;
}

esp_err_t Storage::loadVitals(Vitals& vitals) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    PackedVitals packed;
    size_t required_size = sizeof(packed);
    err = nvs_get_blob(handle, VITALS_KEY, &packed, &reuired_size);
    nvs_close(handle);

    if (err == ESP_OK) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t elapsed_seconds = tv.tv_sec - packed.last_saved_epoch;

        vitals.age_seconds = packed.age_seconds + elapsed_seconds;

        if (elapsed_seconds > 0 && elapsed_seconds < (30 * 86400)) {
            vitals.age_seconds += static_cast<uint32_t>(elapsed_seconds);

            uint32_t hunger_decay = elapsed_seconds / 120;
            vitals.hunger = (packed.hunger > hunger_decay) ? (packed.hunger - hunger_decay) : 0;
            
            uint32_t happiness_decay = elapsed_seconds / 180;
            vitals.happiness = (packed.happiness > happiness_decay) ? (packed.happiness - happiness_decay) : 0;

            vitals.energy = packed.energy;
            ESP_LOGW(TAG, "Offline catch up: %llds elapsed. Hunger: %d, Happiness: %d, Energy: %d", elapsed_seconds, vitals.hunger, vitals.happiness, vitals.energy);
        } else {
            vitals.hunger = packed.hunger;
            vitals.happiness = packed.happiness;
            vitals.energy = packed.energy;
            ESP_LOGW(TAG, "No offline catch up needed.");
        }
    }
    return err;
}