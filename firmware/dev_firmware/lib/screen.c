#include "screen.h"
#include <stdio.h>
#include "./pico/stdlib.h"
#include "hardware/spi.h"
#include "lvgl.h"
#include "pins.h"

void spi_write(unsigned char d)
{
    uint8_t out_buffer[1];
    int read_low = spi_write_read_blocking(spi1, (uint8_t[]){d}, out_buffer, 1);
}

void write_command(unsigned char command)  
{   
    gpio_put(SCREEN_DC_RS, 0);
    spi_write(command);

//   *(portOutputRegister(digitalPinToPort(RS))) &=  ~digitalPinToBitMask(RS);//LCD_RS=0;
//   Lcd_Writ_Bus(VH);
}

void write_data(unsigned char data)
{
    gpio_put(SCREEN_DC_RS, 1);
    spi_write(data);

//   *(portOutputRegister(digitalPinToPort(RS)))|=  digitalPinToBitMask(RS);//LCD_RS=1;
//   Lcd_Writ_Bus(VH);
}

void write_command_and_data(unsigned char command, unsigned char data)
{
    write_command(command);
    write_data(data);
}

void screen_init()
{
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

    screen_reset();

    gpio_put(SCREEN_CS, 0);

    write_command(0xF7);
    write_data(0xA9);
    write_data(0x51);
    write_data(0x2C);
    write_data(0x82);

    write_command(0xC0);
    write_data(0x11);
    write_data(0x09);

    write_command(0xC1);
    write_data(0x41);

    write_command(0xC5);
    write_data(0x00);
    write_data(0x0A);
    write_data(0x80);

    write_command(0xB1);
    write_data(0xB0);
    write_data(0x11);

    write_command(0xB4);
    write_data(0x02);

    write_command(0xB6);
    write_data(0x02);
    write_data(0x22);

    write_command(0xB7);
    write_data(0xC6);

    write_command(0xBE);
    write_data(0x00);
    write_data(0x04);

    write_command(0xE9);
    write_data(0x00);

    write_command(0x36);
    write_data(0x68);   // rotation

    write_command(0x3A);
    write_data(0x66);

    write_command(0xE0);
    write_data(0x00);
    write_data(0x07);
    write_data(0x10);
    write_data(0x09);
    write_data(0x17);
    write_data(0x0B);
    write_data(0x41);
    write_data(0x89);
    write_data(0x4B);
    write_data(0x0A);
    write_data(0x0C);
    write_data(0x0E);
    write_data(0x18);
    write_data(0x1B);
    write_data(0x0F);

    write_command(0xE1);
    write_data(0x00);
    write_data(0x17);
    write_data(0x1A);
    write_data(0x04);
    write_data(0x0E);
    write_data(0x06);
    write_data(0x2F);
    write_data(0x45);
    write_data(0x43);
    write_data(0x02);
    write_data(0x0A);
    write_data(0x09);
    write_data(0x32);
    write_data(0x36);
    write_data(0x0F);

    write_command(0x11);    //Exit Sleep
    sleep_ms(120);
    write_command(0x29);    //Display on

    gpio_put(SCREEN_CS, 1);
}

void screen_reset()
{
    gpio_put(SCREEN_RESET, 0);
    sleep_ms(5);
    gpio_put(SCREEN_RESET, 1);
    sleep_ms(15);
}

void screen_clear(unsigned int j)                   
{	
  unsigned int i,m;
  gpio_put(SCREEN_CS, 0);
  screen_address_set(0,0,479,319);
  for(i=0;i<320;i++)
    for(m=0;m<480;m++)
    {
      write_data((j>>8)&0xF8);
      write_data((j>>3)&0xFC);
      write_data(j<<3);
    }
  gpio_put(SCREEN_CS, 1);
}

void screen_address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{
    write_command(0x2a);
	write_data(x1>>8);
	write_data(x1);
	write_data(x2>>8);
	write_data(x2);
    write_command(0x2b);
	write_data(y1>>8);
	write_data(y1);
	write_data(y2>>8);
	write_data(y2);
	write_command(0x2c);
}


void my_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
    uint16_t x1 = area->x1;
    uint16_t y1 = area->y1;
    uint16_t x2 = area->x2;
    uint16_t y2 = area->y2;
    //    printf("In the flush callback - ");
    //    printf("Area: x1=%d, y1=%d, x2=%d, y2=%d\n", x1, y1, x2, y2);
    // Set the address window for the display
        gpio_put(SCREEN_CS, 0);
    screen_address_set(x1, y1, x2, y2);
    // printf("Address set for area: x1=%d, y1=%d, x2=%d, y2=%d\n", x1, y1, x2, y2);
    // printf("Flush area: %d x %d\n", (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1));
    gpio_put(SCREEN_CS, 0);
    size_t px_count = (x2 - x1 + 1) * (y2 - y1 + 1);
    // printf("Pixels to write: %zu\n", px_count);


    uint16_t * color = (uint16_t *)color_p;
    for (size_t i = 0; i < px_count; i++) {
        uint16_t c = *color++;
        uint8_t r = (c >> 11) & 0x1F;
        uint8_t g = (c >> 5) & 0x3F;
        uint8_t b = c & 0x1F;
        r = r << 3;
        g = g << 2;
        b = b << 3;
        write_data(r);
        write_data(g);
        write_data(b);
    }
// uint16_t * color = (uint16_t *)color_p;
//     for (uint32_t y = y1; y <= y2; y++) {
//         for (uint32_t x = x1; x <= x2; x++) {
//         uint16_t c = *color++;
//             //color++;
//             uint8_t r = (c >> 11) & 0x1F;
//             uint8_t g = (c >> 5) & 0x3F;
//             uint8_t b = c & 0x1F;
//             r = r << 3;
//             g = g << 2;
//             b = b << 3;
//             write_data(r);
//             write_data(g);
//             write_data(b);
//         }
//     }

// for (uint32_t y = y1; y <= y2; y++) {
//     for (uint32_t x = x1; x <= x2; x++) {
//         uint8_t r = ((x/20 + y/20) % 2) ? 0xF8 : 0x00;
//         uint8_t g = ((x/20 + y/20) % 2) ? 0xFC : 0x00;
//         uint8_t b = ((x/20 + y/20) % 2) ? 0xF8 : 0x00;
//         write_data(r);
//         write_data(g);
//         write_data(b);
//     }
// }

    gpio_put(SCREEN_CS, 1);

    // IMPORTANT: Inform LVGL flushing is done
    lv_disp_flush_ready(disp);
}