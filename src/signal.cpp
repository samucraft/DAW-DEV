#include <csignal>
#include <iostream>

#include "signal.h"

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nCtrl+C detected. Exiting program safely..." << std::endl;
        exit(0); // Exit the program
    }
}
