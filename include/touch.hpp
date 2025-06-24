#ifndef DAW_TOUCH_H
#define DAW_TOUCH_H

// Key states
#define TOUCH_UNPRESSED 0
#define TOUCH_PRESSED   1

typedef struct touch
{
    uint8_t pin;
    uint8_t state;
} touch;

// Initialize keys
uint8_t init_touch();

// Set key state
void set_touch(touch *touch, uint8_t state);

// Get key state
uint8_t get_touch(touch *touch);

// Main loop routine for keys
void loop_touch();

#endif
