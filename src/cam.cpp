#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/file.h>
#include <unistd.h>

#include "sound.hpp"

#include "cam.hpp"

#define CAM_ERR        -1
#define GESTURE_NOHAND  0
#define GESTURE_CLOSED  1
#define GESTURE_OPENED  2

static const char* GESTURE_FILE = "/tmp/gesture.txt";

static SIGNAL_TYPE current_sound_selected;

static int read_fifo_gesture() {
    int fd = open(GESTURE_FILE, O_RDONLY);
    if (fd == -1) {
        return CAM_ERR;
    }

    flock(fd, LOCK_SH);  // shared lock

    char buffer[64];
    ssize_t len = read(fd, buffer, sizeof(buffer) - 1);
    buffer[(len > 0) ? len : 0] = '\0';

    flock(fd, LOCK_UN);  // unlock
    close(fd);

    return std::stoi(buffer);
}

void init_cam() {
    current_sound_selected = WAVE_e;
    change_sound_type(current_sound_selected);
}

void cam_check_gesture() {
    int gesture = read_fifo_gesture();
    switch (gesture) {
        case CAM_ERR:
            std::cout << "Couldn't open the camera FIFO!" << std::endl;
            break;

        case GESTURE_NOHAND:
            // just silently skip
            break;

        case GESTURE_CLOSED:
            if (current_sound_selected == KS_e) {
                current_sound_selected = WAVE_e;
                change_sound_type(current_sound_selected);
            }
            break;

        case GESTURE_OPENED:
            if (current_sound_selected == WAVE_e) {
                current_sound_selected = KS_e;
                change_sound_type(current_sound_selected);
            }
            break;
        
        default:
            std::cout << "Error: Gesture (" << gesture << ") not valid!"
                      << std::endl;
            break;
    }
}
