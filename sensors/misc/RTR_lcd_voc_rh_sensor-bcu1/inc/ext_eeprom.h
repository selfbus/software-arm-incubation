#ifndef EXT_EEPROM
#define EXT_EEPROM

#define EEPROM_SIZE 256 //EEPROM size in bytes

//opcodes
#define WREN  6
#define WRDI  4
#define RDSR  5
#define WRSR  1
#define READ  3
#define WRITE 2

class ExtEeprom
{
public:
	/**
	 * Creates a ExtEeprom instance
	 */
	ExtEeprom();

	/**
	 *  initializes the SPI interface 1 for connecting the external EEPROM
	 */
	void init_eeprom(void);

	/*
	 *  Write data to the Buffer from external EEPROM
     *
     *
     * @param Address - a EEPROM address
     * @param data - a pointer to the data that should be written to the address
     * @return 0 on success, else error
	 */
	bool write(unsigned int address, char *data, unsigned int num_bytes);

	/*
	 *  Read data from the Buffer from external EEPROM
	 *
	 * @param Address - a EEPROM address
     * @param data - a pointer to the store variable to get the read data
     * @return 0 on success, else error
	 */
	bool read(unsigned int address, char *data, unsigned int num_bytes);

	/*
	 * Read data directly from the external EEPROM chip
	 *
	 * @param Address - a EEPROM address
     * @param data - a pointer to the store variable to get the read data
     * @return 0 on success, else error
	 */
	bool read_from_chip(unsigned int address, char *data, unsigned int num_bytes);

	/*
	 * Write data directly to the external EEPROM chip
	 *
	 * @param Address - a EEPROM address
     * @param data - a pointer to the data that should be written to the address
     * @return 0 on success, else error
	 */
	bool write_to_chip(void);

	/*
     * Access the external EEPROM to set a unsigned byte
     *
     * @param Address - the address of the data byte to access.
     * @param data - the value to be written
     * @return error value of flash operation
     */
	bool eepromSetUInt8(unsigned int address, unsigned char data);

	/*
     * Access the external EEPROM to set a unsigned short
     *
     * @param Address - the address of the 16 bit data to access.
     * @param data - the value to be written
     * @return error value of flash operation
     */
	bool eepromSetUInt16(unsigned int address, unsigned short data);

	/*
     * Access the external EEPROM to set a unsigned int
     *
     * @param Address - the address of the 32 bit data to access.
     * @param data - the value to be written
     * @return error value of flash operation
     */
	bool eepromSetUInt32(unsigned int address, unsigned int data);

	/*
	 * Access the external EEPROM to get a unsigned byte
	 *
	 * @param virtAddress - the virtual address of the data byte to access.
	 * @return The data byte.
	 */
	unsigned char eepromGetUInt8(unsigned int address);

	 /*
	  * Access the external EEPROM to get a unsigned short
	  *
	  * @param virtAddress - the virtual address of the 16 bit data to access.
	  * @return The 16bit as unsigned short.
	  */
	unsigned short eepromGetUInt16(unsigned int address);

	/*
	 * Access the external EEPROM to get a unsigned int
	 *
	 * @param virtAddress - the virtual address of the 32 bit data to access.
	 * @return The 32bit as unsigned int.
	 */
	unsigned int eepromGetUInt32(unsigned int address);

private:
	bool extEepromModified;
	unsigned int minAddress; //lowest address which has to be written
	unsigned int maxAddress; //highest address which has to be written
	char eepromWriteBuf[EEPROM_SIZE];
};

#endif
