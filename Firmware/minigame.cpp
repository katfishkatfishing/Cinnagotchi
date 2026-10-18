#include "minigame.hpp"
#include "esp_random.h"
#include <cmath>
#include <algorithm>

void CinnamonRollGame::init() {
    pos_x_ = 30.0f;
    pos_y_ = 30.0f;
    vel_x_ = 0.0f;
    vel_y_ = 0.0f;
    score_ = 0;
    time_remaining_ = 35.0f;
    is_active_ = true;
    spawnNewTarget();
}

void CinnamonRollGame::spawnNewTarget() {
    target_x_ = 160.0f + static_cast<float>(esp_random() % 60);
    target_y_ = 160.0f + static_cast<float>(esp_random() % 60);
}

void CinnamonRollGame::update(float deltaTime, Vitals& vitals) {
    if (!is_active_) return true;

    time_remaining_ -= deltaTime;

    if (time_remaining_ <= 0.0f) {
        is_active_ = false;
        Buzzer::chirpSad();
        return false;
    }

    SensorHub::MotionData motion = SensorHub::readMotionData();
    constexpr float GRAVITY_SCALE = 190.0f;
    constexpr float FRICTION = 0.98f;

    vel_x_ += (motion.accelX * GRAVITY_SCALE) * deltaTime;
    vel_y_ += (motion.accelY * GRAVITY_SCALE) * deltaTime;

    vel_x_ *= FRICTION;
    vel_y_ *= FRICTION;

    pos_x_ += vel_x_ * deltaTime;
    pos_y_ += vel_y_ * deltaTime;

    star_anise_.current_angle += star_anise_.spin_speed * deltaTime;
    if (star_anise_.current_angle > 6.28318f) star_anise_.current_angle -= 6.28318f;

    checkWallCollisions();
    checkQuillCollisions();
    checkStarAniseCollisions();

    float dx = pos_x_ - target_x_;
    float dy = pos_y_ - target_y_;
    if (std::sqrt(dx * dx + dy * dy) < (ROLL_RADIUS + TARGET_RADIUS)) {
        score_++;
        vitals.pet(15);
        vitals.feed(10);
        Buzzer::chirpHappy();
        spawnNewTarget();
    }
    return false;
}

void CinnamonRollGame::checkWallCollisions() {
    constexpr float BOUNCE = -0.6f;

    if (pos_x_ - ROLL_RADIUS < 0.0f) {
        pos_x_ = ROLL_RADIUS;
        vel_x_ *= BOUNCE;
        Buzzer::playTone(380, 8);
    } else if (pos_x_ + ROLL_RADIUS > SCREEN_WIDTH) {
        pos_x_ = SCREEN_WIDTH - ROLL_RADIUS;
        vel_x_ *= BOUNCE;
        Buzzer::playTone(380, 8);
    }

    if (pos_y_ - ROLL_RADIUS < 0.0f) {
        pos_y_ = ROLL_RADIUS;
        vel_y_ *= BOUNCE;
        Buzzer::playTone(380, 8);
    } else if (pos_y_ + ROLL_RADIUS > SCREEN_HEIGHT) {
        pos_y_ = SCREEN_HEIGHT - ROLL_RADIUS;
        vel_y_ *= BOUNCE;
        Buzzer::playTOne(380, 8);
    }
}

void CinnamonRollGame::checkQuillCollisions() {
    for (const auto& quill : quills_) {
        float dx = pos_x_ - quill.centerX;
        float dy = pos_y_ - quill.centerY;
        float cos_a = std::cos(-quill.angle_rad);
        float sin_a = std::sin(-quill.angle_rad);

        float local_x = dx * cos_a - dy * sin_a;
        float local_y = dx * sin_a + dy * cos_a;

        float half_length = quill.length * 0.5f;
        float half_thickness = quill.thickness * 0.5f;

        float clamped_x = std::max(-half_length, std::min(local_x, half_length));
        float clamped_y = std::max(-half_thickness, std::min(local_y, half_thickness));

        float clamped_dx = local_x - clamped_x;
        float clamped_dy = local_y - clamped_y;
        float distance_sq = clamped_dx * clamped_dx + clamped_dy * clamped_dy;

        if (distance_sq < (ROLL_RADIUS * ROLL_RADIUS) && distance_sq > 0.0001f) {
            float distance = std::sqrt(distance_sq);
            float overlap = ROLL_RADIUS - distance;

            // local normal vector
            float ln_x = clamped_dx / distance;
            float ln_y = clamped_dy / distance;

            float world_nx = ln_x * std::cos(quill.angle_rad) - ln_y * std::sin(quill.angle_rad);
            float world_ny = ln_x * std::sin(quill.angle_rad) + ln_y * std::cos(quill.angle_rad);

            pos_x_ += world_nx * overlap;
            pos_y_ += world_ny * overlap;

            float dot = vel_x_ * world_nx + vel_y_ * world_ny;
            vel_x_ -= (vel_x_ - 2.0f * dot * world_nx) * 0.55f;
            vel_y_ -= (vel_y_ - 2.0f * dot * world_ny) * 0.55f;

            Buzzer::playTone(520, 10);
        }
    }
}

void CinnamonRollGame::checkStarAniseCollisions() {
    float dx = pos_x_ - star_anise_.centerX;
    float dy = pos_y_ - star_anise_.centerY;
    float distance = std::sqrt(dx * dx + dy * dy);
    float hit_distance = ROLL_RADIUS + star_anise_.radius;

    if (distance < hit_distance && distance > 0.001f) {
        float nx = dx / distance;
        float ny = dy / distance;
        float overlap = hit_distance - distance;

        pos_x_ += nx * overlap;
        pos_y_ += ny * overlap;

        float tangent_x = -ny * (star_anise_.spin_speed * 12.0f);
        float tangent_y = nx * (star_anise_.spin_speed * 12.0f);

        float dot = vel_x_ * nx + vel_y_ * ny;
        vel_x_ = ((vel_x_ - 2.0f * dot * nx) * 0.65f) + tangent_x;
        vel_y_ = ((vel_y_ - 2.0f * dot * ny) * 0.65f) + tangent_y;

        Buzzer::playTone(740, 15);
    }
}