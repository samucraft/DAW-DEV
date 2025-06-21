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
    #define IODIRA0_7       0x1F
#define IODIRB          0x01
    #define IODIRB0_7       0x00
#define GPPUA           0x0C
    #define GPPUA0_7        0x1F
#define GPPUB           0x0D
    #define GPPUB0_7        0x00
#define GPIOA           0x12
#define GPIOB           0x13

#define GPIOA_PORTS 5
#define GPIOB_PORTS 0

static key keys[] = {
    // Need updates
    {"K1", 0, 0b1},  // C     - Do
    {"K2", 1, 0b1},  // C#/Db - Do sos. / Re bemol
    {"K3", 2, 0b1},  // D     - Re
    {"K4", 3, 0b1},  // D#/Eb - Do sos. / Mi bemol
    {"K5", 4, 0b1},  // E     - Mi
    // {"F" , 7 }, // F     - Fa
    // {"F#", 7 }, // F#/Gb - Fa sos. / Sol bemol
    // {"G" , 7 }, // G     - Sol
    // {"G#", 7 }, // G#/Ab - Sol sos. / La bemol
    // {"A" , 7 }, // A     - La
    // {"A#", 7 }, // A#/Bb - La sos. / Si bemol
    // {"B" , 7 } // B     - Si
};

static int fd;

static void write_config(uint8_t reg, uint8_t value) {
    wiringPiI2CWriteReg8(fd, reg, value);
}

static uint8_t read_port(uint8_t port) {
    return wiringPiI2CReadReg8(fd, port);
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

    uint8_t value = read_port(GPIOA);
    for (size_t i = 0; i < GPIOA_PORTS; i++) {
        curr_state = value & 0b1;
        if (curr_state != keys[i].state) {
            keys[i].state = curr_state;
            std::cout << "Key " << keys[i].name << " changed to "
                      << static_cast<int>(curr_state) <<"!\n";
            
            trigger_gate(i);
        }

        value >>= 1;
    }
}
