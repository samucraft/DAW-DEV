#ifndef DAW_KEYS_HPP
#define DAW_KEYS_HPP

typedef struct key
{
    char name[4];
    uint8_t pin;
    uint8_t state;
} key;

// Initialize keys
uint8_t init_keys();

// Main loop routine for keys
void loop_keys();

#endif
