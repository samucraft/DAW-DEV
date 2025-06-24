#include <cstdint>
#include <iostream>
#include <wiringPi.h>

#include "sound.hpp"
#include "touch.hpp"

#define TOUCH_SUCCESS 0
#define TOUCH_INITERR 1

// Defines the max number of keys to load
#define MAX_TOUCH 3

static touch touches[] = {
    {0 , false},
    {3, false},
    {7, false}
};

// Update internal key state
void update_touch(touch *touch, uint8_t state);

uint8_t init_touch() {
    std::cout << "  * initializing touches ...\n";
    if (wiringPiSetup()) {
        std::cout << " Failed with code : \n" << TOUCH_INITERR;
        return TOUCH_INITERR;
    }

    for (size_t i = 0; i < MAX_TOUCH; i++) {
        pinMode(touches[i].pin, OUTPUT);
        set_touch(&touches[i], TOUCH_UNPRESSED);
    }

    std::cout << " Touch Success!\n";
    return TOUCH_SUCCESS;
}

void set_touch(touch *touch, uint8_t state) {
    update_touch(touch, state);
    digitalWrite(touch->pin, state);
}

uint8_t get_touch(touch *touch) {
    return digitalRead(touch->pin);
}

void loop_touch() {
    uint8_t tmp_state;
    for (size_t i = 0; i < MAX_TOUCH; i++) {
        tmp_state = get_touch(&touches[i]);
        if (tmp_state != touches[i].state) {
            update_touch(&touches[i], tmp_state);
            std::cout << "Touch " << static_cast<int>(i) << " changed to "
                      << static_cast<int>(tmp_state) <<"!\n";
            if (tmp_state) {
                trigger_sample(i);
            }
        }
    }
}

void update_touch(touch *touch, uint8_t state) {
    touch->state = state;
}
