#ifndef DAW_KEYS_HPP
#define DAW_KEYS_HPP

#define MAX_KEYS 12

typedef struct key
{
    char name[5];
    uint8_t state;
} key;

// Initialize keys
uint8_t init_keys();

// Main loop routine for keys
void loop_keys();

#endif
