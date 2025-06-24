#include <cstdint>
#include <iostream>

#include "ws2811.h"

#include "keys.hpp"

#include "led.hpp"

#define E_LED_SUCC 0
#define E_LED_FAIL 1

#define LED_BRIGHTNESS 3
#define LED_COLOR      0xF4430D
    #define LED_COLOR_BLK   0x000000
    #define LED_COLOR_SUG1  0x00FF00
    #define LED_COLOR_SUG2  0x0000FF
#define LED_COUNT      18
    #define LED_DISP_CNT    6
#define LED_PIN        18

static ws2811_t ledstring =
{
    .freq = WS2811_TARGET_FREQ,
    .dmanum = 10,
    .channel =
    {
        [0] =
        {
            .gpionum = LED_PIN,
            .invert = 0,
            .count = LED_COUNT,
            .strip_type = WS2811_STRIP_GRB,
            .brightness = LED_BRIGHTNESS,
        },
        [1] =
        {
            .gpionum = 0,
            .invert = 0,
            .count = 0,
            .brightness = 0,
        },
    },
};

// Array with LED indexes
// The index represents the Key index
// The content reporesents the LED index
static uint8_t key_leds[] = {
    17,
    6 ,
    16,
    7 ,
    15,
    14,
    8 ,
    13,
    9 ,
    12,
    10,
    11
};

static uint8_t last_sug1, last_sug2;

uint8_t init_led() {
    if (ws2811_init(&ledstring) != WS2811_SUCCESS) {
        std::cout << "ws2811_init failed" << std::endl;
        return E_LED_FAIL;
    }

    for (int i = 0; i < LED_DISP_CNT; ++i) {
        ledstring.channel[0].leds[i] = LED_COLOR;
    }
    for (int i = LED_DISP_CNT; i < LED_COUNT; ++i) {
        ledstring.channel[0].leds[i] = LED_COLOR_BLK;
    }
    ws2811_render(&ledstring);

    return E_LED_SUCC;
}

void cleanup_led() {
    ws2811_fini(&ledstring);
}

void set_led(uint8_t index, bool state) {
    uint8_t  led_i = key_leds[index];
    uint32_t color = state ? LED_COLOR : LED_COLOR_BLK;

    ledstring.channel[0].leds[led_i] = color;
    ws2811_render(&ledstring);
}

void light_suggestions(uint8_t idx_1, uint8_t idx_2) {
    uint8_t led_1 = key_leds[idx_1];
    uint8_t led_2 = key_leds[idx_2];

    ledstring.channel[0].leds[led_1] = LED_COLOR_SUG1;
    ledstring.channel[0].leds[led_2] = LED_COLOR_SUG2;

    last_sug1 = idx_1;
    last_sug2 = idx_2;
}

void turn_off_suggestions() {
    set_led(last_sug1, false);
    set_led(last_sug2, false);
}
