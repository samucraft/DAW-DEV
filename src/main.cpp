#include <csignal>
#include <cstdint>
#include <iostream>

#include "keys.hpp"
#include "signal.hpp"

int main() {
    uint8_t res;

    std::cout << "<<< DAW-DEV App >>>\n";

    std::cout << "- Keys initialization ...\n";
    res = init_keys();
    if (res) {
        std::cout << " Failed with code : \n" << res;
        return res;
    }
    std::cout << " Success!\n";

    // Register the signal handler for SIGINT
    std::signal(SIGINT, signalHandler);

    while (true) {
    }

    std::cout << "... exiting app ...\n";
    return 0;
}
