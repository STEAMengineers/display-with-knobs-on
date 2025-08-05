#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "math.h"
#include "lib/screen.h"
#include "lvgl/lvgl.h"
#include "lib/anim_frames.h"
#include "motors.h"
#include "pins.h"

//#include "lib/ili9488.h"

//TODO -SCREEN

//Improve GUI
//  -Add image to controls
//  -Add animation picture frame


#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320
// Buffer for the display
#define LV_COLOR_SIZE 2

static uint8_t buf1[DISPLAY_WIDTH * 320 * LV_COLOR_SIZE];


//float deadzone = 0.05f; // Deadzone for motor control
// Global variable to store the last speed of motor A
lv_obj_t * bar_motor_a = NULL;
// Global variable to store the last speed of motor B
lv_obj_t * bar_motor_b = NULL;

lv_obj_t *scale = NULL;
lv_obj_t *arc_label = NULL;
static lv_obj_t * needle_line;

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

void lv_dashboard_set(void)
{
    // Main container (screen)
    lv_obj_t * cont = lv_obj_create(lv_screen_active());
    lv_obj_set_size(cont, 480, 320);
    lv_obj_center(cont);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN); // vertical stacking
    lv_obj_set_style_bg_img_src(cont, &brushedsteelplainwithrivets, 0);

    // Top bar (20px high, full width)
    lv_obj_t * top_bar = lv_obj_create(cont);
    lv_obj_set_size(top_bar, 470, 20);
    lv_obj_set_style_bg_color(top_bar, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_border_width(top_bar, 0, 0);
    lv_obj_set_style_bg_opa(top_bar, LV_OPA_TRANSP, 0); // Transparent background

    // Row container for columns
    lv_obj_t * row = lv_obj_create(cont);
    lv_obj_set_size(row, 470, 300); // Remaining height
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW); // horizontal stacking
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0); // Make background transparent

    // First column (120 wide)
    lv_obj_t * col1 = lv_obj_create(row);
    lv_obj_set_size(col1, 120, 290);
    lv_obj_set_style_border_width(col1, 0, 0);
    lv_obj_set_style_bg_opa(col1, LV_OPA_TRANSP, 0); // Transparent background

    // Second column (120 wide)
    lv_obj_t * col2 = lv_obj_create(row);
    lv_obj_set_size(col2, 120, 290);
    lv_obj_set_style_border_width(col2, 0, 0);
    lv_obj_set_style_bg_opa(col2, LV_OPA_TRANSP, 0); // Transparent background

    // Third column (160 wide), split into two rows
    lv_obj_t * col3 = lv_obj_create(row);
    lv_obj_set_size(col3, 160, 290);
    lv_obj_set_style_border_width(col3, 0, 0);
    lv_obj_set_flex_flow(col3, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(col3, LV_OPA_TRANSP, 0); // Transparent background

    // Disable scrolling and scrollbar for col3
    lv_obj_set_scroll_dir(col3, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(col3, LV_SCROLLBAR_MODE_OFF);

    // Top cell in third column
    lv_obj_t * col3_row1 = lv_obj_create(col3);
    lv_obj_set_size(col3_row1, 150, 140); // Half of 290
    lv_obj_set_style_border_width(col3_row1, 0, 0);
    lv_obj_set_style_bg_opa(col3_row1, LV_OPA_TRANSP, 0); // Transparent background

    // Bottom cell in third column
    lv_obj_t * col3_row2 = lv_obj_create(col3);
    lv_obj_set_size(col3_row2, 170, 140);
    lv_obj_set_style_border_width(col3_row2, 0, 0);
    lv_obj_set_style_bg_opa(col3_row2, LV_OPA_TRANSP, 0); // Transparent background

    // Add a label to the top bar
    //lv_obj_t * label = lv_label_create(top_bar);
    //lv_label_set_text(label, "The Open Source Underwater Vehicle");
    //lv_obj_center(label);

    // Create a vertical bar in col1
    bar_motor_a = lv_slider_create(col1);
    lv_obj_set_size(bar_motor_a, 20, 200); // width, height
    lv_obj_center(bar_motor_a);
    lv_slider_set_range(bar_motor_a, -4095, 4095); // -100% to +100%
    lv_slider_set_value(bar_motor_a, 0, LV_ANIM_OFF); // Start at 0

    static lv_style_t style_main;
    static lv_style_t knob_style;
    static lv_style_t style_indicator;

    lv_style_init(&style_main);
    lv_style_set_bg_color(&style_main, lv_color_hex(0x000000));
    lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
    lv_style_set_border_width(&style_main, 0);
    lv_style_set_radius(&style_main, 2);
    lv_style_set_width(&style_main, 10); // Set bar width
    lv_style_set_shadow_width(&style_main, 2);
    lv_obj_add_style(bar_motor_a, &style_main, LV_PART_MAIN);
 
    lv_style_init(&knob_style);
    //lv_style_set_bg_color(&knob_style, lv_palette_main(LV_PALETTE_RED));
    //lv_style_set_bg_grad_color(&knob_style, lv_palette_lighten(LV_PALETTE_RED, 2));
    //lv_style_set_bg_grad_dir(&knob_style, LV_GRAD_DIR_VER);
    //lv_style_set_radius(&knob_style, LV_RADIUS_CIRCLE); // Make it round if desired
    lv_style_set_pad_top(&knob_style, -25); //Makes no sense but it works
    lv_style_set_width(&knob_style, 50);  // Set knob width
    lv_style_set_height(&knob_style, 42); // Set knob height
    lv_style_set_bg_opa(&knob_style, 0);
    lv_style_set_bg_image_src(&knob_style, &slot_knob_small);
    lv_obj_add_style(bar_motor_a, &knob_style, LV_PART_KNOB);

    //Hide indicator
    lv_obj_set_style_opa(bar_motor_a, LV_OPA_TRANSP, LV_PART_INDICATOR);

    // LV_IMAGE_DECLARE(slot_knob_small);
    // lv_image_set_src(bar_motor_a, &slot_knob_small);

    lv_obj_center(bar_motor_a);

    // Add a label for the bar
    lv_obj_t * bar_label = lv_label_create(col1);
    lv_label_set_text(bar_label, "Motor A");
    lv_obj_align(bar_label, LV_ALIGN_TOP_MID, 0, 5);

    // Create a vertical bar in col2
    bar_motor_b = lv_slider_create(col2);
    lv_obj_set_size(bar_motor_b, 20, 200); // width, height
    lv_obj_center(bar_motor_b);
    lv_bar_set_range(bar_motor_b, -4095, 4095); // -100% to +100%
    lv_slider_set_value(bar_motor_b, 0, LV_ANIM_OFF); // Start at 0
    lv_obj_add_style(bar_motor_b, &style_main, LV_PART_MAIN);
    lv_obj_add_style(bar_motor_b, &knob_style, LV_PART_KNOB);
  //Hide indicator
    lv_obj_set_style_opa(bar_motor_b, LV_OPA_TRANSP, LV_PART_INDICATOR);
  

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

// Add the scale widget (second child, will be below the label)
scale = lv_scale_create(col3_row1);

//Animation image in col3_row2
anim_img = lv_image_create(col3_row2);
lv_image_set_src(anim_img, anim_frames[0]);
//lv_obj_set_align(anim_img, LV_ALIGN_TOP_MID);
lv_obj_set_pos(anim_img, 0, 0);    /* Or in one function */
lv_obj_set_style_pad_all(col3_row2, 0, 0);
lv_obj_set_style_border_width(col3_row2, 0, 0);
lv_timer_create(anim_img_timer_cb, 100, NULL); // 100 ms interval for animation

}

int main()
{
    stdio_init_all();
    printf("Booting...\n");
    printf("Pico SDK Version: %s\n", PICO_SDK_VERSION_STRING);
    printf("========================================\n");
    printf("Display with Knobs on Firmware\n");
    printf("========================================\n");

    printf("Initialising GPIOs...\n");
    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(41);
    adc_gpio_init(42);
    adc_gpio_init(43);
    adc_gpio_init(44);

    printf("Initialising motors...\n");
    initialise_motors();
    
    printf("Initialising LVGL...\n");
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
    lv_display_set_buffers(display1, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display1, my_flush_cb); 
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x888888), 0);
    lv_obj_invalidate(lv_screen_active());
    lv_dashboard_set();

    while (1) {
        adc_select_input(4);  //Joystick A x
        uint adc_x_raw = adc_read();
        adc_select_input(3);  //Joystick A y
        uint adc_y_raw = adc_read();
        adc_select_input(1);  //Joystick B y
        uint adc_z_raw = adc_read();

        //printf("X: %d, Y: %d, Z: %d\n", adc_x_raw, adc_y_raw, adc_z_raw);
        float adc_x_norm = (((float)adc_x_raw -2105)/2105) * -1.0f; // Normalize to -1.0 to 1.0
        float adc_y_norm = (((float)adc_y_raw -2105)/2105) * -1.0f; // Normalize to -1.0 to 1.0

        float left, right;
        differential_drive(adc_x_norm, adc_y_norm, deadzone, &left, &right); // 0.05f is a typical deadzone

        motor_set_speed(MOTOR_A_PWM_PIN, MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, left);
        motor_set_speed(MOTOR_B_PWM_PIN, MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN, right);

        lv_bar_set_value(bar_motor_a, (int)(last_motor_a_speed * 4095), LV_ANIM_OFF);
        lv_bar_set_value(bar_motor_b, (int)(last_motor_b_speed * 4095), LV_ANIM_OFF);

        // Normalize adc_z_raw to -1.0 .. 1.0 (centered at 2105)
        float adc_z_norm = (((float)adc_z_raw - 2105) / 2105.0f) * -1.0f;
        if (adc_z_norm > 1.0f) adc_z_norm = 1.0f;
        if (adc_z_norm < -1.0f) adc_z_norm = -1.0f;

        dive_motor_drive(adc_z_norm, deadzone); // 0.05f is a typical deadzone
        
        // Calculate the angle for the needle based on scale_value (-100 to 100 mapped to 135 to 405 degrees)
        int scale_value;
        if(fabsf(adc_z_norm) < deadzone) {
            scale_value = 0;
        } else {
            scale_value = (int)roundf(adc_z_norm * 100.0f);
        }
//lv_label_set_text_fmt(arc_label, "%d%%", scale_value);

// Map scale_value (-100 to 100) to angle (135 to 405)
//float angle = 135 + ((float)(scale_value + 100) / 200.0f) * 270.0f;
//lv_obj_set_style_transform_angle(needle_line, angle * 10, 0); // LVGL uses 0.1 deg units
        lv_timer_handler(); // Call the LVGL timer handler to process events
        sleep_ms(100); // Sleep for a short time to allow LVGL to process events


    }
    return 0;
}