#include "motors.h"
#include <stdio.h>
#include <math.h>
#include <pico/stdlib.h>
#include <hardware/pwm.h>
#include <hardware/gpio.h>
#include "pins.h"

// Global variables
float deadzone = 0.05f;
volatile float last_motor_a_speed = 0;
volatile float last_motor_b_speed = 0;

void initialise_motors(void) {
        //Configure motor block 1
    //Motor A
    motor_configure_pwm(MOTOR_A_PWM_PIN, 1000.0f);
    gpio_init(MOTOR_A_IN1_PIN);
    gpio_init(MOTOR_A_IN2_PIN);
    gpio_set_dir(MOTOR_A_IN1_PIN, GPIO_OUT);
    gpio_set_dir(MOTOR_A_IN2_PIN, GPIO_OUT);
    //Motor B
    motor_configure_pwm(MOTOR_B_PWM_PIN, 1000.0f);
    gpio_init(MOTOR_B_IN1_PIN);
    gpio_init(MOTOR_B_IN2_PIN);
    gpio_set_dir(MOTOR_B_IN1_PIN, GPIO_OUT);
    gpio_set_dir(MOTOR_B_IN2_PIN, GPIO_OUT);


    gpio_init(STBY_PIN1);
    gpio_set_dir(STBY_PIN1, GPIO_OUT);
    gpio_put(STBY_PIN1, true); // Enable the motor driver

    //Initialise direction pins to stopped (both low)
    gpio_put(MOTOR_A_IN1_PIN, 0);
    gpio_put(MOTOR_A_IN2_PIN, 0);
    gpio_put(MOTOR_B_IN1_PIN, 0);
    gpio_put(MOTOR_B_IN2_PIN, 0);

        //Configure motor block 2
    //Motor C
    motor_configure_pwm(MOTOR_C_PWM_PIN, 1000.0f);
    gpio_init(MOTOR_C_IN1_PIN);
    gpio_init(MOTOR_C_IN2_PIN);
    gpio_set_dir(MOTOR_C_IN1_PIN, GPIO_OUT);
    gpio_set_dir(MOTOR_C_IN2_PIN, GPIO_OUT);

    gpio_init(STBY_PIN2);
    gpio_set_dir(STBY_PIN2, GPIO_OUT);
    gpio_put(STBY_PIN2, true); // Enable the motor driver

    //Initialise direction pins to stopped
    gpio_put(MOTOR_C_IN1_PIN, 0); // Set IN1 low
    gpio_put(MOTOR_C_IN2_PIN, 0); // Set IN2 low
}

void motor_configure_pwm(uint8_t gpio_pin, float target_frequency) {
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);

    const uint32_t clock_sys = 125000000;
    float clkdiv = 4.0f;
    uint32_t wrap = (clock_sys / (target_frequency * clkdiv)) - 1;

    while (wrap > 65535) {
        clkdiv += 1.0f;
        wrap = (clock_sys / (target_frequency * clkdiv)) - 1;
    }

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clkdiv);
    pwm_config_set_wrap(&config, wrap);

    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(gpio_pin, 0);
}

void motor_store_speed(uint8_t pwm_pin, float speed) {
    if (pwm_pin == MOTOR_A_PWM_PIN) {
        last_motor_a_speed = speed;
        printf("Motor A speed set to: %f\n", speed);
    } else if (pwm_pin == MOTOR_B_PWM_PIN) {
        last_motor_b_speed = speed;
        printf("Motor B speed set to: %f\n", speed);
    }
}

void motor_set_speed(uint8_t pwm_pin, uint8_t in1_pin, uint8_t in2_pin, float speed ){
    if (speed > 0.0f) {
        gpio_put(in1_pin, 0); // IN1 low
        gpio_put(in2_pin, 1); // IN2 high
    } else if (speed < 0.0f) {
        gpio_put(in1_pin, 1); // IN1 high
        gpio_put(in2_pin, 0); // IN2 low
    } else {
        gpio_put(in1_pin, 0);
        gpio_put(in2_pin, 0);
    }
    float duty_cycle = fabsf(speed);
    pwm_set_gpio_level(pwm_pin, (uint16_t)(duty_cycle * 65535));
    motor_store_speed(pwm_pin, speed);
}

void differential_drive(float adc_x, float adc_y, float deadzone, float *left, float *right) {
    if(fabsf(adc_x) < deadzone) adc_x = 0.0f;
    if(fabsf(adc_y) < deadzone) adc_y = 0.0f;

    float l = adc_y + adc_x;
    float r = adc_y - adc_x;

    float max_mag = fmaxf(fabsf(l), fabsf(r));
    if(max_mag > 1.0f) {
        l /= max_mag;
        r /= max_mag;
    }

    *left = l;
    *right = r;
}

void dive_motor_drive(float adc_z_norm, float deadzone) {
    if(fabsf(adc_z_norm) < deadzone) {
        motor_set_speed(MOTOR_C_PWM_PIN, MOTOR_C_IN1_PIN, MOTOR_C_IN2_PIN, 0.0f);
        return;
    }
    motor_set_speed(MOTOR_C_PWM_PIN, MOTOR_C_IN1_PIN, MOTOR_C_IN2_PIN, adc_z_norm);
}