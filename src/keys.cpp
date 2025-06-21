#include <cstdint>
#include <iostream>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "keys.hpp"
#include "sound.hpp"

// Error codes
#define KEY_SUCCESS 0
#define KEY_WIP_INI 1

// Register map
#define MCP23017_ADDR   0x20
#define IODIRA          0x00
    #define IODIRA0_7       0xFF
#define IODIRB          0x01
    #define IODIRB0_7       0x0F
#define GPPUA           0x0C
    #define GPPUA0_7        0xFF
#define GPPUB           0x0D
    #define GPPUB0_7        0x0F
#define GPIOA           0x12
#define GPIOB           0x13

#define MAX_KEYS 12

static key keys[] = {
    // Need updates
    {"K1" , 0 , 0b1}, // C     - Do
    {"K2" , 1 , 0b1}, // C#/Db - Do sos. / Re bemol
    {"K3" , 2 , 0b1}, // D     - Re
    {"K4" , 3 , 0b1}, // D#/Eb - Do sos. / Mi bemol
    {"K5" , 4 , 0b1},  // E     - Mi
    {"K6" , 5 , 0b1}, // F     - Fa
    {"K7" , 6 , 0b1}, // F#/Gb - Fa sos. / Sol bemol
    {"K8" , 7 , 0b1}, // G     - Sol
    {"K9" , 8 , 0b1}, // G#/Ab - Sol sos. / La bemol
    {"K10", 9 , 0b1}, // A     - La
    {"K11", 10, 0b1}, // A#/Bb - La sos. / Si bemol
    {"K12", 11, 0b1}  // B     - Si
};

static int fd;

static void write_config(uint8_t reg, uint8_t value) {
    wiringPiI2CWriteReg8(fd, reg, value);
}

static uint16_t read_data() {
    uint16_t gpioa_data = wiringPiI2CReadReg8(fd, GPIOA);
    uint16_t gpiob_data = wiringPiI2CReadReg8(fd, GPIOB);
    return (gpioa_data | (gpiob_data << 8));
}

static uint8_t get_bit(uint16_t data, uint8_t bit) {
    return (data >> bit) & 0b1;
}

uint8_t init_keys() {
    std::cout << "- Keys initialization ...\n";

    std::cout << "  * initializing WiringPi module ...\n";
    fd = wiringPiI2CSetup(MCP23017_ADDR);
    if (fd < 0) {
        std::cout << " Failed with code : \n" << KEY_WIP_INI;
        return KEY_WIP_INI;
    }

    std::cout << "  * initializing keys ...\n";
    write_config(IODIRA, IODIRA0_7);
    write_config(IODIRB, IODIRB0_7);
    write_config(GPPUA, GPPUA0_7);
    write_config(GPPUB, GPPUB0_7);

    std::cout << " Success!\n";
    return KEY_SUCCESS;
}

void loop_keys() {
    uint8_t curr_state;
    uint16_t data = read_data();

    for (size_t i = 0; i < MAX_KEYS; i++) {
        curr_state = get_bit(data, keys[i].pin);
        if (curr_state != keys[i].state) {
            keys[i].state = curr_state;
            std::cout << "Key " << keys[i].name << " changed to "
                      << static_cast<int>(curr_state) <<"!\n";
            
            trigger_gate(i);
        }
    }
}
