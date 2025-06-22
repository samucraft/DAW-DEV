#include <iostream>
#include <time.h>
#include <unistd.h>
#include <wiringPiI2C.h>

#include "sound.hpp"

#include "analog.hpp"

#define ANALOG_SUCCESS 0
#define ANALOG_INITERR 1
#define ANALOG_WRITERR 2
#define ANALOG_READERR 3

#define ADS7830_ADDRESS   0x48

/*
 * Representation of ADS7830 control byte for Single-ended Mode:
 * | S-E | A:0 | A:2 | A:1 | PD1 | PD0 |  -  |  -  |
 * |  1  | A:0 | A:2 | A:1 |  0  |  0  |  0  |  0  |
 */

// Volume is connected to AIN0 -> 0b00[0] -> 0b[0]00 (after odd fix)
#define ANALOG_VOLUME_CTRL 0x80

// Reverb is connected to AIN1 -> 0b00[1] -> 0b[1]00 (after odd fix)
#define ANALOG_REVERB_CTRL 0xC0

#define MAX_ANALOG_VALUE_INT 255
#define MAX_ANALOG_VALUE_FLT 255.0f

static int analog_fd;

static bool reverb_status;

static uint8_t read_analog(uint8_t control_byte, uint8_t *value){
    // Write control byte
    if (write(analog_fd, &control_byte, 1) != 1) {
        std::cerr << "Failed to write control byte" << std::endl;
        return ANALOG_WRITERR;
    }

    // Wait a little for ADC to complete
    usleep(500);

    // Read byte result
    if (read(analog_fd, value, 1) != 1) {
        std::cerr << "Failed to read ADC value" << std::endl;
        return ANALOG_READERR;
    }
    return ANALOG_SUCCESS;
}

static uint8_t invert_value(uint8_t value) {
    return MAX_ANALOG_VALUE_INT - value;
}

static float normalize_value(uint8_t value) {
    return static_cast<float>(value) / MAX_ANALOG_VALUE_FLT;
}

uint8_t init_analog() {
    analog_fd = wiringPiI2CSetup(ADS7830_ADDRESS);
    if (analog_fd < 0) {
        std::cerr << "Failed to init I2C communication to ADS7830" << std::endl;
        return ANALOG_INITERR;
    }

    reverb_status = false;

    return ANALOG_SUCCESS;
}

void loop_analog() {
    uint8_t volume, reverb;

    // Volume control
    if (read_analog(ANALOG_VOLUME_CTRL, &volume) != ANALOG_SUCCESS) {
        std::cerr << "Failed to read volume value" << std::endl;
    } else {
        /*
         * Since turning the volume knob to the right sets the value to 0, we
         * need to convert the volume we get from 255-0 to 0-255.
         */
        volume = invert_value(volume);
        set_volume(normalize_value(volume));
    }

    // Reverb control
    if (read_analog(ANALOG_REVERB_CTRL, &reverb) != ANALOG_SUCCESS) {
        std::cerr << "Failed to read reverb value" << std::endl;
    } else {
        bool new_reverb = normalize_value(invert_value(reverb)) > 0.5f
                          ? true : false;
        if (new_reverb != reverb_status) {
            reverb_status = new_reverb;
            trigger_reverb();
            std::cout << "Reverb is now set to " << reverb_status << std::endl;
        }
    }
}
