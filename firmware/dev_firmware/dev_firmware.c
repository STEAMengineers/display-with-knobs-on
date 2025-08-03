#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "math.h"
#include "lib/screen.h"
#include "lvgl/lvgl.h"
#include "lib/anim_frames.h"

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
#define SPI_PORT spi1
#define PIN_MISO 12
#define PIN_CS   13
#define PIN_SCK  14
#define PIN_MOSI 11
#define SCREEN_RESET 18
#define SCREEN_DC 17
#define SCREEN_BACKLIGHT 16

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320
//#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))
// Buffer for the display
// This buffer will be used to hold the pixel data for the display
// The size of the buffer is calculated based on the display width, height, and color depth
//static uint8_t buf1[DISPLAY_WIDTH * 10 * 2]; // 10 rows of pixels, each pixel is 2 bytes (RGB565)


#define LV_COLOR_SIZE 2
static uint8_t buf1[DISPLAY_WIDTH * 320 * LV_COLOR_SIZE];

// static lv_draw_buf_t buf1[DISPLAY_WIDTH * 10 * LV_COLOR_SIZE];

volatile float last_motor_a_speed = 0; // Global variable to store the last speed of motor A
lv_obj_t * bar_motor_a = NULL;
volatile float last_motor_b_speed = 0; // Global variable to store the last speed of motor B
lv_obj_t * bar_motor_b = NULL;

 lv_obj_t *arc_dial = NULL;
lv_obj_t *arc_label = NULL;

// Animation frames for the animation
const lv_image_dsc_t * anim_frames[] = {
    &animation00, &animation01, &animation02, &animation03, &animation04, &animation05, &animation06, &animation07,
    &animation08, &animation09, &animation10, &animation11, &animation12, &animation13, &animation14, &animation15,
    &animation16, &animation17, &animation18, &animation19, &animation20, &animation21, &animation22, &animation23,
    &animation24, &animation25, &animation26, &animation27, &animation28, &animation29, &animation30, &animation31,
    &animation32, &animation33, &animation34, &animation35, &animation36, &animation37
};
const uint8_t anim_frame_count = sizeof(anim_frames) / sizeof(anim_frames[0]);
static lv_obj_t * anim_img = NULL;
static uint8_t anim_frame_idx = 0;

static void anim_img_timer_cb(lv_timer_t * timer) {
    anim_frame_idx = (anim_frame_idx + 1) % anim_frame_count;
    lv_image_set_src(anim_img, anim_frames[anim_frame_idx]);
}

static uint32_t my_tick(void) {

	return to_ms_since_boot(get_absolute_time());
}


//Motor Functions

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

        // Store the last speed for display updates
        if (pwm_pin == MOTOR_A_PWM_PIN) {
            last_motor_a_speed = speed;
        } else if (pwm_pin == MOTOR_B_PWM_PIN) {
            last_motor_b_speed = speed;
        }   

}


void lv_dashboard_set(void)
{
    // Main container (screen)
    lv_obj_t * cont = lv_obj_create(lv_screen_active());
    lv_obj_set_size(cont, 470, 310);
    lv_obj_center(cont);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN); // vertical stacking

    // Top bar (20px high, full width)
    lv_obj_t * top_bar = lv_obj_create(cont);
    lv_obj_set_size(top_bar, 470, 20);
    lv_obj_set_style_bg_color(top_bar, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_border_width(top_bar, 0, 0);

    // Row container for columns
    lv_obj_t * row = lv_obj_create(cont);
    lv_obj_set_size(row, 470, 290); // Remaining height
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW); // horizontal stacking

    // First column (120 wide)
    lv_obj_t * col1 = lv_obj_create(row);
    lv_obj_set_size(col1, 120, 290);
    lv_obj_set_style_border_width(col1, 0, 0);

    // Second column (120 wide)
    lv_obj_t * col2 = lv_obj_create(row);
    lv_obj_set_size(col2, 120, 290);
    lv_obj_set_style_border_width(col2, 0, 0);

    // Third column (160 wide), split into two rows
    lv_obj_t * col3 = lv_obj_create(row);
    lv_obj_set_size(col3, 160, 290);
    lv_obj_set_style_border_width(col3, 0, 0);
    lv_obj_set_flex_flow(col3, LV_FLEX_FLOW_COLUMN);
// Disable scrolling and scrollbar for col3
lv_obj_set_scroll_dir(col3, LV_DIR_NONE);
lv_obj_set_scrollbar_mode(col3, LV_SCROLLBAR_MODE_OFF);
    // Top cell in third column
    lv_obj_t * col3_row1 = lv_obj_create(col3);
    lv_obj_set_size(col3_row1, 160, 130); // Half of 290
    lv_obj_set_style_border_width(col3_row1, 0, 0);

    // Bottom cell in third column
    lv_obj_t * col3_row2 = lv_obj_create(col3);
    lv_obj_set_size(col3_row2, 160, 140);
    lv_obj_set_style_border_width(col3_row2, 0, 0);

    // Add a label to the top bar
    lv_obj_t * label = lv_label_create(top_bar);
    lv_label_set_text(label, "The Open Source Underwater Vehicle");
    lv_obj_center(label);

// Create a vertical bar in col1
bar_motor_a = lv_bar_create(col1);
lv_obj_set_size(bar_motor_a, 40, 200); // width, height
lv_obj_center(bar_motor_a);
lv_bar_set_range(bar_motor_a, -4095, 4095); // -100% to +100%
lv_bar_set_value(bar_motor_a, 0, LV_ANIM_OFF); // Start at 0

// Add a label for the bar
lv_obj_t * bar_label = lv_label_create(col1);
lv_label_set_text(bar_label, "Motor A");
lv_obj_align(bar_label, LV_ALIGN_TOP_MID, 0, 5);

// Create a vertical bar in col2
bar_motor_b = lv_bar_create(col2);
lv_obj_set_size(bar_motor_b, 40, 200); // width, height
lv_obj_center(bar_motor_b);
lv_bar_set_range(bar_motor_b, -4095, 4095); // -100% to +100%
lv_bar_set_value(bar_motor_b, 0, LV_ANIM_OFF); // Start at 0

// Add a label for the bar
lv_obj_t * bar_label_b = lv_label_create(col2);
lv_label_set_text(bar_label_b, "Motor B");
lv_obj_align(bar_label_b, LV_ALIGN_TOP_MID, 0, 5);

// Set flex layout for col3_row1
lv_obj_set_flex_flow(col3_row1, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(col3_row1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
lv_obj_set_style_pad_all(col3_row1, 0, 0);

// Add the label (first child, will be at the top)
lv_obj_t * bar_label_z = lv_label_create(col3_row1);
lv_label_set_text(bar_label_z, "Dive Motor");
lv_obj_set_style_pad_top(bar_label_z, 5, 0);
lv_obj_set_style_pad_bottom(bar_label_z, 5, 0);
lv_obj_set_width(bar_label_z, LV_PCT(100));
lv_obj_set_style_text_align(bar_label_z, LV_TEXT_ALIGN_CENTER, 0);

// Add the arc dial (second child, will be below the label)
arc_dial = lv_arc_create(col3_row1);
lv_obj_set_size(arc_dial, 100, 100);
lv_obj_set_style_align(arc_dial, LV_ALIGN_CENTER, 0);

// Set arc range: -100 to 100, with 0 in the middle
lv_arc_set_range(arc_dial, -100, 100);
lv_arc_set_value(arc_dial, 0); // Start at 0

// Optional: Style the arc
lv_obj_set_style_arc_width(arc_dial, 12, LV_PART_INDICATOR);
lv_obj_set_style_arc_color(arc_dial, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);

// Add a label to show the value
arc_label = lv_label_create(arc_dial);
lv_label_set_text(arc_label, "0%");
lv_obj_center(arc_label);

//Animation image in col3_row2
anim_img = lv_image_create(col3_row2);
lv_image_set_src(anim_img, anim_frames[0]);
lv_obj_set_align(anim_img, LV_ALIGN_LEFT_MID);
lv_obj_set_style_pad_all(col3_row2, 0, 0);
lv_obj_set_style_border_width(col3_row2, 0, 0);
lv_timer_create(anim_img_timer_cb, 100, NULL); // 100 ms interval for animation


}

int main()
{
    stdio_init_all();
    printf("Booting...\n");
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
    // SPI initialisation and other screen pins.
 
      spi_init(spi1, 40000*1000);
    gpio_set_function(SCREEN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SCREEN_CLK,  GPIO_FUNC_SPI);
    gpio_set_function(SCREEN_MOSI, GPIO_FUNC_SPI);
    
    gpio_init(SCREEN_CS);
    gpio_set_dir(SCREEN_CS, GPIO_OUT);
    gpio_put(SCREEN_CS, 1);

    gpio_init(SCREEN_RESET);
    gpio_set_dir(SCREEN_RESET, GPIO_OUT);
    gpio_put(SCREEN_RESET, 1);

    gpio_init(SCREEN_DC_RS);
    gpio_set_dir(SCREEN_DC_RS, GPIO_OUT);
    gpio_put(SCREEN_DC_RS, 1);

    gpio_init(SCREEN_LED);
    gpio_set_dir(SCREEN_LED, GPIO_OUT);
    gpio_put(SCREEN_LED, 1);

    gpio_put(SCREEN_CS, 0);

    // Initialise LVGL
        lv_init();
        lv_tick_set_cb(my_tick);
        screen_init();
        screen_clear(0xf800);
        lv_display_t * display1 = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
        if (display1 == NULL) {
            printf("Failed to create display\n");
            return -1;
        }
//lv_display_set_draw_buffers(display1, buf1, NULL);
//lv_display_set_render_mode(display1, LV_DISPLAY_RENDER_MODE_PARTIAL);
        lv_display_set_buffers(display1, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
        lv_display_set_flush_cb(display1, my_flush_cb);   
// lv_display_set_draw_buffers(display1, buf1, NULL);
// lv_display_set_render_mode(display1, LV_DISPLAY_RENDER_MODE_PARTIAL);
        lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFF0000), 0);
        lv_obj_invalidate(lv_screen_active());
        lv_dashboard_set();

    // static lv_style_t style_indic;
    // lv_obj_t * bar = lv_bar_create(lv_screen_active());
    // lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
    // lv_obj_set_size(bar, 20, 200);
    // lv_obj_center(bar);
    // lv_bar_set_range(bar, 0, 4095);


    while (1) {
        adc_select_input(3);  //Joystick A x
        uint adc_x_raw = adc_read();
        adc_select_input(4);  //Joystick A y
        uint adc_y_raw = adc_read();
        adc_select_input(1);  //Joystick B y
        uint adc_z_raw = adc_read();

        //printf("X: %d, Y: %d, Z: %d\n", adc_x_raw, adc_y_raw, adc_z_raw);
    //Normalise joystick input -1 to 1
        float adc_x_norm = ((float)adc_x_raw -2105)/2105;
        float adc_y_norm = ((float)adc_y_raw -2105)/2105;

        // https://xiaoxiae.github.io/Robotics-Simplified-Website/drivetrain-control/arcade-drive/
        float max_speed = fmax(fabsf(adc_x_norm), fabsf(adc_y_norm));
        float tot_speed = adc_x_norm + adc_y_norm;
        float diff_speed = adc_x_norm - adc_y_norm;
        //printf("%f, %f, %f\n", max_speed, tot_speed, diff_speed );
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
// Assuming 'bar_motor_a' and 'bar_motor_b' are accessible (make them global or static, or pass them as needed)
lv_bar_set_value(bar_motor_a, (int)(last_motor_a_speed * 4095), LV_ANIM_OFF);
lv_bar_set_value(bar_motor_b, (int)(last_motor_b_speed * 4095), LV_ANIM_OFF);

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

// Map duty_cycle_z to -100..100, with 0 at 2110
// Example normalization if adc_z_raw is 0..4095, 2110 is center
float duty_cycle_z = (((float)adc_z_raw - 2110) / 2110.0f) * -1.0f; // Normalize to -1.0 to 1.0
if (duty_cycle_z > 1.0f) duty_cycle_z = 1.0f;
if (duty_cycle_z < -1.0f) duty_cycle_z = -1.0f;
int arc_value = (int)(duty_cycle_z * 100.0f); // duty_cycle_z should be -1..1

lv_arc_set_value(arc_dial, arc_value);
lv_label_set_text_fmt(arc_label, "%d%%", arc_value);

        lv_timer_handler(); // Call the LVGL timer handler to process events
        sleep_ms(100); // Sleep for a short time to allow LVGL to process events


    }

  

}