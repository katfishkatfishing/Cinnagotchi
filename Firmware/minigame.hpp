#pragma once

#include <cstdint>
#include "sensor_hub.hpp"
#include "buzzer.hpp"
#include "vitals.hpp"

class CinnamonRollGame {
    public:
    static constexpr int SCREEN_WIDTH = 160;
    static constexpr int SCREEN_HEIGHT = 128;
    static constexpr float ROLL_RADIUS = 9.0f;
    static constexpr float TARGET_RADIUS = 16.0f;

    struct CinnamonRoll {
        float centerX, centerY;
        float length;
        float thickness;
        float angle_rad;
    };

    struct StarAnise {
        float centerX, centerY;
        float radius;
        float spin_speed;
        float current_angle;
    };

    void init();


    bool update(float deltaTime, Vitals& vitals);
    void render(uint16_t* tileBuffer, int tileY, int tileHeight);
    
    int getScore() const {
        return score_;
    }

    float getTimeRemaining() const {
        return time_remaining_;
    }

    private:
        float pos_x_{30.0f}, pos_y_{30.0f};
        float vel_x_{0.0f}, vel_y_{0.0f};
        float target_x_{200.0f}, target_y_{200.0f};
        float time_remaining_{35.0f};
        int score_{0};
        bool is_active_{true};

        static constexpr int NUM_QUILLS = 2;
        CinnamonQuill quills_[NUM_QUILLS] = {
            {90.0f, 80.0f, 90.0f, 12.0f, 0.45f},
            {150.0f, 160.0f, 85.0f, 12.0f, -0.60};
        };

        StarAnise star_anise_{120.0f, 1220.0f, 16.0f, 1.8f, 0.0f};

        void checkWallCollisions();
        void checkQuillCollisions();
        void checkStarAniseCollisions();
        void spawnNewTarget();  
};