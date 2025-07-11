#include <csignal>
#include <cstdint>
#include <iostream>

#include "disp.hpp"
#include "led.hpp"
#include "signal.h"
#include "sound.hpp"

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nCtrl+C detected. Exiting program safely..." << std::endl;

        cleanup_sound();
        cleanup_disp();
        cleanup_led();

        exit(0); // Exit the program
    }
}
