/*
 *  OneWire Gateway Applikation for LPC1115
 *
 *  Copyright (C) 2021-2024 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef KNXPRODEDITORHEADER_ONEWIREGATEWAY_H
#define KNXPRODEDITORHEADER_ONEWIREGATEWAY_H

// Maximale Anzahl von OneWire Geräten für die App pro Kanal
// Die Anzahl muss gleich der Anzahl MAX_DS_DEVICES aus der ds18x20_onewire_ds2482.h sein
#define MAX_NUMBER_OF_ONEWIRE_DEVICES	5

// Anzahl der OneWire Kanäle
// 2TE OneWire Gateway: 2 Kanäle á 10 OneWire Geräte
// 4TE OneWire Gateway: 4 Kanäle á 5 OneWire Geräte
#define NUMBER_OF_ONEWIRE_CHANNELS 		4 // 2 oder 4

#define EE_GENERAL_SETTINGS				 0x44FF
#define EE_FIRST_ONEWIRE_ADRESS        	 0x4500
#define EE_SIZE_OF_ONEWIRE_ACSII_ADDRESS 0x0010
#define EE_FIRST_ONEWIRE_CONFIG_BYTE   	 0x4510
#define EE_FIRST_CYCLIC_TIME_BYTE		 0x4511
#define EE_FIRST_VALUE_CHANGE_VALUE		 0x4512
#define EE_FIRST_VALUE_CORRECT_VALUE	 0x4514
#define EE_SIZE_OF_EACH_ONEWIRE_DEVICE   0x0016

#define TIMER32_0_STEP 10000 //unit: ms Timer Interrupt Zeit


/**
 * Header File for DIY KNX Device
 * generated with KNXprodEditor 0.45
 *
 * Format:
 * MANUFACTURER = knxMaster->Manufacturer->KnxManufacturerId
 * DEVICETYPE = ApplicationProgram->ApplicationNumber
 * APPVERSION = ApplicationProgram->ApplicationVersion
 *
 * Parameter: EE_...
 * Simple parameter: EE_PARAMETER_[Name]   [Address in hex] //Addr:[Address in hex], Size:[Size in hex], Description in language 1
 * Union Parameter: EE_UNIONPARAMETER_[Address in hex]   [Address in hex] //Addr:[Address in hex], Size:[Size in hex], Description Parameter 1 in language 1, Description Parameter 2 in language 1, ...
 *
 * Communication objects: COMOBJ...
 * COMOBJ_[Description]_[Function text]   [Communication object number] //Com-Objekt number: [number] ,Description: [in language 1], Function [in language 1]
 *
 *
 *
 * Device information:
 * Device: OneWireGateway, 4TE, 4 Ch, 20 Devices
 * ApplicationProgram: M-004C_A-0034-10-4ABC
 *
 */


#define MANUFACTURER 76 //!< Manufacturer ID
#define DEVICETYPE 52 //!< Device Type
#define APPVERSION 16 //!< Application Version


#endif
