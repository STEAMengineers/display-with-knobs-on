#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "math.h"
//#include "lib/ili9488.h"

//TODO -SCREEN
//https://github.com/zapta/pio_tft/tree/main
//functionalise

// Motor Defines
// Block 1
#define MOTOR_A_PWM_PIN 24
#define MOTOR_A_IN1_PIN 29
#define MOTOR_A_IN2_PIN 28
#define MOTOR_B_PWM_PIN 25
#define MOTOR_B_IN1_PIN 32
#define MOTOR_B_IN2_PIN 31
#define STBY_PIN1 30

// Block 2
#define MOTOR_C_PWM_PIN 26
#define MOTOR_C_IN1_PIN 33
#define MOTOR_C_IN2_PIN 34
#define STBY_PIN2 35

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 12
#define PIN_CS   13
#define PIN_SCK  14
#define PIN_MOSI 11
#define SCREEN_RESET 18
#define SCREEN_DC 17
#define SCREEN_BACKLIGHT 16


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

void motor_set_speed(uint8_t pwm_pin, uint8_t in1_pin, uint8_t in2_pin, float speed ){

        //set direction
        if (speed > 0){
            gpio_put(in1_pin, false); // Set IN1 low
            gpio_put(in2_pin, true); // Set IN2 high
        } else {
            gpio_put(in2_pin, false); // Set IN2 low
            gpio_put(in1_pin, true); // Set IN1 high
        }    
        //set speed
            float duty_cycle = fabsf(speed); 
            pwm_set_gpio_level(pwm_pin, (uint16_t)(duty_cycle * 65535));

}


int main()
{
    stdio_init_all();
    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(41);
    adc_gpio_init(42);
    adc_gpio_init(43);
    adc_gpio_init(44);

    //ili9488_init();


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

    //Initialise direction pins to stopped
    gpio_put(MOTOR_A_IN1_PIN, true); // Set IN1 high
    gpio_put(MOTOR_A_IN2_PIN, true); // Set IN2 low
    gpio_put(MOTOR_B_IN1_PIN, true); // Set IN1 high
    gpio_put(MOTOR_B_IN2_PIN, true); // Set IN2 low


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
    gpio_put(MOTOR_C_IN1_PIN, true); // Set IN1 high
    gpio_put(MOTOR_C_IN2_PIN, true); // Set IN2 low

    //initialise screen
    gpio_init(SCREEN_BACKLIGHT);
    gpio_set_dir(SCREEN_BACKLIGHT, GPIO_OUT);
    gpio_put(SCREEN_BACKLIGHT, true);


    //Setup SPI
    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Other Screen pin init
    //gpio_set_function(LED_SCN, GPIO_FUNC_SIO);
    //gpio_set_dir(LED_SCN, GPIO_OUT);
    gpio_set_function(SCREEN_RESET, GPIO_FUNC_SIO);
    gpio_set_dir(SCREEN_RESET, GPIO_OUT);
    gpio_set_function(SCREEN_DC, GPIO_FUNC_SIO);
    gpio_set_dir(SCREEN_DC, GPIO_OUT);

    
    //Chip select is active-low, so we'll initialise it to a driven-high state
   gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    gpio_put(PIN_CS, false);

    uint8_t input[20];
	uint8_t buf[] = {0xB9, 0xFF, 0x83, 0x57};

    uint16_t reg[] = {0x00D0};
    uint16_t reg_read[1];

    spi_write_read_blocking(spi_default, buf, input, 4);

    gpio_put(PIN_CS, true);

    sleep_us(100);

    gpio_put(PIN_CS, false);

    spi_write16_read16_blocking(spi_default, reg, reg_read, 1);

    gpio_put(PIN_CS, true);



    printf("reg = %d", (int)reg_read[0]);



    while (1) {
            printf("<<<<reg = %d\n>", (int)reg_read[0]);
        adc_select_input(3);  //Joystick A x
        uint adc_x_raw = adc_read();
        adc_select_input(4);  //Joystick A y
        uint adc_y_raw = adc_read();
        adc_select_input(1);  //Joystick B y
        uint adc_z_raw = adc_read();
        
        //Normalise joystick input -1 to 1
        float adc_x_norm = ((float)adc_x_raw -2105)/2105;
        float adc_y_norm = ((float)adc_y_raw -2105)/2105;

        // https://xiaoxiae.github.io/Robotics-Simplified-Website/drivetrain-control/arcade-drive/
        float max_speed = fmax(fabsf(adc_x_norm), fabsf(adc_y_norm));
        float tot_speed = adc_x_norm + adc_y_norm;
        float diff_speed = adc_x_norm - adc_y_norm;
        // printf("%f, %f, %f\n", max_speed, tot_speed, diff_speed );
        if (adc_y_norm > 0) {
            if(adc_x_norm > 0) {
                //left max, right diff
                motor_set_speed(MOTOR_A_PWM_PIN, MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, max_speed);
                motor_set_speed(MOTOR_B_PWM_PIN, MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN, diff_speed);
            } else {
                //left tot, right max
                motor_set_speed(MOTOR_A_PWM_PIN, MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, tot_speed);
                motor_set_speed(MOTOR_B_PWM_PIN, MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN, max_speed);
            }
        } else {
            if(adc_x_norm > 0 ) {
                // left tot, right -max
                motor_set_speed(MOTOR_A_PWM_PIN, MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, tot_speed);
                motor_set_speed(MOTOR_B_PWM_PIN, MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN, -1 * max_speed);

            } else {
                // left -max, right diff
                motor_set_speed(MOTOR_A_PWM_PIN, MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, -1 * max_speed);
                motor_set_speed(MOTOR_B_PWM_PIN, MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN, diff_speed);
            }
        }





        //printf("%f - %f = ", adc_x_norm);        
        //double magnitude_xy = hypot((double)adc_x_norm, (double)adc_y_norm);
        //double angle_xy = atan2((double)adc_y_norm, (double)adc_x_norm);




        //printf("%f\n", magnitude_xy);
        //printf("\n");    

        // Display the joystick position something like this:
        // X: [            o             ]  Y: [              o         ]
//        const uint bar_width = 40;
//        const uint adc_max = (1 << 12) - 1;
//        uint bar_x_pos = adc_x_raw * bar_width / adc_max;
//        uint bar_y_pos = adc_y_raw * bar_width / adc_max;
//        printf("\rX: [");
//        for (uint i = 0; i < bar_width; ++i)
//            putchar( i == bar_x_pos ? 'o' : ' ');
//        printf("%d", adc_x_raw);    
//        printf("]  Y: [");
//        for (uint i = 0; i < bar_width; ++i)
//            putchar( i == bar_y_pos ? 'o' : ' ');
//        printf("]\n");


        //Speed and direction of dive motor
        if(adc_z_raw > 2110) {
            //Motor forward
            gpio_put(MOTOR_C_IN1_PIN, true); // Set IN1 high
            gpio_put(MOTOR_C_IN2_PIN, false); // Set IN2 low
            float duty_cycle_z = (float)adc_z_raw - 2110;
            //duty_cycle_x = (2105 - duty_cycle_x)/2105; 
            duty_cycle_z = duty_cycle_z / 2110;
            pwm_set_gpio_level(MOTOR_C_PWM_PIN, (uint16_t)(duty_cycle_z * 65535));

        } else if(adc_z_raw < 2100) {
            //motor reverse
            gpio_put(MOTOR_C_IN1_PIN, false); // Set IN1 low
            gpio_put(MOTOR_C_IN2_PIN, true); // Set IN2 high
            float duty_cycle_z = (2105 - (float)adc_z_raw)/2105; 
            pwm_set_gpio_level(MOTOR_C_PWM_PIN, (uint16_t)(duty_cycle_z * 65535));

        } else {
            gpio_put(MOTOR_C_IN1_PIN, true); // Set IN1 low
            gpio_put(MOTOR_C_IN2_PIN, true); // Set IN2 high
        }

        sleep_ms(50);

    }

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

}
