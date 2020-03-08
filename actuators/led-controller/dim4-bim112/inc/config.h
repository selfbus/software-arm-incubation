/*
 * config.h
 *
 *  Created on: 27.01.2018
 *      Author: uwe223
 */

#ifndef CONFIG_H_
#define CONFIG_H_



#define MANUFACTURER 0x83		// MDT

typedef struct
{
    unsigned int noOfChannels;  //!> how many channels are supported with this hardware
    unsigned short deviceType;  //!> bcu.begin devicetype
    unsigned short appVersion;  //!> application version
    byte hardwareVersion[6];    //!> The hardware identification number (see selfbus wiki "Verwendung der sblib")
    const char *version;
} HardwareVersion;

extern const HardwareVersion * currentVersion;

const HardwareVersion hardwareVersion[2] =
{
{ 4, 0x003D, 0x14, { 0, 0, 0, 0, 0x02, 0x3D }, "AKD-0424V.01 RGBW LED Controller" },
{ 4, 0x003D, 0x22, { 0, 0, 0, 0, 0x03, 0x3D }, "AKD-0424V.02 RGBW LED Controller" },
};

//#ifdef LED4
#define HARDWARE_ID 1			//TODO test
#define PWM
//#endif



#endif /* CONFIG_H_ */
