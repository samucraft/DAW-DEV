#include <csignal>
#include <cstdint>
#include <iostream>

#include "accel.hpp"
#include "analog.hpp"
#include "cam.hpp"
#include "disp.hpp"
#include "keys.hpp"
#include "led.hpp"
#include "signal.hpp"
#include "sound.hpp"
#include "touch.hpp"
#include "utils.hpp"

int main() {
    std::cout << "<<< DAW-DEV App >>>\n";

    RET_IF_ERR(init_keys());
    RET_IF_ERR(init_sound());
    RET_IF_ERR(init_disp());
    RET_IF_ERR(init_led());
    RET_IF_ERR(init_analog());
    RET_IF_ERR(init_touch());
    init_accel();
    init_cam();

    // Register the signal handler for SIGINT
    std::signal(SIGINT, signalHandler);

    while (true) {
        loop_keys();
        loop_touch();
        loop_accel();
        loop_analog();

        cam_check_gesture();
    }

    cleanup_sound();
    cleanup_disp();
    cleanup_led();

    std::cout << "... exiting app ...\n";
    return 0;
}
