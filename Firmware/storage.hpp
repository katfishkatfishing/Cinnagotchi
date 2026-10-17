#pragma once

#include "vitals.hpp"
#include "esp_err.h"

class Storage {
    public:
        static esp_err_t init();
        static esp_err_t saveVitals(const Vitals& vitals);
        static esp_err_t loadVitals(VItals& vitals);

    private:
        static constexpr const char* STORAGE_NAMESPACE = "cinnagotchi";
        static constexpr const char* VITALS_KEY = "pet_data";

};