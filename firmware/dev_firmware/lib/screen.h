#ifndef SCREEN_H
#define SCREEN_H

#include "lvgl.h"

void screen_init(void);
void screen_reset(void);
void spi_write(unsigned char d);
void write_command(unsigned char command);
void write_data(unsigned char data);
void write_command_and_data(unsigned char command, unsigned char data);
void screen_address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);
void screen_clear(unsigned int j);
void my_flush_cb (lv_display_t * disp, const lv_area_t * area, uint8_t * color_p);

#endif
