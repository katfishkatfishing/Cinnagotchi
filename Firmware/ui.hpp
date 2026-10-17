#pragma once

#include "button_driver.hpp"
#include <cstdint>
#include "button_driver.hpp
#include "vitals.hpp"
#include "buzzer.hpp"

enum class ViewState : uint8_t {
    VIEW_HOME,
    VIEW_MAIN_MENU,
    VIEW_SUBMENU_FOOD,
    VIEW_STATS_CARD
};

class UINavigator {
    public:
        static constexpr uint8_t MENU_TIMEOUT_MS = 4;
        static constexpr uint8_t FOOD_ITEMS_COUNT = 3;

        void handleInput(const ButtonMessage& btn, Vitals& vitals) {
            switch (current_view_) {
                case ViewState::VIEW_HOME:
                    handleHomeInput(btn);
                    break;
                case ViewState::VIEW_MAIN_MENU:
                    handleMainMenuInput(btn, vitals);
                    break;
                case ViewState::VIEW_SUBMENU_FOOD:
                    handleFoodMenuInput(btn, vitals);
                    break;
                case ViewState::VIEW_MAIN_MENU:
                    handleFoodSubmenuInput(btn, vitals);
                    break;
            }
        }

        ViewState getCurrentView() const {
            return current_view_;
        }
        uint8_t getMenuCursor() const {
            return menu_cursor_;
        }
        uint8_t getSubmenuCursor() const {
            return submenu_cursor_;
        }

    private:
        void handleHomeInput(const ButtonMessage& btn, Vitals& vitals) {
            if (btn.id == ButtonId::BTN_SCROLL) {
                menu_cursor_ = (menu_cursor_ + 1) % MENU_ITEMS_COUNT;
                buzzer_.click();
            } else if (btn.id == ButtonId::BIN_BACK) {
                current_view_ = ViewState::VIEW_HOME;
                Buzzer_.click();
            } else if (btn.id == ButtonId::BTN_SELECT) {
                Buzzer::playTone(1760, 400); // A6 
                switch (menu_cursor_) {
                    case 0: // food
                        current_view_ = ViewState::VIEW_SUBMENU_FOOD;
                        submenu_cursor_ = 0;
                        break;
                    case 1: //play
                        vitals.pet(25);
                        Buzzer::chirpHappy();
                        current_view_ = ViewState::VIEW_HOME;
                        break;
                    case 2: //clean
                        vitals.cleanliness = 100;
                        Buzzer::chirpHappy();
                        current_view_ = ViewState::VIEW_HOME;
                        break;
                    case 3: //stats
                        current_view_ = ViewState::VIEW_STATS_CARD;
                        break;
                }
            }
        }

        void handleFoodSubmenuInput(const ButtonMessage& btn, Vitals& vitals) {
            if (btn.id == ButtonId::BTN_SCROLL) {
                submenu_cursor_ = (submenu_cursor_ + 1) % FOOD_ITEMS_COUNT;
                Buzzer::click();
            } else if (btn.id == ButtonId::BTN_BACK) {
                current_view_ = ViewState::VIEW_MAIN_MENU;
                Buzzer::click();
            } else if (btn.id == ButtonId::BTN_SELECT) {
                uint8_t feedAmounts[3] = {30, 15, 10};
                vitals.feed(feedAmounts[submenu_cursor_]);
                Buzzer::chirpHappy();
                current_view_ = ViewState::VIEW_HOME;
            }
        }

        void handleStatsInput(const ButtonMessage& btn) {
            if (btn.id == ButtonId::BTN_BACK || btn.id == ButtonId::BTN_SELECT) {
                current_view_ = ViewState::VIEW_HOME;
                Buzzer::click();
            }
        }
};