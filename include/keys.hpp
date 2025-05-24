#ifndef DAW_KEYS_HPP
#define DAW_KEYS_HPP

// Error codes
#define KEY_SUCCESS 0
#define KEY_WIP_INI 1

// Key states
#define KEY_UNPRESSED 0
#define KEY_PRESSED   1

typedef struct key
{
    char name[3];
    uint8_t pin;
    uint8_t state;
} key;

// Initialize keys
uint8_t init_keys();

// Set key state
void set_key(key *key, uint8_t state);

// Get key state
uint8_t get_key(key *key);

// Main loop routine for keys
void loop_keys();

#endif
