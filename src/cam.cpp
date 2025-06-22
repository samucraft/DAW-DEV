#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/file.h>
#include <unistd.h>

#include "sound.hpp"

#include "cam.hpp"

#define GESTURE_NOHAND  '0'
#define GESTURE_CLOSED  '1'
#define GESTURE_OPENED  '2'
#define CAM_ERR         '3'

static const char* GESTURE_FILE = "/tmp/gesture.txt";

static SIGNAL_TYPE current_sound_selected;

static char read_fifo_gesture() {
    int fd = open(GESTURE_FILE, O_RDONLY);
    if (fd == -1) {
        return CAM_ERR;
    }

    flock(fd, LOCK_SH);  // shared lock

    char buffer[4];
    read(fd, buffer, sizeof(buffer) - 1);

    flock(fd, LOCK_UN);  // unlock
    close(fd);

    return buffer[0];
}

void init_cam() {
    current_sound_selected = WAVE_e;
    change_sound_type(current_sound_selected);
}

void cam_check_gesture() {
    char gesture = read_fifo_gesture();
    switch (gesture) {
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

        case CAM_ERR:
            std::cout << "Couldn't open the camera FIFO!" << std::endl;
            break;
        
        default:
            std::cout << "Error: Gesture (" << gesture << ") not valid!"
                      << std::endl;
            break;
    }
}
