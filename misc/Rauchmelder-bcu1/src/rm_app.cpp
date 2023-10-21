/*
 *    _____ ________    __________  __  _______    ____  __  ___
 *   / ___// ____/ /   / ____/ __ )/ / / / ___/   / __ \/  |/  /
 *   \__ \/ __/ / /   / /_  / __  / / / /\__ \   / /_/ / /|_/ /
 *  ___/ / /___/ /___/ __/ / /_/ / /_/ /___/ /  / _, _/ /  / /
 * /____/_____/_____/_/   /_____/\____//____/  /_/ |_/_/  /_/
 *
 *  Original written for LPC922:
 *  Copyright (c) 2016 Stefan Haller
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017-2022 Oliver Stefan <o.stefan252@googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <stdint.h>
#include <sblib/eib/datapoint_types.h>
#include "rm_app.h"
#include "rm_const.h"
#include "rm_com.h"
#include "rm_eeprom.h"

BCU1 bcu = BCU1();

// Befehle an den Rauchmelder
/*
const unsigned char CmdTab[RM_CMD_COUNT] =
{
	0x04,   // RM_CMD_SERIAL
	0x09,   // RM_CMD_OPERATING_TIME
	0x0B,   // RM_CMD_SMOKEBOX
	0x0C,   // RM_CMD_BATTEMP
	0x0D,   // RM_CMD_NUM_ALARMS
	0x0E	// RM_CMD_NUM_ALARMS_2
};
*/

const struct
{
	unsigned const char cmdno;      // Zu sendender RM_CMD Befehl
	unsigned const char objects[MAX_OBJ_CMD];	// Zuordnung der ComObjekte zu den Befehlen
} CmdTab[RM_CMD_COUNT] =
{	//CMD	objNr.
	{ 0x04, {6,  0xFF, 0xFF, 0xFF} },	// <STX>C4214710F31F<ETX> RM_CMD_SERIAL
	{ 0x09, {7,  0xFF, 0xFF, 0xFF} },	// <STX>C9000047E31F<ETX> RM_CMD_OPERATING_TIME
	{ 0x0B, {8,  9,    15,   0xFF} },	// <STX>CB0065000111<ETX> RM_CMD_SMOKEBOX
	{ 0x0C, {10, 11,   0xFF, 0xFF} }, 	// <STX>CC000155551B<ETX> RM_CMD_BATTEMP
	{ 0x0D, {16, 17,   18,   19} },		// <STX>CD0000000007<ETX> RM_CMD_NUM_ALARMS
	{ 0x0E, {20, 21,   0xFF, 0xFF} } 	// <STX>CE000048<ETX> RM_CMD_NUM_ALARMS_2
};

// Mapping von den Kommunikations-Objekten auf die Rauchmelder Requests
// und die Daten in der Rauchmelder Antwort. Der Index in die Tabelle ist
// die ID vom Kommunikations-Objekt (objid).
const struct
{
	unsigned const char cmd;       // Zu sendender RM_CMD Befehl
	unsigned const char offset;    // Byte-Offset in der Antwort
	unsigned const char dataType;  // Datentyp der Antwort
} objMappingTab[NUM_OBJS] =
{
	/* 0 OBJ_ALARM_BUS*/           { RM_CMD_INTERNAL,        0, RM_TYPE_NONE },
	/* 1 OBJ_TALARM_BUS*/          { RM_CMD_INTERNAL,        0, RM_TYPE_NONE },
	/* 2 OBJ_RESET_ALARM*/         { RM_CMD_INTERNAL,        0, RM_TYPE_NONE },
	/* 3 OBJ_STAT_ALARM*/          { RM_CMD_INTERNAL,        0, RM_TYPE_NONE },
	/* 4 OBJ_STAT_ALARM_DELAYED*/  { RM_CMD_INTERNAL,        0, RM_TYPE_NONE },
	/* 5 OBJ_STAT_TALARM*/         { RM_CMD_INTERNAL,        0, RM_TYPE_NONE },
	/* 6 OBJ_SERIAL*/              { RM_CMD_SERIAL,          0, RM_TYPE_LONG },
	/* 7 OBJ_OPERATING_TIME*/      { RM_CMD_OPERATING_TIME,  0, RM_TYPE_QSEC },
	/* 8 OBJ_SMOKEBOX_VALUE*/      { RM_CMD_SMOKEBOX,        0, RM_TYPE_SHORT },
	/* 9 OBJ_POLLUTION*/           { RM_CMD_SMOKEBOX,        3, RM_TYPE_BYTE },
	/*10 OBJ_BAT_VOLTAGE*/         { RM_CMD_BATTEMP,         0, RM_TYPE_MVOLT },
	/*11 OBJ_TEMP*/                { RM_CMD_BATTEMP,         2, RM_TYPE_TEMP },
	/*12 OBJ_ERRCODE*/             { RM_CMD_INTERNAL,        0, RM_TYPE_NONE },
	/*13 OBJ_BAT_LOW*/             { RM_CMD_INTERNAL,        0, RM_TYPE_NONE },
	/*14 OBJ_MALFUNCTION*/         { RM_CMD_INTERNAL,        0, RM_TYPE_NONE },
	/*15 OBJ_CNT_SMOKEALARM*/      { RM_CMD_SMOKEBOX,        2, RM_TYPE_BYTE },
	/*16 OBJ_CNT_TEMPALARM*/       { RM_CMD_NUM_ALARMS,      0, RM_TYPE_BYTE },
	/*17 OBJ_CNT_TESTALARM*/       { RM_CMD_NUM_ALARMS,      1, RM_TYPE_BYTE },
	/*18 OBJ_CNT_ALARM_WIRE*/      { RM_CMD_NUM_ALARMS,      2, RM_TYPE_BYTE },
	/*19 OBJ_CNT_ALARM_BUS*/	   { RM_CMD_NUM_ALARMS,      3, RM_TYPE_BYTE },
	/*20 OBJ_CNT_TALARM_WIRE*/     { RM_CMD_NUM_ALARMS_2,    0, RM_TYPE_BYTE },
	/*21 OBJ_CNT_TALARM_BUS*/	   { RM_CMD_NUM_ALARMS_2,    1, RM_TYPE_BYTE }
};


// Flag für lokalen Alarm und Wired Alarm (über grüne Klemme / Rauchmelderbus)
bool alarmLocal;

// Flag für remote Alarm über EIB
bool alarmBus;

// Flag für lokalen Testalarm und Wired Testalarm
bool testAlarmLocal;

// Flag für remote Testalarm über EIB
bool testAlarmBus;

// Flag für den gewünschten Alarm Status wie wir ihn über den EIB empfangen haben
bool setAlarmBus;

// Flag für den gewünschten Testalarm Status wie wir ihn über den EIB empfangen haben
bool setTestAlarmBus;

// Flag für Bus Alarm & -Testalarm ignorieren
bool ignoreBusAlarm;

// Rauchmelder Fehlercodes
unsigned char errCode;

// Flags für Com-Objekte senden
unsigned char objSendReqFlags[NUM_OBJ_FLAG_BYTES];

// Werte der Com-Objekte. Index ist die der RM_CMD
unsigned long objValues[RM_CMD_COUNT];


// Nummer des Befehls an den Rauchmelder der gerade ausgeführt wird.
// RM_CMD_NONE wenn keiner.  So lange ein RM_CMD ausgeführt wird darf auf
// objValues[cmdCurrent] nicht zugegriffen werden. Es muss stattdessen objOldValue
// verwendet werden.
unsigned char cmdCurrent;

// Backup des Com-Objekt Wertebereichs der gerade von cmdCurrent neu vom
// Rauchmelder geholt wird.
unsigned long objValueCurrent;


// Zähler für die Zeit die auf eine Antwort vom Rauchmelder gewartet wird.
// Ist der Zähler 0 dann wird gerade auf keine Antwort gewartet.
unsigned char answerWait;

// Initialwert für answerWait in 0,5s
#define INITIAL_ANSWER_WAIT 6

// Zähler für keine Antwort vom Rauchmelder
unsigned char noAnswerCount;

// Maximale Anzahl in noAnswerCount ab der ein Rauchmelder Fehler gemeldet wird
#define NO_ANSWER_MAX 5


// Zähler für Alarm am JP2 - EXTRA_ALARM_PIN
unsigned char extraAlarmCounter;

// Schwelle für extraAlarmCounter in 0,5s
#define EXTRA_ALARM_LIMIT 5


// Countdown Zähler für zyklisches Senden eines Alarms.
unsigned char alarmCounter;

// Countdown Zähler für zyklisches Senden eines Testalarms.
unsigned char TalarmCounter;

// Countdown Zähler für zyklisches Senden des Alarm Zustands.
unsigned char alarmStatusCounter;

// Countdown Zähler für verzögertes Senden eines Alarms
unsigned char delayedAlarmCounter;

// Countdown Zähler für zyklisches Senden der (Info) Com-Objekte
unsigned char infoCounter;

// Nummer des Com-Objekts das bei zyklischem Info Senden als nächstes geprüft/gesendet wird
unsigned char infoSendObjno;

// Nummer des Befehls, welcher als nächstes zyklisch an den Rauchmelder gesendet wird
unsigned char readCmdno;

// Halbsekunden Zähler 0..119
unsigned char eventTime = DEFAULT_EVENTTIME;

// Tabelle für 1<<x, d.h. pow2[3] == 1<<3
const unsigned char pow2[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

// Im Byte Array arr das bitno-te Bit setzen
#define ARRAY_SET_BIT(arr, bitno) arr[bitno>>3] |= pow2[bitno & 7]

// Im Byte Array arr das bitno-te Bit löschen
#define ARRAY_CLEAR_BIT(arr, bitno) arr[bitno>>3] &= ~pow2[bitno & 7]

// Testen ob im Byte Array arr das bitno-te Bit gesetzt ist
#define ARRAY_IS_BIT_SET(arr, bitno) (arr[bitno>>3] & pow2[bitno & 7])


// Verwendet um Response Telegramme zu kennzeichnen.
#define OBJ_RESPONSE_FLAG 0x40


/**
 * Den Alarm Status auf den Bus senden falls noch nicht gesendet.
 *
 * @param newAlarm - neuer Alarm Status
 *//*
void send_obj_alarm(bool newAlarm)
{
	if (alarmLocal != newAlarm)
	{
		objectWrite(OBJ_ALARM_BUS, newAlarm);
		if()
		objectWrite(OBJ_STAT_ALARM, newAlarm);
	}
}*/


/**
 * Den Testalarm Status auf den Bus senden falls noch nicht gesendet.
 *
 * @param newAlarm - neuer Testalarm Status
 */
void send_obj_test_alarm(bool newAlarm)
{
	if (testAlarmLocal != newAlarm)
	{
		bcu.comObjects->objectWrite(OBJ_TALARM_BUS, newAlarm);
		bcu.comObjects->objectWrite(OBJ_STAT_TALARM, newAlarm);
	}
}


/**
 * Fehlercode setzen
 *
 * @param newErrCode - neuer Fehlercode
 */
void set_errcode(unsigned char newErrCode)
{
	if (newErrCode == errCode)
		return;

	// Wenn sich der Status der Batterie geändert hat dann OBJ_BAT_LOW senden,
	// sonst den allgemeinen Fehler Indikator OBJ_MALFUNCTION.
	if ((errCode ^ newErrCode) & ERRCODE_BATLOW)
		ARRAY_SET_BIT(objSendReqFlags, OBJ_BAT_LOW);
	else ARRAY_SET_BIT(objSendReqFlags, OBJ_MALFUNCTION);

	errCode = newErrCode;
}


/**
 * Die empfangene Nachricht vom Rauchmelder verarbeiten.
 * Wird von _receive() aufgerufen.
 */
void rm_process_msg(unsigned char* bytes, unsigned char len)
{
	uint8_t cmd;
	uint8_t msgType;
	uint8_t byteno;
	uint8_t mask;

	answerWait = 0;
	if (noAnswerCount)
	{
		noAnswerCount = 0;
		set_errcode(errCode & ~ERRCODE_COMM);
	}

	msgType = bytes[0];
	if ((msgType & 0xf0) == 0xc0) // Com-Objekt Werte empfangen
	{
		msgType &= 0x0f;

		for (cmd = 0; cmd < RM_CMD_COUNT; ++cmd)
		{
			if (CmdTab[cmd].cmdno == msgType)
				break;
		}

		if (cmd < RM_CMD_COUNT)
		{
			objValueCurrent = objValues[cmd];
			cmdCurrent = cmd;

			///\todo eleganter umsetzbar?
			objValues[cmd] = 0;
			for( unsigned char lencnt = 1; lencnt<len; lencnt++ ){
				objValues[cmd] |= (bytes[lencnt] << ((lencnt-1)*8));
			}
			// vorher: objValues[cmd] = *(unsigned long*)(bytes + 1); // führt zu HardFault auf ARM Controller!!!!

			cmdCurrent = RM_CMD_NONE;
			
			// Informationen aus den empfangenen Daten vom Rauchmelder der sblib zur Verfügung stellen
			// Dazu alle Com-Objekte suchen auf die die empfangenen Daten passen (mapping durch CmdTab)
			// notwendig für den Abruf von Informationen über KNX aus den Status Objekten (GroupValueRead -> GroupValueResponse)
			for(unsigned char cmdObj_cnt=0; CmdTab[cmd].objects[cmdObj_cnt] != 0xFF && cmdObj_cnt < MAX_OBJ_CMD; cmdObj_cnt++)
			{
				uint8_t objno = CmdTab[cmd].objects[cmdObj_cnt];
				bcu.comObjects->objectSetValue(objno, read_obj_value(objno));

				// Versand der erhaltenen Com-Objekte einleiten.
				// Sofern sie für den Versand vorgemerkt sind.
				byteno = objno >> 3;
				mask = pow2[objno & 7];

				if (objSendReqFlags[byteno] & mask)
				{
				    bcu.comObjects->objectWrite(objno, read_obj_value(objno));
					objSendReqFlags[byteno] &= ~mask;
				}
			}
		}
	}
	else if (msgType == 0x82 && len >= 5) // Status Meldung
	{
		unsigned char subType = bytes[1];
		bool newAlarm;

		// (Alarm) Status

		unsigned char status = bytes[2];

		// Lokaler Alarm: Rauch Alarm | Temperatur Alarm | Wired Alarm
		newAlarm = (subType & 0x10) | (status & (0x04 | 0x08));
		if ((bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_ALARM_DELAYED) && newAlarm) // wenn Alarm verzögert gesendet werden soll und Alarm ansteht
		{
			delayedAlarmCounter = bcu.userEeprom->getUInt8(CONF_ALARM_DELAYED);
			bcu.comObjects->objectSetValue(OBJ_STAT_ALARM_DELAYED, read_obj_value(OBJ_STAT_ALARM_DELAYED));
		}
		else if (alarmLocal != newAlarm)//wenn Alarm nicht verzögert gesendet werden soll oder Alarm nicht mehr ansteht (nur 1x senden)
		{
		    bcu.comObjects->objectWrite(OBJ_ALARM_BUS, newAlarm);
		}

		if (alarmLocal != newAlarm){ //sobald neuer AlarmStaus ansteht, soll dieser versendet werden
		    bcu.comObjects->objectWrite(OBJ_STAT_ALARM, newAlarm);
		}

		alarmLocal = newAlarm;

		// Lokaler Testalarm: (lokaler) Testalarm || Wired Testalarm
		newAlarm = status & (0x20 | 0x40);
		send_obj_test_alarm(newAlarm);
		testAlarmLocal = newAlarm;

		// Bus Alarm
		alarmBus = status & 0x10;

		// Bus Testalarm
		testAlarmBus = status & 0x80;

		// Batterie schwach/leer
		if ((status ^ errCode) & ERRCODE_BATLOW)
		{
			set_errcode((errCode & ~ERRCODE_BATLOW) | (status & ERRCODE_BATLOW));
			
			// Werte für OBJ_ERRCODE (Objekt 12), OBJ_BAT_LOW (Objekt 13) und OBJ_MALFUNCTION (Objekt 14) für die sblib zur Verfügung stellen
			// notwendig für den Abruf von Informationen über KNX aus den Status Objekten (GroupValueRead -> GroupValueResponse)
			for(unsigned char objno=12; objno<=14; objno++){
			    bcu.comObjects->objectSetValue(objno, read_obj_value(objno));
			}
		}


		/*
		* In der folgenden Passage ist für mich die Versendung der Objekte nicht nachvollziehbar:
		* Es wird kontrolliert, ob die Taste am Rauchmelder gedrückt wurde, anschließend wir überprüft, ob ein Alarm oder TestAlarm vom Bus vorliegt
		* Dann wird der jeweilige Status versendet.
		* für welchen Anwendungfall ist dieses sinnvoll?
		* zur Zeit wird vom lokalen Rauchmelder der setAlarmBus ausgelöst (quasi local loopback) und die Tastenerkennung löst aus
		* Somit wird die Status Nachricht EIN 2x versendet (1x aus send_obj_alarm bzw. send_obj_test_alarm) und einmal hier.
		* AUS wird hier allerdings nicht versendet, da setAlarmBus bzw. setTestAlarmBus dann false sind
		*
		* Daher habe ich mich entschieden, diese Versendung vorerst zu deaktivieren
		*/

		if (subType & 0x08)  // Taste am Rauchmelder gedrückt
		{
			if (setAlarmBus) //wenn Alarm auf Bus anliegt
			{
				setAlarmBus = 0;
				delayedAlarmCounter = 0; // verzögerten Alarm abbrechen
				//objectWrite(OBJ_STAT_ALARM, read_obj_value(OBJ_STAT_ALARM));
			}

			if (setTestAlarmBus) //wenn Testalarm auf Bus anliegt
			{
				setTestAlarmBus = 0;
				//objectWrite(OBJ_STAT_TALARM, read_obj_value(OBJ_STAT_TALARM));
			}
		}


		if (subType & 0x02)  // Defekt am Rauchmelder
		{
			unsigned char status = bytes[4];
			unsigned char newErrCode = errCode & (ERRCODE_BATLOW | ERRCODE_COMM);

			if (status & 0x04)
				newErrCode |= ERRCODE_TEMP1;

			if (status & 0x10)
				newErrCode |= ERRCODE_TEMP2;

			// TODO Rauchkammer Defekt behandeln

			set_errcode(newErrCode);
		}
	}
}


/**
 * Die Rauchmelder Antwort als Long Zahl liefern.
 *
 * @param cvalue - das erste Byte der Rauchmelder Antwort.
 * @return Der Wert mit getauschten Bytes.
 */
unsigned long answer_to_long(unsigned char* cvalue)
{
	return (cvalue[0] << 24) | (cvalue[1] << 16) | (cvalue[2] << 8) | cvalue[3];
}


/**
 * Die Rauchmelder Antwort als Integer Zahl liefern.
 *
 * @param cvalue - das erste Byte der Rauchmelder Antwort.
 * @return Der Wert mit getauschten Bytes.
 */
unsigned short answer_to_short(unsigned char* cvalue)
{
	return (cvalue[0] << 8) | cvalue[1];
}


/**
 * Wert eines Com-Objekts liefern.
 *
 * @param objno - die ID des Kommunikations-Objekts
 * @return Den Wert des Kommunikations Objekts
 */
unsigned long read_obj_value(unsigned char objno)
{
	unsigned char cmd = objMappingTab[objno].cmd;

//	DEBUG_WRITE_BYTE(objno);
//	DEBUG_WRITE_BYTE(eeprom[COMMSTABPTR]);
//	//DEBUG_WRITE_BYTE(eeprom[eeprom[COMMSTABPTR]+objno*3+4]); // objtype

	// Interne Com-Objekte behandeln
	if (cmd == RM_CMD_INTERNAL)
	{
		switch (objno)
		{
		case OBJ_ALARM_BUS:
		case OBJ_STAT_ALARM:
			return alarmLocal;

		case OBJ_TALARM_BUS:
		case OBJ_STAT_TALARM:
			return testAlarmLocal;

		case OBJ_RESET_ALARM:
			return ignoreBusAlarm;

		case OBJ_STAT_ALARM_DELAYED:
			return delayedAlarmCounter != 0;

		case OBJ_BAT_LOW:
			return (errCode & ERRCODE_BATLOW) != 0;

		case OBJ_MALFUNCTION:
			return (errCode & ~ERRCODE_BATLOW) != 0;

		case OBJ_ERRCODE:
			return errCode;
		}
	}
	// Com-Objekte verarbeiten die Werte vom Rauchmelder darstellen
	else if (cmd != RM_CMD_NONE)
	{
		unsigned long lval;
		unsigned char* answer;

		if (cmd == cmdCurrent) {
			answer = (unsigned char*) &objValueCurrent;
		}
		else {
			answer = (unsigned char*) &objValues[cmd];
		}
		answer += objMappingTab[objno].offset;

		switch (objMappingTab[objno].dataType)
		{
		case RM_TYPE_BYTE:
			return *answer;

		case RM_TYPE_LONG:
			return answer_to_long(answer);

		case RM_TYPE_QSEC:  // Betriebszeit verarbeiten
		    lval = answer_to_long(answer) >> 2; // Wert in Sekunden
		    if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_OPERATION_TIME_TYPE)
		        return lval / 3600; // Stunden, 16Bit
		    else
		        return lval;        // Sekunden, 32Bit

		case RM_TYPE_SHORT:
			return answer_to_short(answer);

		case RM_TYPE_TEMP:
			// Conversion per temp sensor: (answer[x] * 0.5°C - 20°C) * 100 [for DPT9]
			lval = ((int) answer[0]) + answer[1];
			lval *= 25;  // in lval sind zwei Temperaturen, daher halber Multiplikator
			lval -= 2000;
			lval += (signed char)bcu.userEeprom->getUInt8(CONF_TEMP_OFFSET) *10;  // Temperaturabgleich
			return (floatToDpt9(lval));

		case RM_TYPE_MVOLT:
			if ((answer[0]  == 0) && (answer[1] == 1))
			{
			    return (floatToDpt9(BATTERY_VOLTAGE_INVALID));
			}
			lval = answer_to_short(answer);
			// Conversion: lval * 5.7 * 3.3V / 1024 * 1000mV/V * 100 [for DPT9]
			lval *= 9184;
			lval /= 5;
			return (floatToDpt9(lval));

		default: // Fehler: unbekannter Datentyp
			return -2;
		}
	}

	// Fehler: unbekanntes Com Objekt
	return -1;
}


/**
 * Empfangenes write_value_request Telegramm verarbeiten
 *
 * @param objno - Nummer des betroffenen Kommunikations-Objekts
 */
void objectUpdated(int objno)
{
 	if (objno == OBJ_ALARM_BUS) // Bus Alarm
	{

 		setAlarmBus = bcu.comObjects->objectRead(objno) & 0x01; //ToDo: prüfen ob ok   //war: setAlarmBus = telegramm[7] & 0x01;

 		// Wenn wir lokalen Alarm haben dann Bus Alarm wieder auslösen
		// damit der Status der anderen Rauchmelder stimmt
 		if (!setAlarmBus && alarmLocal)
 		   bcu.comObjects->objectWrite(OBJ_ALARM_BUS, read_obj_value(OBJ_ALARM_BUS)); //send_obj_value(OBJ_ALARM_BUS);

 		if (ignoreBusAlarm)
 			setAlarmBus = 0;
	}
	else if (objno == OBJ_TALARM_BUS) // Bus Test Alarm
	{
		setTestAlarmBus = bcu.comObjects->objectRead(objno) & 0x01; //ToDo: prüfen ob ok   //war: setTestAlarmBus = telegramm[7] & 0x01;

 		// Wenn wir lokalen Testalarm haben dann Bus Testalarm wieder auslösen
		// damit der Status der anderen Rauchmelder stimmt
 		if (!setTestAlarmBus && testAlarmLocal)
 		   bcu.comObjects->objectWrite(OBJ_TALARM_BUS, read_obj_value(OBJ_TALARM_BUS)); //send_obj_value(OBJ_TALARM_BUS);

 		if (ignoreBusAlarm)
 			setTestAlarmBus = 0;
 	}
	else if (objno == OBJ_RESET_ALARM) // Bus Alarm rücksetzen
	{
		setAlarmBus = 0;
		setTestAlarmBus = 0;
		ignoreBusAlarm = 1;
	}
}

/**
 * Checks if the smoke detector is on the base plate and switches the supply voltage on
 */
void checkRmAttached2BasePlate(void)
{
    bool rmActiv = digitalRead(RM_ACTIVITY_PIN);
    digitalWrite(LED_BASEPLATE_DETACHED, rmActiv);

    if (digitalRead(RM_SUPPORT_VOLTAGE_PIN) == RM_SUPPORT_VOLTAGE_ON)
    {
        return; // supply voltage is already on
    }

    // der Rauchmelder wurde auf die Bodenplatte gesteckt => Spannungsversorgung aktivieren
    if ((rmActiv == RM_IS_ACTIVE) || (millis() >= SUPPLY_VOLTAGE_TIMEOUT_MS))
    {
        digitalWrite(RM_SUPPORT_VOLTAGE_PIN, RM_SUPPORT_VOLTAGE_ON); // Spannungsversorgung aktivieren
        delay(SUPPLY_VOLTAGE_DELAY_MS);
        digitalWrite(LED_SUPPLY_VOLTAGE_DISABLED, true);
    }
}

/**
 * Befehl an den Rauchmelder versenden
 * Der Empfang und die Verarbeitung der Antwort des Rauchmelders erfolgt in process_msg().
 *
 * @param cmd - Index des zu sendenden Befehls aus der CmdTab
 */
void send_Cmd(unsigned char cmd){
	if (isReceiving())
	{
	    return;
	}

    checkRmAttached2BasePlate();

    ///\todo set remaining objValues to invalid values before sending serial command
    switch (cmd)
    {
        case RM_CMD_SERIAL:
            break;

        case RM_CMD_OPERATING_TIME:
            break;

        case RM_CMD_SMOKEBOX:
            break;

        case RM_CMD_BATTEMP:
            objValues[RM_CMD_BATTEMP] = 0;
            break;

        case RM_CMD_NUM_ALARMS:
            break;

        case RM_CMD_NUM_ALARMS_2:
            break;

        default:
            ;
    }

    rm_send_cmd(CmdTab[cmd].cmdno);
    answerWait = INITIAL_ANSWER_WAIT;
}

/**
 * Ein Com-Objekt bearbeiten.
 *
 * @param objno - die Nummer des zu bearbeitenden Com Objekts
 */
void process_obj(unsigned char objno)
{
	// Der Wert des Com-Objekts ist bekannt, also sofort senden
	// Die Werte werden zyklisch (minütlich) alle vom Rauchmelder abgefragt, daher sind immer alle Werte aktuell vorhanden

	unsigned char byteno = objno >> 3;
	unsigned char mask = pow2[objno & 7];

	if (objSendReqFlags[byteno] & mask)
	{
	    bcu.comObjects->objectWrite(objno, read_obj_value(objno));
		objSendReqFlags[byteno] &= ~mask;
	}
}


/**
 * Com-Objekte bearbeiten, Worker Funktion.
 *
 * Com-Objekte, die Daten vom Rauchmelder benötigen, werden nur bearbeitet wenn
 * nicht gerade auf Antwort vom Rauchmelder gewartet wird.
 *
 * @return 1 wenn ein Com-Objekt verarbeitet wurde, sonst 0.
 */
unsigned char do_process_objs(unsigned char *flags)
{
	unsigned char byteno, bitno, objno, cmd, flagsByte;

	for (byteno = 0; byteno < NUM_OBJ_FLAG_BYTES; ++byteno)
	{
		flagsByte = flags[byteno];
		if (!flagsByte) continue;

		for (bitno = 0; bitno < 8; ++bitno)
		{
			if (flagsByte & pow2[bitno])
			{
				objno = (byteno << 3) + bitno;
				cmd = objMappingTab[objno].cmd;
				if (!answerWait || cmd == RM_CMD_NONE || cmd == RM_CMD_INTERNAL)
				{
					process_obj(objno);
					return 1;
				}
			}
		}
	}

	return 0;
}

/**
 * Com-Objekte bearbeiten.
 */
void process_objs()
{
	do_process_objs(objSendReqFlags);
}


/**
 * Den Zustand der Alarme bearbeiten. Wenn wir der Meinung sind der Bus-Alarm soll einen
 * bestimmten Zustand haben dann wird das dem Rauchmelder so lange gesagt bis der auch
 * der gleichen Meinung ist.
 */
void process_alarm_stats()
{
	if (setAlarmBus && !alarmBus)
	{
		// Alarm auslösen
		rm_set_alarm_state(RM_ALARM);
		answerWait = INITIAL_ANSWER_WAIT;
	}
	else if (setTestAlarmBus && !testAlarmBus)
	{
		// Testalarm auslösen
	    rm_set_alarm_state(RM_TEST_ALARM);
		answerWait = INITIAL_ANSWER_WAIT;
	}
	else if ((!setAlarmBus && alarmBus) || (!setTestAlarmBus && testAlarmBus))
	{
		// Alarm und Testalarm beenden
	    rm_set_alarm_state(RM_NO_ALARM);
		answerWait = INITIAL_ANSWER_WAIT;
	}
}


/**
 * Timer Event.
 * Is called every 500ms
 */
extern "C" void TIMER32_0_IRQHandler()
{
    // Clear the timer interrupt flags. Otherwise the interrupt handler is called
    // again immediately after returning.
    timer32_0.resetFlags();

	--eventTime;

	// Wir warten auf eine Antwort vom Rauchmelder
	if (answerWait)
	{
		--answerWait;

		// Wenn keine Antwort vom Rauchmelder kommt dann den noAnswerCount Zähler
		// erhöhen. Wenn der Zähler NO_ANSWER_MAX erreicht dann ist es ein Rauchmelder
		// Fehler.
		if (!answerWait)
		{
			if (noAnswerCount < 255)
			{
				++noAnswerCount;
			}
			if (noAnswerCount >= NO_ANSWER_MAX)
			{
				set_errcode(errCode | ERRCODE_COMM);
			}
		}
	}

	// Alles danach wird nur jede Sekunde gemacht
	if (eventTime & 1)
		return;

	// Alarm: verzögert und zyklisch senden
	if (alarmLocal)
	{
		// Alarm verzögert senden
		if (delayedAlarmCounter)
		{
			--delayedAlarmCounter;
			if (!delayedAlarmCounter)   // Verzögerungszeit abgelaufen
			{
			    bcu.comObjects->objectSetValue(OBJ_STAT_ALARM_DELAYED, read_obj_value(OBJ_STAT_ALARM_DELAYED)); // Status verzögerter Alarm zurücksetzen
				//ARRAY_SET_BIT(objSendReqFlags, OBJ_ALARM_BUS);  // Vernetzung Alarm senden
				//ARRAY_SET_BIT(objSendReqFlags, OBJ_STAT_ALARM); // Status Alarm senden

				bcu.comObjects->objectWrite(OBJ_ALARM_BUS, alarmLocal);
			}
		}
		else // Alarm zyklisch senden
		{
			if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_ALARM_INTERVAL)
			{
				--alarmCounter;
				if (!alarmCounter)
				{
					alarmCounter = bcu.userEeprom->getUInt8(CONF_ALARM_INTERVAL);     // Zykl. senden Zeit holen
					if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_ALARM_INTERVAL_NW)
                    {
                        ARRAY_SET_BIT(objSendReqFlags, OBJ_ALARM_BUS);  // Vernetzung Alarm senden
                    }
					ARRAY_SET_BIT(objSendReqFlags, OBJ_STAT_ALARM);
				}
			}
		}
	}
	// Kein Alarm, zyklisch 0 senden
	else
	{
	    if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_TALARM_INTERVAL_S0)
        {
            --alarmCounter;
            if (!alarmCounter)
            {
                alarmCounter = bcu.userEeprom->getUInt8(CONF_ALARM_INTERVAL);     // Zykl. senden Zeit holen
                ARRAY_SET_BIT(objSendReqFlags, OBJ_STAT_ALARM);
            }
        }
    }

	// Testalarm: zyklisch senden
	if (testAlarmLocal)
	{
		if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_TALARM_INTERVAL)
		{
			--TalarmCounter;
			if (!TalarmCounter)
			{
				TalarmCounter = bcu.userEeprom->getUInt8(CONF_TALARM_INTERVAL);
				if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_TALARM_INTERVAL_NW)
				{
				    ARRAY_SET_BIT(objSendReqFlags, OBJ_TALARM_BUS);
				}
				ARRAY_SET_BIT(objSendReqFlags, OBJ_STAT_TALARM);
			}
		}
	}

	// Jede zweite Sekunde ein Status Com-Objekt senden.
	// (Vormals war es jede 4. Sekunde, aber dann reicht 1 Minute nicht für 16 eventuell zu sendende Status Objekte (ComOject 6 - 21))
	// Aber nur senden wenn kein Alarm anliegt.
	if ((eventTime & DEFAULT_KNX_OBJECT_TIME) == 0 && infoSendObjno &&
	    !(alarmLocal | alarmBus | testAlarmLocal | testAlarmBus))
	{
		// Info Objekt zum Senden vormerken wenn es dafür konfiguriert ist.
		// Leider sind die Bits in der VD in der falschen Reihenfolge, daher 7-x
		if ((infoSendObjno >= 14 && (bcu.userEeprom->getUInt8(CONF_INFO_14TO21) & pow2[7 - (infoSendObjno - 14)])) ||
				(infoSendObjno < 14 && infoSendObjno >= 6 && (bcu.userEeprom->getUInt8(CONF_INFO_6TO13) & pow2[7 - (infoSendObjno - 6)])))
		{
			ARRAY_SET_BIT(objSendReqFlags, infoSendObjno);
		}

		--infoSendObjno;
	}

	// alle 8 Sekunden einen der 6 Befehle aus der CmdTab an den Rauchmelder senden, um alle Status Daten aus dem Rauchmelder abzufragen
	// notwendig, da die ARM sblib keine Funktion aufruft, wenn ein Objekt ausgelesen wird
	// daher müssen alle Informationen immer im Speicher vorliegen
	if ((eventTime & DEFAULT_SERIAL_COMMAND_TIME) == 0 && readCmdno &&
			!(alarmLocal | alarmBus | testAlarmLocal | testAlarmBus))
	{
		if (!answerWait){
			readCmdno--;
			send_Cmd(readCmdno);
		}
	}

	/* Hier kann die minütliche Kontrolle der Bodenplatte aktiviert werden

	// Zur vollen Minute die Spannungsversorgung ausschalten, um zu prüfen, ob der RM noch auf der Bodenplatte arretiert ist
	// Der RM_ACTIVITY_PIN wird nur low, wenn die Unterstützung der Versorgungsspannung ausgeschaltet ist
	if (!eventTime && !(alarmLocal | alarmBus | testAlarmLocal | testAlarmBus))
	{
		digitalWrite(RM_SUPPORT_VOLTAGE_PIN, RM_SUPPORT_VOLTAGE_OFF);
	}

	if(eventTime == 4){ //bei 2 Sekunden kontrollieren, ob der RM noch auf der Bodenplatte arretiert ist
		pinMode(RM_ACTIVITY_PIN, INPUT | PULL_DOWN);	//Pin als Eingang mit Pulldown Widerstand konfigurieren
		if(digitalRead(RM_ACTIVITY_PIN) == RM_IS_ACTIVE){
			digitalWrite(RM_SUPPORT_VOLTAGE_PIN, RM_SUPPORT_VOLTAGE_ON); // Spannungsversorgung wieder aktivieren
		}
		pinMode(RM_ACTIVITY_PIN, INPUT);
	}
	*/

	if (!eventTime) // einmal pro Minute
	{
		eventTime = DEFAULT_EVENTTIME;

		// Bus Alarm ignorieren Flag rücksetzen wenn kein Alarm mehr anliegt
		if (ignoreBusAlarm & !(alarmBus | testAlarmBus))
			ignoreBusAlarm = 0;

		if(!readCmdno){
			readCmdno = RM_CMD_COUNT;
		}

		// Status Informationen zyklisch senden
		if (bcu.userEeprom->getUInt8(CONF_SEND_ENABLE) & CONF_ENABLE_INFO_INTERVAL)
		{
			--infoCounter;
			if (!infoCounter)
			{
				infoCounter = bcu.userEeprom->getUInt8(CONF_INFO_INTERVAL);
				infoSendObjno = OBJ_HIGH_INFO_SEND;
			}
		}
	}
}

void setupPeriodicTimer(uint32_t milliseconds)
{
    // Enable the timer interrupt
    enableInterrupt(TIMER_32_0_IRQn);

    // Begin using the timer
    timer32_0.begin();

    // Let the timer count milliseconds
    timer32_0.prescaler((SystemCoreClock / 1000) - 1);

    // On match of MAT1, generate an interrupt and reset the timer
    timer32_0.matchMode(MAT1, RESET | INTERRUPT);

    // Match MAT1 when the timer reaches this value (in milliseconds)
    timer32_0.match(MAT1, milliseconds- 1); // -1 because counting starts from 0, e.g. 0-499=500ms

    timer32_0.start();
}

/**
 * Alle Applikations-Parameter zurücksetzen
 */
void initApplication()
{
    rm_serial_init();   //serielle Schnittstelle für die Kommunikation mit dem Rauchmelder initialisieren

	// Werte initialisieren
	for (uint8_t i = 0; i < NUM_OBJ_FLAG_BYTES; ++i)
	{
		objSendReqFlags[i] = 0;
	}

    for (uint8_t i = 0; i < RM_CMD_COUNT; ++i)
    {
        objValues[i] = 0;
    }

	answerWait = 0;
	noAnswerCount = 0;
	cmdCurrent = RM_CMD_NONE;

	alarmBus = 0;
	alarmLocal = 0;

	testAlarmBus = 0;
	testAlarmLocal = 0;

	setAlarmBus = 0;
	setTestAlarmBus = 0;
	ignoreBusAlarm = 0;

	infoSendObjno = 0;
	readCmdno = RM_CMD_COUNT;
	infoCounter = bcu.userEeprom->getUInt8(CONF_INFO_INTERVAL);
	alarmCounter = 1;
	TalarmCounter = 1;
	alarmStatusCounter = 1;
	delayedAlarmCounter = 0;
	extraAlarmCounter = 0;

	errCode = 0;

    // set all comObjects to default
    for (uint8_t i = 0; i < NUM_OBJS; i++)
    {
        bcu.comObjects->objectSetValue(i, 0);
    }

    pinMode(LED_BASEPLATE_DETACHED, OUTPUT);
    digitalWrite(LED_BASEPLATE_DETACHED, false);
    pinMode(LED_SUPPLY_VOLTAGE_DISABLED, OUTPUT);
    digitalWrite(LED_SUPPLY_VOLTAGE_DISABLED, false);

    pinMode(RM_ACTIVITY_PIN, INPUT | PULL_DOWN); // smoke detector base plate state, pulldown configured, Pin is connected to 3,3V VCC of the RM
    pinMode(RM_COMM_ENABLE_PIN, OUTPUT);
    digitalWrite(RM_COMM_ENABLE_PIN, RM_COMM_ENABLE); // Kommunikation mit dem RM aktivieren
    pinMode(RM_SUPPORT_VOLTAGE_PIN, OUTPUT);
    digitalWrite(RM_SUPPORT_VOLTAGE_PIN, RM_SUPPORT_VOLTAGE_OFF); // zuerst die Spannungsversorgung ausschalten


	// TODO Alarm-Status vom Rauchmelder abfragen
}
