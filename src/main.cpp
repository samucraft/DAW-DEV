#include <csignal>
#include <cstdint>
#include <iostream>

#include "accel.hpp"
#include "keys.hpp"
#include "signal.hpp"
#include "sound.hpp"
#include "touch.hpp"
#include "utils.hpp"

int main() {
    std::cout << "<<< DAW-DEV App >>>\n";

    RET_IF_ERR(init_keys());
    RET_IF_ERR(init_sound());
    // init_touch();
    // init_accel();

    // Register the signal handler for SIGINT
    std::signal(SIGINT, signalHandler);

    while (true) {
        loop_keys();
        // loop_touch();
        // loop_accel();
    }

    cleanup_sound();

    std::cout << "... exiting app ...\n";
    return 0;
}
