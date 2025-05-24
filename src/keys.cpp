#include <cstdint>
#include <iostream>
#include <wiringPi.h>

#include "keys.hpp"

// Defines the max number of keys to load
#define MAX_KEYS 1

static key keys[] = {
    {"C", 7} // C - Do
};

// Update internal key state
void update_key(key *key, uint8_t state);

uint8_t init_keys() {
    std::cout << "  * initializing WiringPi module ...\n";
    if (wiringPiSetup()) {
        return KEY_WIP_INI;
    }

    std::cout << "  * initializing keys ...\n";
    for (size_t i = 0; i < MAX_KEYS; i++) {
        pinMode(keys[i].pin, OUTPUT);
        set_key(&keys[i], KEY_UNPRESSED);
    }

    return KEY_SUCCESS;
}

void set_key(key *key, uint8_t state) {
    update_key(key, state);
    digitalWrite(key->pin, state);
}

uint8_t get_key(key *key) {
    return digitalRead(key->pin);
}

void update_key(key *key, uint8_t state) {
    key->state = state;
}
