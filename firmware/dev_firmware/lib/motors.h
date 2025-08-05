#pragma once
#include <stdint.h>

extern float deadzone;
extern volatile float last_motor_a_speed;
extern volatile float last_motor_b_speed;

void initialise_motors(void);
void motor_configure_pwm(uint8_t gpio_pin, float target_frequency);
void motor_store_speed(uint8_t pwm_pin, float speed);
void motor_set_speed(uint8_t pwm_pin, uint8_t in1_pin, uint8_t in2_pin, float speed);
void differential_drive(float adc_x, float adc_y, float deadzone, float *left, float *right);
void dive_motor_drive(float adc_z_norm, float deadzone);