#ifndef __SERVO_H__
#define __SERVO_H__

#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define MOTOR5_GPIO 22U
#define MOTOR6_GPIO 16U

void motor_init(uint gpio);

void set_motor_angle(uint gpio, uint8_t angle);

void turn_on_motor(uint gpio);

void turn_off_motor(uint gpio);
#endif