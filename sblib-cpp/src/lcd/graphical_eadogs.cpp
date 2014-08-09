/*
 *  graphical_eadogs.cpp - class for EA-DOGS graphical LCD displays.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/lcd/graphical_eadogs.h>
#include <sblib/digital_pin.h>

// Display width in pixels
#define DISPLAY_WIDTH 102

// Display height in bytes
#define DISPLAY_HEIGHT 8


/*
 * UC1701 programming commands
 */

// Set column address bits 0..3: bits 0-3
#define CMD_SET_COL_ADDR_LSB  0x00

// Set column address bits 4..7: bits 0-3
#define CMD_SET_COL_ADDR_MSB  0x10

// Power control: bit0: booster, bit1: regulator, bit2: follower
#define CMD_POWER_CTRL        0x28

// Set the scroll line: bits 0-4
#define CMD_SCROLL_LINE       0x40

// Set the page address: bits 0-3
#define CMD_PAGE_ADDR         0xb0

// Configure the internal resistor ratio: bits 0..2
#define CMD_RESISTOR_RATIO    0x20

// Adjust the contrast of the LCD panel: bits 0..5
#define CMD_CONTRAST          0x81

// Show the SRAM contents
#define CMD_PIXELS_NORMAL     0xa4

// Set all pixels to on
#define CMD_PIXELS_ALL_ON     0xa5

// Set display to normal mode
#define CMD_DISP_NORMAL       0xa6

// Set display to inverse mode
#define CMD_DISP_INVERSE      0xa7

// Disable the display (sleep mode)
#define CMD_DISABLE           0xae

// Enable the display (recover from sleep mode)
#define CMD_ENABLE            0xaf

// Set normal segment direction (SEG 0..131)
#define CMD_SEG_DIR_NORMAL    0xa0

// Set mirrored segment direction (SEG 131..0)
#define CMD_SEG_DIR_MIRROR    0xa1

// Set normal COM direction (COM 0..63)
#define CMD_COM_DIR_NORMAL    0xc0

// Set mirrored COM direction (COM 63..0)
#define CMD_COM_DIR_MIRROR    0xc8

// Reset the display
#define CMD_RESET             0xe2

// Set LCD bias ratio to 1/9
#define CMD_BIAS_RATIO_1_9    0xa2

// Set LCD bias ratio to 1/7
#define CMD_BIAS_RATIO_1_7    0xa3

// Set advanced program control 0
#define CMD_ADV_CONTROL       0xfa



LcdGraphicalEADOGS::LcdGraphicalEADOGS(int spiPort, int pinData, int pinClock, int pinCD, int pinCS,
    const Font& fnt)
:LcdGraphical(fnt, DISPLAY_WIDTH, DISPLAY_HEIGHT)
,spi(spiPort)
,pinCD(pinCD)
,pinCS(pinCS)
{
    pinMode(pinData,  OUTPUT | SPI_MOSI);
    pinMode(pinClock, OUTPUT | SPI_CLOCK);
    pinMode(pinCD, OUTPUT);

    if (pinCS)
        pinMode(pinCS,  OUTPUT | SPI_SSEL);
}

void LcdGraphicalEADOGS::begin()
{
    spi.setClockDivider(128);
    spi.begin();

    digitalWrite(pinCD, true);
    spi.transfer(CMD_SCROLL_LINE);
    spi.transfer(CMD_SEG_DIR_MIRROR);
    spi.transfer(CMD_COM_DIR_NORMAL);
    spi.transfer(CMD_PIXELS_NORMAL);
    spi.transfer(CMD_DISP_INVERSE);
    spi.transfer(CMD_BIAS_RATIO_1_9);
    spi.transfer(CMD_POWER_CTRL | 3); // booster, regulator, follower on
    spi.transfer(CMD_CONTRAST);
    spi.transfer(0x10); // the contrast value
    spi.transfer(CMD_ADV_CONTROL);
    spi.transfer(0x90);  // temperature compensation -0.11% / °C
    spi.transfer(CMD_ENABLE);
    digitalWrite(pinCD, false);
}

void LcdGraphicalEADOGS::end()
{
    spi.end();
}

void LcdGraphicalEADOGS::pos(int x, int y)
{
}
