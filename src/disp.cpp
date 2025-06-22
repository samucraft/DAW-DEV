#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>

#include "disp.hpp"

// Error codes
#define DISP_SUCCESS 0
#define DISP_INI_ERR 1

#define NEXTION_SERIAL_DEV "/dev/serial0"

static int serial_port;

static void send_command(const std::string& cmd) {
    std::string full_cmd = cmd + "\xFF\xFF\xFF"; // Nextion commands end with 3 x 0xFF
    write(serial_port, full_cmd.c_str(), full_cmd.length());
}

uint8_t init_disp() {
    // Open the serial port
    serial_port = open(NEXTION_SERIAL_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_port == -1) {
        std::cerr << "Failed to open serial port." << std::endl;
        return DISP_INI_ERR;
    }

    // Configure the serial port
    struct termios options;
    tcgetattr(serial_port, &options);
    cfsetispeed(&options, B9600); // Set baud rate (should match Nextion setting)
    cfsetospeed(&options, B9600);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8; // 8 data bits
    options.c_cflag &= ~PARENB; // No parity
    options.c_cflag &= ~CSTOPB; // 1 stop bit
    options.c_cflag &= ~CRTSCTS; // No flow control
    tcsetattr(serial_port, TCSANOW, &options);

    return DISP_SUCCESS;
}

void cleanup_disp() {
    close(serial_port);
}

void set_chord(std::string chord, std::string composition) {
    send_command(chord);
    send_command(composition);
}
