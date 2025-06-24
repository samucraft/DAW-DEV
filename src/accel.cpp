/*
    Based on:
    MPU6050 Interfacing with Raspberry Pi
    http://www.electronicwings.com
*/

#include <cmath>
#include <wiringPiI2C.h>
#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>

#include "accel.hpp"
#include "sound.hpp"

#define Device_Address 0x68	/*Device Address/Identifier for MPU6050*/

#define PWR_MGMT_1   0x6B
#define SMPLRT_DIV   0x19
#define CONFIG       0x1A
#define GYRO_CONFIG  0x1B
#define INT_ENABLE   0x38
#define ACCEL_XOUT_H 0x3B
#define ACCEL_YOUT_H 0x3D
#define ACCEL_ZOUT_H 0x3F
#define GYRO_XOUT_H  0x43
#define GYRO_YOUT_H  0x45
#define GYRO_ZOUT_H  0x47

static int fd;

static float Acc_x;
static float Acc_y;
static float Acc_z;

static float Gyro_x;
static float Gyro_y;
static float Gyro_z;

static float Ax;
static float Ay;
static float Az;

static float Gx;
static float Gy;
static float Gz;

static void MPU6050_Init() {
    wiringPiI2CWriteReg8(fd, SMPLRT_DIV, 0x07); /* Write to sample rate register */
    wiringPiI2CWriteReg8(fd, PWR_MGMT_1, 0x01); /* Write to power management register */
    wiringPiI2CWriteReg8(fd, CONFIG, 0);        /* Write to Configuration register */
    wiringPiI2CWriteReg8(fd, GYRO_CONFIG, 24);  /* Write to Gyro Configuration register */
    wiringPiI2CWriteReg8(fd, INT_ENABLE, 0x01); /*Write to interrupt enable register */
}

static short read_raw_data(int addr){
    short high_byte,low_byte,value;
    high_byte = wiringPiI2CReadReg8(fd, addr);
    low_byte = wiringPiI2CReadReg8(fd, addr+1);
    value = (high_byte << 8) | low_byte;
    return value;
}

void init_accel() {
    Acc_x = 0.0f;
    Acc_y = 0.0f;
    Acc_z = 0.0f;

    Gyro_x = 0.0f;
    Gyro_y = 0.0f;
    Gyro_z = 0.0f;

    Ax = 0.0f;
    Ay = 0.0f;
    Az = 0.0f;

    Gx = 0.0f;
    Gy = 0.0f;
    Gz = 0.0f;

    fd = wiringPiI2CSetup(Device_Address); /*Initializes I2C with device Address*/

    MPU6050_Init(); /* Initializes MPU6050 */
}

void loop_accel() {
    /*Read raw value of Accelerometer and gyroscope from MPU6050*/
    Acc_x = read_raw_data(ACCEL_XOUT_H);
    Acc_y = read_raw_data(ACCEL_YOUT_H);
    Acc_z = read_raw_data(ACCEL_ZOUT_H);

    // Disable
    Gyro_x = read_raw_data(GYRO_XOUT_H);
    Gyro_y = read_raw_data(GYRO_YOUT_H);
    Gyro_z = read_raw_data(GYRO_ZOUT_H);

    /* Divide raw value by sensitivity scale factor */
    Ax = Acc_x / 16384.0f;
    Ay = Acc_y / 16384.0f;
    Az = Acc_z / 16384.0f;

    // Disable
    Gx = Gyro_x / 131.0f;
    Gy = Gyro_y / 131.0f;
    Gz = Gyro_z / 131.0f;

    // Disable
    // printf("\n Gx=%.3f °/s\tGy=%.3f °/s\tGz=%.3f °/s\tAx=%.3f g\tAy=%.3f g\tAz=%.3f g\n",Gx,Gy,Gz,Ax,Ay,Az);

    // Disable
    float mag2 = std::sqrt((Gx*Gx) + (Gy*Gy) + (Gz*Gz));
    float mag3 = std::sqrt(std::sqrt((Ax*Ax) + (Ay*Ay) + (Az*Az)) + std::sqrt((Gx*Gx) + (Gy*Gy) + (Gz*Gz)));
    
    float mag = std::sqrt((Ax*Ax) + (Ay*Ay) + (Az*Az));
    printf("|%f|%f|%f|\n", mag, mag2, mag3);

    if (mag >= 1.1f) {
        trigger_vibrato();
    }

    delay(100);
}
