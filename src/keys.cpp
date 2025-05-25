#include <cstdint>
#include <iostream>
#include <wiringPi.h>

#include "keys.hpp"
#include "sound.hpp"

// Defines the max number of keys to load
#define MAX_KEYS 5
// #define MAX_KEYS 12

static key keys[] = {
    // Need updates
    {"C4" , 7 }, // C     - Do
    {"E4", 0}, // C#/Db - Do sos. / Re bemol
    {"G4" , 2}, // D     - Re
    {"B4", 3}, // D#/Eb - Do sos. / Mi bemol
    {"C5" , 4}, // E     - Mi
    // {"F" , 7 }, // F     - Fa
    // {"F#", 7 }, // F#/Gb - Fa sos. / Sol bemol
    // {"G" , 7 }, // G     - Sol
    // {"G#", 7 }, // G#/Ab - Sol sos. / La bemol
    // {"A" , 7 }, // A     - La
    // {"A#", 7 }, // A#/Bb - La sos. / Si bemol
    // {"B" , 7 } // B     - Si
};

// Update internal key state
void update_key(key *key, uint8_t state);

uint8_t init_keys() {
    std::cout << "- Keys initialization ...\n";

    std::cout << "  * initializing WiringPi module ...\n";
    if (wiringPiSetup()) {
        std::cout << " Failed with code : \n" << KEY_WIP_INI;
        return KEY_WIP_INI;
    }

    std::cout << "  * initializing keys ...\n";
    for (size_t i = 0; i < MAX_KEYS; i++) {
        pinMode(keys[i].pin, OUTPUT);
        set_key(&keys[i], KEY_UNPRESSED);
    }

    std::cout << " Success!\n";
    return KEY_SUCCESS;
}

void set_key(key *key, uint8_t state) {
    update_key(key, state);
    digitalWrite(key->pin, state);
}

uint8_t get_key(key *key) {
    return digitalRead(key->pin);
}

void loop_keys() {
    uint8_t tmp_state;
    for (size_t i = 0; i < MAX_KEYS; i++) {
        tmp_state = get_key(&keys[i]);
        if (tmp_state != keys[i].state) {
            update_key(&keys[i], tmp_state);
            std::cout << "Key " << keys[i].name << " changed to "
                      << static_cast<int>(tmp_state) <<"!\n";
            
            trigger_gate(i);
        }
    }
}

void update_key(key *key, uint8_t state) {
    key->state = state;
}
