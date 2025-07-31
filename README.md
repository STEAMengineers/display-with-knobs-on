# display-with-knobs-on

Think [cheap yellow display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) but not necessarily yellow and with two joystick inputs and four channels of brushed motor output. Originally designed to control [The open source underwater vehicle](https://github.com/concretedog/Tiny-Opensource-Underwater-Vehicle-TOUV-) project. 

## Features

* [RP2350b](https://datasheets.raspberrypi.com/rp2350/rp2350-product-brief.pdf) - Dual Arm Cortex-M33 or dual Hazard3 RISC-V processors
@ 150MHz
* [W25Q128JV-DTR](https://www.winbond.com/resource-files/w25q128jv_dtr%20revc%2003272018%20plus.pdf) - 16Mb primary flash 
* [MSP3520 screen](https://www.lcdwiki.com/3.5inch_SPI_Module_ILI9488_SKU%3AMSP3520) - ILI9488 powered 320x480 LCD screen with resistive touch and SD card slot (Headers)
* 2 x [TB6612FNG](https://toshiba.semicon-storage.com/us/product/linear/motordriver/detail.TB6612FNG.html) - Four channels of brushed motor outputs
* [IP5310](https://www.injoinic.com/api//static/uploads/20250528/20250528180445_6836dfbd54166.pdf) - Battery charger management with 3A boost output @ 5V for motors
* 2 x Joystick - Two axis analoge joysticks with push button inputs

  ## GPIO

  ### Motors

  A
  * IN1 - GPIO29
  * IN2 - GPIO28
  * PWM - GPIO24

  B
  * IN1 - GPIO31
  * IN2 - GPIO32
  * PWM - GPIO25

  STANDBY - GPIO30

  C
  * IN1 - GPIO34
  * IN2 - GPIO33
  * PWM - GPIO26

  D
  * IN1 - GPIO36
  * IN2 - GPIO37
  * PWM - GPIO27

  STANDBY - GPIO35

  ### Joysticks
  A
  * X - GPIO43
  * Y - GPIO44
  * Button - GPIO39
 
  B
  * X - GPIO41
  * Y - GPIO42
  * Button - GPIO38
 
  ### Screen

  Display
  * CS - GPIO13
  * RESET - GPIO18
  * DC/RS - GPIO17
  * MOSI - GPIO11
  * MISO - GPIO12
  * SCK - GPIO14
  * LED - GPIO16
 
  Touch
  * CS - GPIO10
  * IRQ - GPIO15
  * MOSI - GPIO11
  * MISO - GPIO12
  * SCK - GPIO14
 
  SD card
  * CS - GPIO21
  * SCK - GPIO22
  * MOSI - GPIO19
  * MISO - GPIO20



   


