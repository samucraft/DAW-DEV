#ifndef DAW_LED_H
#define DAW_LED_H

uint8_t init_led();

void cleanup_led();

void set_led(uint8_t index, bool state);

void light_suggestions(uint8_t idx_1, uint8_t idx_2);

void turn_off_suggestions();

#endif
