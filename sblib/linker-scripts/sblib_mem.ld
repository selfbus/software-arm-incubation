/*
 * (c) Code Red Technologies Ltd, 2008-2013
 * (c) NXP Semiconductors 2013-2013
 * Linker script memory definitions
 * Created from LinkMemoryTemplate
 * By LPCXpresso v6.1.0 [Build 164] [2013-10-21]  on Mon Dec 09 10:10:48 CET 2013)
*/

MEMORY
{
  /* Define each memory region */
  MFlash32 (rx) : ORIGIN = 0x0, LENGTH = 0x7800 /* 30K bytes */
  EEPROM (rx) : ORIGIN = 0x7800, LENGTH = 0x800 /* 2K bytes */
  RamLoc8 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x2000 /* 8K bytes */


}
  /* Define a symbol for the top of each memory region */
  __top_MFlash32 = 0x0 + 0x7800;
  __top_EEPROM = 0x7800 + 0x800;
  __top_RamLoc8 = 0x10000000 + 0x2000;



