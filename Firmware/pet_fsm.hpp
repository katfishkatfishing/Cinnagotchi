#pragma once
#include "vitals.hpp"

enum class PetState {
    idle,
    happy,
    hungry,
    sleeping,
};

class PetFSM {
    public:
    void update(Vitals& vitals) {
        vitals.tickDecay();

        if (vitals.energy < 15) {
            currentState = PetState::sleeping;
        } else if (vitals.hunger < 30) {
            currentState = PetState::hungry;
        } else if (vitals.happiness < 30) {
            currentState = PetState::happy;
        } else {
            currentState = PetState::idle;
        }
    }

    PetState getState() const {
        return currentState;
    }

    void forceState(PetState state) {
        currentState = state;
    }

    private:
    PetState currentState{PetState::idle};
};