#pragma once
#include <cstdint>
#include <algorithm>

struct Vitals {
    uint8_t hunger{100};
    uint8_t happiness{100};
    uint8_t energy{100};
    uint8_t cleanliness{100};
    uint32_t age_seconds{0};

    void tickDecay() {
        hunger = (hunger > 1) ? hunger - 1 : 0;
        happiness = (happiness > 1) ? happiness - 1 : 0;
        cleanliness = (cleanliness > 1) ? cleanliness - 1 : 0;
        age_seconds+= 5;
    }

    void feed(uint8_t amount) {
        hunger = std::min<uint8_t>(100, hunger + amount);
    }

    void pet(uint8_t amount) {
        happiness = std::min<uint8_t>(100, happiness + amount);
    }
};