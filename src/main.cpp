#include <csignal>
#include <cstdint>
#include <iostream>

#include "keys.hpp"
#include "signal.hpp"
#include "utils.hpp"

int main() {
    std::cout << "<<< DAW-DEV App >>>\n";

    RET_IF_ERR(init_keys());

    // Register the signal handler for SIGINT
    std::signal(SIGINT, signalHandler);

    while (true) {
    }

    std::cout << "... exiting app ...\n";
    return 0;
}
