#include <cstdint>
#include <iostream>
#include <wiringPi.h>

#include "sound.hpp"
#include "touch.hpp"

#define TOUCH_SUCCESS 0
#define TOUCH_INITERR 1

// Defines the max number of keys to load (must support up to 5 in the future)
#define MAX_TOUCH 3

typedef struct touch
{
    uint8_t pin;
    uint8_t state;
} TOUCH;

static touch touches[] = {
    {7, false},
    {0, false},
    {2, false},
    {3, false},
    {4, false}
};

static uint8_t get_touch(TOUCH *touch) {
    return digitalRead(touch->pin);
}

uint8_t init_touch() {
    std::cout << "  * initializing touches ...\n";
    if (wiringPiSetup()) {
        std::cout << " Failed with code : \n" << TOUCH_INITERR;
        return TOUCH_INITERR;
    }

    for (size_t i = 0; i < MAX_TOUCH; i++) {
        pinMode(touches[i].pin, INPUT);
    }

    std::cout << " Touch Success!\n";
    return TOUCH_SUCCESS;
}

void loop_touch() {
    uint8_t tmp_state;
    for (size_t i = 0; i < MAX_TOUCH; i++) {
        tmp_state = get_touch(&touches[i]);
        if (tmp_state != touches[i].state) {
            touches[i].state = tmp_state;
            std::cout << "Touch " << static_cast<int>(i) << " changed to "
                      << static_cast<int>(tmp_state) <<"!\n";

            if (tmp_state) {
                trigger_sample(i);
            }
        }
    }
}

void update_touch(TOUCH *touch, uint8_t state) {
    touch->state = state;
}
