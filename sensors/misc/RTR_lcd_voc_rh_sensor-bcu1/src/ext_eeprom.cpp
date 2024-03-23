#include "ext_eeprom.h"
#include <sblib/core.h>
#include <sblib/spi.h>
#include <sblib/io_pin_names.h>

SPI eeprom_spi(SPI_PORT_1);

ExtEeprom::ExtEeprom(){
	minAddress = EEPROM_SIZE;
	maxAddress = 0;
	extEepromModified = false;
}

void ExtEeprom::init_eeprom(void){

    pinMode(PIN_SSEL1, OUTPUT); //wird manuell bedient
    pinMode(PIN_MOSI1, OUTPUT | SPI_MOSI);
    pinMode(PIN_MISO1, INPUT  | SPI_MISO);
    pinMode(PIN_SCK1,  OUTPUT | SPI_CLOCK);

    eeprom_spi.setClockDivider(128);
    eeprom_spi.begin();

    read_from_chip(0, eepromWriteBuf, EEPROM_SIZE);


}

bool ExtEeprom::write(unsigned int address, char *data, unsigned int num_bytes){
	if(address < minAddress){
		minAddress = address;
	}
	if(address+num_bytes-1 > maxAddress){
		maxAddress = address+num_bytes-1;
	}

	for(unsigned int i=0; i<num_bytes; i++){
		eepromWriteBuf[address+i] = data[i];
	}
	extEepromModified = true;
	return 0;
}


bool ExtEeprom::read(unsigned int address, char *data, unsigned int num_bytes){
	for(unsigned int i=0; i<num_bytes; i++){
		data[i]=eepromWriteBuf[address+i];
	}
	return 0;
}

bool ExtEeprom::read_from_chip(unsigned int address, char *data, unsigned int num_bytes){
	digitalWrite(PIN_SSEL1, 0);
	eeprom_spi.transfer(READ);
	eeprom_spi.transfer(address);
	for(unsigned int i=0; i<num_bytes; i++){
		data[i]=eeprom_spi.transfer(0x00);
	}
	digitalWrite(PIN_SSEL1, 1);
	return 0;
}

bool ExtEeprom::write_to_chip(){
	if(extEepromModified){

		unsigned int numBytes = maxAddress - minAddress + 1;
		unsigned int num_pages = (numBytes / 16) + 1;
		unsigned int num_bytes_per_page;
		unsigned int startAddress;
		unsigned int endAddress;

		//loop for more than 16 bytes to write
		for(unsigned int j=0; j<num_pages; j++){ //j=page counter

			digitalWrite(PIN_SSEL1, 0);
			eeprom_spi.transfer(WREN); //enable writing
			digitalWrite(PIN_SSEL1, 1);
			delay(1);

			num_bytes_per_page = numBytes - j*16;
			if(num_bytes_per_page > 16){
				num_bytes_per_page = 16;
			}
			startAddress = minAddress + j*16;
			endAddress = startAddress+num_bytes_per_page-1;

			digitalWrite(PIN_SSEL1, 0);
			eeprom_spi.transfer(WRITE);
			eeprom_spi.transfer(startAddress);
			for(unsigned int i=startAddress; i<=endAddress; i++){ // i= byte counter
				eeprom_spi.transfer(eepromWriteBuf[i]);
			}
			digitalWrite(PIN_SSEL1, 1);
			delay(10);
			digitalWrite(PIN_SSEL1, 0);
			eeprom_spi.transfer(WRDI); //disable writing
			digitalWrite(PIN_SSEL1, 1);

		}
	}
	return 0;
}

bool ExtEeprom::eepromSetUInt8(unsigned int address, unsigned char data){
	return write(address, (char*)&data, 1);;
}
bool ExtEeprom::eepromSetUInt16(unsigned int address, unsigned short data){
	return write(address, (char*)&data, 2);
}
bool ExtEeprom::eepromSetUInt32(unsigned int address, unsigned int data){
	return write(address, (char*)&data, 4);
}

unsigned char ExtEeprom::eepromGetUInt8(unsigned int address){
	unsigned char ret;
	read(address, (char*)&ret, 1);
	return ret;
}
unsigned short ExtEeprom::eepromGetUInt16(unsigned int address){
	unsigned short ret;
	read(address, (char*)&ret, 2);
	return ret;
}
unsigned int ExtEeprom::eepromGetUInt32(unsigned int address){
	unsigned int ret;
	read(address, (char*)&ret, 4);
	return ret;
}
