/*
 * config.h
 *
 *  Created on: 27.01.2018
 *      Author: uwe223
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <sblib/io_pin_names.h>
#include <sblib/digital_pin.h>
#include "app_in.h"

//Input logic level
//#define INVERT 			    //  ifndef: 0-->1 steigende Flanke ; pull_down
							    //  ifdef:  0-->1 fallende Flanke ; pull_up

#define MANUFACTURER 0x83		// MDT
//#define DEVICETYPE   0x0030	// Binäreingang
//#define APPVERSION   0x20		// Version 2.0
//#define DEVICETYPE   0x002E	// Tasterschnittstelle
//#define APPVERSION   0x16		// Version 1.6


//static const char APP_VERSION[] __attribute__((used)) = "Binary Input 1.1.21";
typedef struct
{
    unsigned int noOfChannels;  //!> how many channels are supported with this hardware
    bool ledOutputFunction;		//!> LED output for device "Tasterschnittstelle"
    unsigned short deviceType;  //!> bcu.begin devicetype
    unsigned short appVersion;  //!> application version
    unsigned short baseAddress; //!> Base address of the config parameters (0x4400 + Offset first param.(debounce-Time))
    unsigned short logicBaseAddress; //!> Base address of the logic parameters
    byte hardwareVersion[6];    //!> The hardware identification number (see selfbus wiki "Verwendung der sblib")
    const char *version;
} HardwareVersion;

extern const HardwareVersion * currentVersion;

const HardwareVersion hardwareVersion[7] =
{
{ 16, false, 0x0030, 0x20, 0x4574, 0x4868, { 0, 0, 0, 0, 0x00, 0x1E }, "Binary input 16f" },
{ 8,  false, 0x0031, 0x20, 0x44D4, 0x4650, { 0, 0, 0, 0, 0x01, 0x1E }, "Binary input 8f" },
{ 4,  false, 0x0032, 0x20, 0x4484, 0x4544, { 0, 0, 0, 0, 0x01, 0x1F }, "Binary input 4f" },
{ 6,  true,  0x002C, 0x16, 0x44E4, 0x4602, { 0, 0, 0, 0, 0x06, 0x1E }, "Binary input/LED 6f" },
{ 4,  true,  0x002E, 0x16, 0x44AC, 0x456C, { 0, 0, 0, 0, 0x04, 0x1E }, "Binary input/LED 4f" },
{ 2,  true,  0x002F, 0x16, 0x4474, 0x44D6, { 0, 0, 0, 0, 0x02, 0x1E }, "Binary input/LED 2f" },
{ 2,  false, 0x012F, 0x16, 0x4474, 0x44D6, { 0, 0, 0, 0, 0x01, 0x1E }, "Binary input 230VAC 2f" } };

#ifdef BI16
#define HARDWARE_ID 0
#elif defined BI8
#define HARDWARE_ID 1
#elif defined BI4
#define HARDWARE_ID 2
#elif defined TI6L
#define HARDWARE_ID 3
#elif defined TI4L
#define HARDWARE_ID 4
#elif defined TI2L
#define HARDWARE_ID 5
#elif defined TI2
#define HARDWARE_ID 6
#endif

//LED output pulse/blink time in ms
#define DELAYPULSEON  100
#define DELAYPULSEOFF 1400
#define DELAYBLINK    700

//Led_Indication leds(SPI_PORT_1, PIN_LT9, PIN_LT1, PIN_LT2, PIN_LT3);

// Input pins
const int inputPins[] =
{
#ifdef __LPC11UXX__
  PIN_PWM
, PIN_APRG
, PIN_IO1

, PIN_IO2
, PIN_IO3
, PIN_IO4
, PIO_SDA
, PIN_IO5

, PIN_IO14
, PIN_IO15
, PIN_IO13
, PIN_IO11

, PIN_IO9
, PIN_IO10
, PIN_TX
, PIN_RX

#elif defined TS_ARM
    PIO2_2,  //  A ; IO2
	PIO0_9,  //  B ; IO3
	PIO2_11, //  C ; IO4
	PIO1_1,  //  D ; IO5

	PIO3_0,  //  E ; IO6
	PIO3_1,  //  F ; IO7
	PIO3_2,  //  G ; IO8
	PIO0_5,  //  H ; IO17 -- pullup Widerstand auf TS-Arm beachten

	PIO2_9,  //  I ; IO9
	PIO0_8,  //  J ; IO10
	PIO1_10, //  K ; IO11
	PIO0_11, //  L ; IO12

	PIO1_0,  //  M ; IO13
	PIO1_2,  //  N ; IO14
	PIO2_3,  //  O ; IO15
	PIO1_5,  //  P ; IO16


#else
  PIO2_5, //  A0
  PIO3_5, //  A1
  PIO0_7, //  A2
  PIO2_9, //  A3

  PIO1_11, // B0
  PIO1_4, //  B1
  PIO1_2, //  B2
  PIO1_1, //  B3

  PIO1_7, //  C0
  PIO1_6, //  C1
  PIO1_5, //  C2
  PIO3_2, //  C3

  PIO2_8, //  D0
  PIO2_7, //  D1
  PIO2_6, //  D2
  PIO3_3, //  D3

#endif
};

#endif /* CONFIG_H_ */
