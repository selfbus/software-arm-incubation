/*
 *  Original written for LPC922:
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017 Oliver Stefan <o.stefan252@googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef rm_eeprom_h
#define rm_eeprom_h


//-----------------------------------------------------------------------------
// ETS VD Konfigurations-Parameter
//-----------------------------------------------------------------------------

/**
 * Intervall für Alarm zyklisch senden in Sekunden
 */
#define CONF_ALARM_INTERVAL 506

/**
 * Alarm verzögert senden in Sekunden.
 */
#define CONF_ALARM_DELAYED 504

/**
 * Intervall für Testalarm zyklisch senden in Sekunden.
 */
#define CONF_TALARM_INTERVAL 503

/**
 * Intervall für Status-Informationen zyklisch senden in Minuten
 */
#define CONF_INFO_INTERVAL 505

/**
 * Konfigurationsbyte für zyklisches und verzögertes Senden.
 */
#define CONF_SEND_ENABLE 507

/**
 * Bitmaske in CONF_SEND_ENABLE für Alarm verzögert senden.
 */
#define CONF_ENABLE_ALARM_DELAYED 0x80

/**
 * Bitmaske in CONF_SEND_ENABLE für Alarm zyklisch senden.
 */
#define CONF_ENABLE_ALARM_INTERVAL 0x40

/**
 * Bitmaske in CONF_SEND_ENABLE für Testalarm zyklisch senden.
 */
#define CONF_ENABLE_TALARM_INTERVAL 0x20

/**
 * Bitmaske in CONF_SEND_ENABLE für Status-Informationen zyklisch senden.
 */
#define CONF_ENABLE_INFO_INTERVAL 0x10

/**
 * Bitmaske in CONF_SEND_ENABLE für Betriebszeit.
 */
#define CONF_ENABLE_OPERATION_TIME_TYPE 0x08

/**
 * Bitmaske in CONF_SEND_ENABLE für Alarm Vernetzung zyklisch senden.
 */
#define CONF_ENABLE_ALARM_INTERVAL_NW 0x04

/**
 * Bitmaske in CONF_SEND_ENABLE für Testalarm Vernetzung zyklisch senden.
 */
#define CONF_ENABLE_TALARM_INTERVAL_NW 0x02

/**
 * Bitmaske in CONF_SEND_ENABLE für Testalarm Status bei 0 zyklisch senden.
 */
#define CONF_ENABLE_TALARM_INTERVAL_S0 0x01

/**
 * Bits aktivieren das zyklische Senden der Com-Objekte 6..13
 */
#define CONF_INFO_6TO13		508

/**
 * Bits aktivieren das zyklische Senden der Com-Objekte 14..21
 */
#define CONF_INFO_14TO21	509

/**
 * Temperaturabgleich -12/+12 Grad in 0.1°C Schritten
 */
#define CONF_TEMP_OFFSET    510


#endif /* rm_eeprom_h */
