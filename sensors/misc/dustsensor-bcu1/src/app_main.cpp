/*
 *  A simple example for SPI.
 *
 *  This example configures SPI for output and sends a byte every second.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eibBCU1.h>
#include <sblib/internal/iap.h>
#include <sblib/ioports.h>
#include <sblib/spi.h>
#include <sblib/serial.h>


//#include "app_in8.h"
//#include "com_objs.h"
//#include "params.h"
//#include "Interrupt.h"

//	PIO2_2	IO 1
//	PIO0_7	IO 2
//	PIO2_10 IO 3
//	PIO2_9	IO 4
//	PIO0_2	IO 5  SPI  SSEL0
//	PIO0_8	IO 6  SPI  MISO0
//	PIO0_9	IO 7  SPI  MOSI0
//	PIO2_11	IO 8  SPI  SCK 0


//int blinkPin = PIO2_6;
#define BLINK_PIN 		PIO2_6
#define PEEK_DET_SHARP	PIO1_6
#define ADC_CONF	   	PIO1_5
#define PULSE_SHARP		PIO1_7
#define INFO_LED		PIO2_6
#define RUN_LED			PIO3_3

#define ARRAY_SIZE		1000
#define TRESHHOLD_HIGH  15000			// 0-65535
#define TRESHHOLD_LOW	10000			// vielleicht noetig ???
#define SAMPLE_RATE		10				// Wieviel

#define PARAMETER_BASE 	0x01F4
// Wichtig; Globale Variablen werden in den verschiedenen Funktionen verwendet,

SPI spi(SPI_PORT_0);

BCU1 bcu = BCU1();

APP_VERSION("SBdust_s", "1", "10")

volatile bool timer_expired=false; // Bool  fuer die IF-Abfrage, wenn true wird gesendet

unsigned short int limit;
int min_limit;
int max_limit;

////////////////////////////////////////////////////////////////////////////////////////////////////

void setup_Timer32_0_Interrupt()
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
    timer32_0.match(MAT1, 1000);	//

    timer32_0.start();
}

// Funktion: Messung wird ausgefuehrt
void messung(unsigned short int messwerte[SAMPLE_RATE])
{
    digitalWrite(PULSE_SHARP, true);		// Erzeugt den Puls, der LED, in der Rauchkammer (Eigentliche Messung)
    for(int i=0;i<1400;i++);				// Wartezeit, ab wann gesamplet wird, Erfahrungswerte :0.28 ms warten von Messtechnik

    for(int j=0; j<SAMPLE_RATE; j++)
    {		// Sampleung starten

        digitalWrite(ADC_CONF, true);		// IO 13 ansteuern Flanke erzeugen
        for(int i=0;i<10;i++);				// Abwarten einer gewissen Zeit
        digitalWrite(ADC_CONF, false);

        messwerte[j]=spi.transfer(0);		// Sendet Wert und empfaengt Wert vom Sensor und schreibt ins Messwerte Array[Global]
    }

    digitalWrite(PULSE_SHARP, false);		// LED aus
}

// TimerInterrupt
extern "C" void TIMER32_0_IRQHandler()
{
    // Toggle the Info LED
    digitalWrite(INFO_LED, !digitalRead(INFO_LED));

    // Clear the timer interrupt flags. Otherwise the interrupt handler is called
    // again immediately after returning.
    timer32_0.resetFlags();

	timer_expired=true;
}

/**
 * Initialize the application.
 */
BcuBase* setup()
{
	//bcu.begin(4, 0x7054, 2); 	// We are a "Jung 2118" device, version 0.2
	bcu.begin(76, 0x0442, 3); 	// We are a "EigenDevice" device, version 0.3, eigengebraeu

	// Interruptanweisung fuer die LED
	pinMode(INFO_LED, OUTPUT);	// Info LED
	pinMode(RUN_LED,  OUTPUT);	// Run LED

	// Interrupt_Timer_32_0 aktivieren
	setup_Timer32_0_Interrupt();

    // SPI
    pinMode(PIO0_2,  OUTPUT);
    pinMode(PIO0_9,  OUTPUT | SPI_MOSI);
    pinMode(PIO2_11, OUTPUT | SPI_CLOCK);
    pinMode(PIO0_8,  INPUT  | SPI_MISO);

    // ADC Pins
    pinMode(PEEK_DET_SHARP, OUTPUT);
    digitalWrite(PEEK_DET_SHARP,false); // Setzt den Pin auf 0, sonst deckelt dieser den Sensor
    pinMode(ADC_CONF, 		OUTPUT);
    pinMode(PULSE_SHARP, 	OUTPUT);

    spi.setClockDivider(2);		 	// bei 12 = 1 Mhz  | bei 2= 6 Mhz
    spi.setDataSize(SPI_DATA_16BIT);
    spi.begin();

    /////////////////////////////////////////////////////////////////////////////////
    // UART-Schnittstelle
	serial.begin(115200);

	serial.println("Selfbus serial port example");

	serial.print("Target MCU has ");
	serial.print(iapFlashSize() / 1024);
	serial.println("k flash");
	serial.println();

	// Hier wird das Objekt ausgelesen
	for(int i=0; i<16;i++)
	{
		serial.print("ObjektNr ", i, DEC, 2);
		serial.println(" ", bcu.comObjects->objectSize(i), DEC, 2);
	}

	unsigned short int grenzwert_test = bcu.userEeprom->getUInt16(0x1F5);

	// Auslesung der EEPROM Werte
	serial.println("EEPROM ab 0x1f4:"); // Grenzwert kann hier auch ausgelesen werden.
	serial.println("Grenzwert: ", grenzwert_test);
	for(unsigned int i = 0; i < 12; i++){					// 0 & 1 sind Grenzwert
		serial.println(bcu.userEeprom->getUInt16(0x1F4 + i));
	}
	bcu.comObjects->objectWrite(0,(unsigned int) 0);

	// Grenzwert holen
	limit = bcu.userEeprom->getUInt16(0x1F5); // Grenzwert auslesen! aus der Speicherstelle 0xF4 & 0xF5
	min_limit = limit * 0.9f;
	max_limit = limit * 1.1f;

	return (&bcu);
}

/**
 * The main processing loop.
 */
void loop()
{
	static bool Grenzwert=0;
	unsigned short int messwerte[SAMPLE_RATE] = {0};    // Messwerte die gesampelt wurden, werden hier abgelegt.
    volatile unsigned short int adc_value=0;            // Variable zur  Uebertragung der Werte, ueber KNX und bei UART
    volatile unsigned int on = 1;
    //volatile unsigned int off = 0;
    //volatile bool unterschritten=false;

	// Diese If-Abfrage sendet den Wert auf das KNX, wenn timer_expired auf true gesetzt wird
    if(timer_expired)
    {

    	messung(messwerte);

		// Hier wird nach dem Maxwert gesucht!
		for(int i=0; i<SAMPLE_RATE;i++)	{		// Sucht Max Wert
			if(messwerte[i]>adc_value){			// messwerte[] / value    sind globale variable
				adc_value = messwerte[i];
			}
		}

		// Send to UART
		serial.print("ADC value: ");
		serial.print(adc_value, DEC, 5);
		serial.print("  ADC value: ");
		serial.print(adc_value, HEX, 4);


		// Spannung berechnen und Ausgeben
		float spannung= ((float)4.096/(float)65000)*adc_value;
		int vorkomma= (int)spannung;
		float zwischenrechnung= spannung- (int)(spannung);
		int nachkomma=zwischenrechnung*1000;

		serial.print("   Spannung [V] : ");
		serial.print(vorkomma,DEC,1);
		serial.print(".");
		serial.println(nachkomma,DEC,3);



		// Sendet an den KNX Bus, ob Grenzwert  ueberschritten ist, oder unterschritten ist!!
    	if( max_limit < adc_value) // Schreibt das Grenzwert = 1, wenn Limit um 10% ueberschritten ist
		{
			if(!Grenzwert){
			    bcu.comObjects->objectWrite(0,on);
				Grenzwert=true;
				serial.print("Grenzwert  ueberschritten :");
				serial.println(adc_value);
			}
		}
    	if( min_limit > adc_value)		// Schreibt das Grenzwert 0 ist, wenn Limit um 10% unterschritten ist
		{
			if(Grenzwert) {
				//unterschritten=true;
			    bcu.comObjects->objectWrite(0, (unsigned int) 0);
				Grenzwert=false;
				serial.print("Grenzwert unterschritten :");
				serial.println(adc_value);
			}
		}

    	bcu.comObjects->objectWrite(2, (unsigned int) (adc_value));		// OBjekt wird  uebertragen
		timer_expired= false;
    }

    // Irgendwie sendet obejctWrite keine null, deswegen wurde das aus der grossen If rausgenommen ...
//    if(unterschritten){
//    	objectWrite(0, off);
//    	unterschritten=false;
//    }

    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle())
        waitForInterrupt();
}

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{

}
