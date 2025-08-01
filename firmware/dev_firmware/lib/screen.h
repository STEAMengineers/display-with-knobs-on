#ifndef SCREEN_H
#define SCREEN_H

#define SCREEN_CS 13
#define SCREEN_RESET 18
#define SCREEN_DC_RS 17
#define SCREEN_MOSI 11
#define SCREEN_CLK 14
#define SCREEN_LED 16
#define SCREEN_MISO 12

void screen_init(void);
void screen_reset(void);
void spi_write(unsigned char d);
void write_command(unsigned char command);
void write_data(unsigned char data);
void write_command_and_data(unsigned char command, unsigned char data);
void screen_address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);
void screen_clear(unsigned int j);

#endif
