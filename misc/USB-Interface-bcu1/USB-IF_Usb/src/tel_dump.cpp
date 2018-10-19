/*
 *  tel_dump.cpp - Printing KNX packets as debug output
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <GenFifo.h>
#include <stdarg.h>
#include <stdio.h>
#include "chip.h"
#include "string.h"
#include "busdevice_if.h"
#include "buffermgr.h"
#include "knxusb_const.h"

#include "tel_dump.h"

TelDump teldump;

void TelDump::PrintPhyAddr(uint8_t dat0, uint8_t dat1)
{
	buf_printf("%hu.%hu.%hu", dat0 >> 4, dat0 & 0xf, dat1);
}

void TelDump::PrintPacketNo( unsigned no )
{
	buf_printf("#%u ", no);
}

void TelDump::DumpHexData(uint8_t tel[], unsigned int start, unsigned int len)
{
	for (unsigned i=0; i<len;i++)
	{
		buf_printf("%02X ", tel[i+start]);
	}
}

void TelDump::DumpMemData(uint8_t tel[], unsigned int len, unsigned ExtendedOffset, unsigned ItsUserMem)
{
	unsigned Addr = 0;
	if (ItsUserMem)
	{
		Addr = tel[8+ExtendedOffset] << 16;
	}
	Addr += ((tel[8+ExtendedOffset+ItsUserMem] << 8) | tel[9+ExtendedOffset+ItsUserMem]);
	buf_printf("Len 0x%02X Addr 0x%04X Data (hex) ", len, Addr);
	DumpHexData(tel, 10+ExtendedOffset, len);
}

struct TPropDesc
{
	unsigned short id;
	char text[32];
};

// See KNX standard 03_05_01 Resources
TPropDesc PropDesc[] =
{
		{0, "(unknown)"},
		{1, "Interface Object Type"},
		{2, "Interface Object Name"},
		{5, "Load Control"},
		{6, "Run Control"},
		{7, "Table Reference"},
		{8, "Service Control"},
		{9, "Firmware Revision"},
		{10, "Services Supported"},
		{11, "Serial Number"},
		{12, "Manufacturer Identifier"},
		{13, "Program Version"},
		{14, "Device Control"},
		{15, "Order Info"},
		{16, "PEI Type"},
		{17, "PortADDR"},
		{18, "Pollgroup Settings"},
		{19, "Manufacturer Data"},
		{21, "Description"},
		{23, "Table"},
		{25, "Version"},
		{27, "Memory Control Table"},
		{28, "Error Code"},
		{29, "Object Index"},
		{30, "Download Counter"},
		{51, "Routing Count"},
		{52, "MaxRetryCount"},
		{53, "Error Flags"},
		{54, "Programming Mode"},
		{55, "Product Identification"},
		{56, "MAX. APDU-Length"},
		{57, "Subnetwork Address"},
		{58, "Device Address"},
		{59, "PID_CONFIG_LINK"},
		{60, "Address report"},
		{61, "Address Check"},
		{62, "Object Value"},
		{63, "Object Link"},
		{64, "Application"},
		{65, "Parameter"},
		{66, "Object Address"},
		{67, "PSU Type"},
		{68, "PSU Status"},
		{69, "PSU Enable"},
		{70, "Domain Address"},
		{71, "Interface Object List"},
		{72, "Management Descriptor 1"},
		{73, "PL110 Parameters"},
		{75, "BiBat Receive Block Table"},
		{76, "BiBat Random Pause Table"},
		{77, "BiBat Receive Block Number"},
		{78, "Hardware Type"},
		{79, "BiBat Retransmitter Number"},
		{80, "KNX Serial Number Table"},
		{81, "BiBat Master Individual Address"},
		{82, "RF Domain Address"},
		{83, "Device Descriptor"},
		{85, "group tel rate limit time base"},
		{86, "group tel rate limit num of tel"},
		{255, ""}
};

void TelDump::DumpPropValHeader(uint8_t tel[], unsigned int ExDReq)
{
	unsigned Id = tel[9+ExDReq];
	buf_printf(" PropId 0x%02X ObjIdx 0x%02hX NoElem 0x%02hX StartIdx 0x%03X \"", Id, tel[8+ExDReq], tel[10+ExDReq] >> 4, ((tel[10+ExDReq] & 0xf) << 4) + tel[11+ExDReq]);
	unsigned search = 0;
	while (PropDesc[search].id != Id)
	{
		if (PropDesc[search].id == 255)
		{
			search = 0;
			break;
		}
		search++;
	}
	buf_printf("%s\" ", PropDesc[search].text);
}

void TelDump::DbgParseTele(uint8_t tel[], unsigned int len)

{
	unsigned ExDReq = 0;
	unsigned dlen = 0;
	if (len >= 7)
	{
		if ((tel[0] & 0xd3) == 0x90)
		{
			buf_printf("DataReq");
		} else if ((tel[0] & 0xd3) == 0x90)
		{
			buf_printf("ExtDReq");
			ExDReq = 1;
		} else if (tel[0] == 0xf0)
		{
			buf_printf("PollDRq");
		} else
		{
			buf_printf("?? 0x%02X", tel[0]);
		}
		buf_printf(" Prio %hu Repeat %hu Src ",(tel[0] >> 2) & 3, 1-((tel[0] >> 5) & 1));
		PrintPhyAddr(tel[1], tel[2]);
		buf_printf(" Trg ");
		if (tel[5] & 0x80)
		{
			buf_printf("%hu/%hu/%hu", tel[3] >> 3, tel[3] & 0x7, tel[4]);
		} else {
			PrintPhyAddr(tel[3], tel[4]);
		}
		buf_printf(" RoutC %hu", (tel[5] >> 4) & 7);
		if (ExDReq)
		{
			dlen = tel[6];
		} else {
			dlen = tel[5] & 15;
		}
		buf_printf(" Len %hu", dlen);
		// TPCI/APCI
		unsigned tpci = tel[6+ExDReq] >> 6;
		unsigned apci = tel[6+ExDReq] & 3;
		switch (tpci)
		{
		case 0:
			// UDP Unnumbered Data Packet
			buf_printf(" UDP ");
			break;
		case 1:
			// NDP Numbered DataPacket
			buf_printf(" NDP ");
			PrintPacketNo( (tel[6+ExDReq] >> 2) & 0xf );
			break;
		case 2:
			// UCD Unnumbered Control Data
			buf_printf(" UCD ");
			switch (apci)
			{
			case 0:
				buf_printf("Connect ");
				break;
			case 1:
				buf_printf("Disconnect ");
				break;
			default:
				buf_printf("???? ");
			}
			break;
			case 3:
				// NCD Numbered Control Data
				buf_printf(" NCD ");
				PrintPacketNo( (tel[6+ExDReq] >> 2) & 0xf );
				switch (apci)
				{
				case 2:
					buf_printf("ACK  ");
					break;
				case 3:
					buf_printf("NACK ");
					break;
				default:
					buf_printf("???? ");
				}
		}
		if ((tpci & 2) == 0)
		{
			// See KNX standard 03_03_07 Application Layer
			apci <<= 8;
			apci |= tel[7+ExDReq];
			if (apci == 0)
			{
				buf_printf("Group value Read");
			} else if ((apci & 0x3c0) == 0x040) {
				buf_printf("Group value Response (hex) ");
				if (dlen > 1)
				{
					DumpHexData(tel, 8+ExDReq, dlen-1);
				} else {
					buf_printf("(6bit) %02X", apci & 0x3f);
				}
			} else if ((apci & 0x3c0) == 0x080) {
				buf_printf("Group value Write (hex) ");
				if (dlen > 1)
				{
					DumpHexData(tel, 8+ExDReq, dlen-1);
				} else {
					buf_printf("(6bit) %02X", apci & 0x3f);
				}
			} else if (apci == 0x0c0) {
				buf_printf("Phys Addr Write ");
				PrintPhyAddr(tel[8+ExDReq], tel[9+ExDReq]);
			} else if (apci == 0x100) {
				buf_printf("Phys Addr Read");
			} else if (apci == 0x140) {
				buf_printf("Phys Addr Response");
			} else if (apci == 0x3dc) {
				buf_printf("Individual Address SN Read (hex) ");
				DumpHexData(tel, 8+ExDReq, dlen-1);
			} else if (apci == 0x3dd) {
				buf_printf("Individual Address SN Response (hex) ");
				DumpHexData(tel, 8+ExDReq, dlen-1);
			} else if (apci == 0x3de) {
				buf_printf("Individual Address SN Write (hex) ");
				DumpHexData(tel, 8+ExDReq, dlen-1);
			} else if (apci == 0x3e0) {
				buf_printf("Set Sys ID   ");
			} else if (apci == 0x3e1) {
				buf_printf("Req Sys ID   ");
			} else if (apci == 0x3e2) {
				buf_printf("Sys ID Resp  ");
			} else if (apci == 0x3e1) {
				buf_printf("Req Sys ID?  ");
			} else if ((apci & 0x3f0) == 0x200) {
				buf_printf("Memory Read - Len 0x%02X Addr 0x%04X", apci & 0x00f, (tel[8+ExDReq] << 8) | tel[9+ExDReq]);
			} else if ((apci & 0x3f0) == 0x240) {
				buf_printf("Memory Response - ");
				DumpMemData(tel, dlen-3, ExDReq);
			} else if ((apci & 0x3f0) == 0x280) {
				buf_printf("Memory Write - ");
				DumpMemData(tel, dlen-3, ExDReq);
				//} else if (apci == 0x2d0) {
				// buf_printf("Memory CC Tx - Short - ? ");
			} else if ((apci & 0x3c0) == 0x180) {
				buf_printf("ADC Read - Ch%u NoCnv ", apci & 0x3f, tel[8+ExDReq]);
			} else if ((apci & 0x3c0) == 0x1c0) {
				buf_printf("ADC Response - Ch%u NoCnv %hu Val 0x%04X", apci & 0x3f, tel[8+ExDReq], (tel[9+ExDReq] << 8) | tel[10+ExDReq]);
			} else if (apci == 0x300) {
				buf_printf("Mask Version Read");
			} else if (apci == 0x340) {
				buf_printf("Mask Version Response 0x%04X", (tel[8+ExDReq] << 8) | tel[9+ExDReq]);
			} else if (apci == 0x380) {
				buf_printf("Restart! ");
				DumpHexData(tel, 8+ExDReq, dlen-1);
			} else if (apci == 0x3d5) {
				buf_printf("Prop Value Read - ");
				DumpPropValHeader(tel, ExDReq);
			} else if (apci == 0x3d6) {
				buf_printf("Prop Value Response - ");
				DumpPropValHeader(tel, ExDReq);
				DumpHexData(tel, 12+ExDReq, dlen-5);
			} else if (apci == 0x3d7) {
				buf_printf("Prop Value Write - ");
				DumpPropValHeader(tel, ExDReq);
				DumpHexData(tel, 12+ExDReq, dlen-5);
			} else if (apci == 0x3d8) {
				buf_printf("Prop Desc Read ");
			} else if (apci == 0x3d9) {
				buf_printf("Prop Desc Response ");
			} else if (apci == 0x2c0) {
				buf_printf("UserMem Read - Len %u Addr 0x%04X", apci & 0x00f, (tel[8+ExDReq] << 8) | tel[9+ExDReq]);
			} else if (apci == 0x2c1) {
				buf_printf("UserMem Resp - ");
				DumpMemData(tel, dlen-3, ExDReq, 1);
			} else if (apci == 0x2c2) {
				buf_printf("UserMem Write - ");
				DumpMemData(tel, dlen-3, ExDReq, 1);
				//   } else if (apci == 0x2c3) {
				//    buf_printf("Mem AC TX short - ?");
			} else if (apci == 0x2c5) {
				buf_printf("UserMfgInfo Read");
			} else if (apci == 0x2c6) {
				buf_printf("UserMfgInfo Response");
			} else if ((apci & 0x3f8) == 0x2f8) {
				buf_printf("Mfg Specific - %u", apci & 7);
			} else if (apci == 0x3d1) {
				buf_printf("Authorize Request - Data (hex) ");
				DumpHexData(tel, 8+ExDReq, dlen-1);
			} else if (apci == 0x3d2) {
				buf_printf("Authorize Response - Data (hex) ");
				DumpHexData(tel, 8+ExDReq, dlen-1);
			} else if (apci == 0x3d3) {
				buf_printf("Authorize Key Write - ?");
			} else if (apci == 0x3d4) {
				buf_printf("Authorize Key Response - ?");
			} else if (apci == 0x2c7) {
				buf_printf("Function Property Command ");
			} else if (apci == 0x2c8) {
				buf_printf("Function Property State Read - ?");
			} else if (apci == 0x2c9) {
				buf_printf("Function Property State Response - ?");
			} else if (apci == 0x3da) {
				buf_printf("Network Parameter Read - ");
				DumpHexData(tel, 8+ExDReq, dlen-1);
			} else if (apci == 0x3db) {
				buf_printf("Network Parameter Response - ");
				DumpHexData(tel, 8+ExDReq, dlen-1);
			} else {
				buf_printf("?? Unknown APCI 0x%03X", apci);
			}
		}
	} else {
		buf_printf("?? Paket too short ??");
	}
	buf_printf("\r\n");
}

void TelDump::buf_printf(const char *fmt, ...)
{
	va_list args;
	unsigned actlen;
	if (remlen)
	{
		va_start(args, fmt);
		actlen = vsnprintf(linebuffer, remlen, fmt, args);
		remlen -= actlen;
		linebuffer += actlen;
		va_end(args);
	}
}

// Den evtl. langen String in 64 Byte Häppchen splitten und im Cdc-Fifo einreihen
void Split_CdcEnqueue(char* ptr, unsigned len)
{
	while (len)
	{
		unsigned partlen = len;
		if (partlen > 64)
			partlen = 64;
		int buffno = buffmgr.AllocBuffer();
		if (buffno < 0)
			return; // Kein Speicher frei? -> abbrechen
		uint8_t* partptr = buffmgr.buffptr(buffno);
		*partptr++ = partlen+3; // Länge
		*partptr++ = 0; // Checksumme (unbenutzt)
		*partptr++ = 3; // Kennung für CDC (hier eigentlich unnötig)
		memcpy(partptr, ptr, partlen);
		ptr+=partlen;
		len-=partlen;
  	if (cdc_txfifo.Push(buffno) != TFifoErr::Ok)
  	{
  		int no;
  		while (cdc_txfifo.Pop(no) == TFifoErr::Ok)
  		{
  			buffmgr.FreeBuffer(no);
  		}
  	}
	}
}

// Auf was zeigt buffptr? Auf das noch weiter verpackte 64+2 Paket!
// Dann den Pointer intern anpassen!
int TelDump::Dump(bool DirSend, uint8_t* buffptr)
{
	char line[320];
	linebuffer = &(line[0]);
	remlen = sizeof(line)-1;

	if (CdcDeviceMode != TCdcDeviceMode::BusMon)
		return 0;

	unsigned tellen = buffptr[0];
	if ((tellen > 2) && (tellen < 66))
	{
		tellen -= 2+C_HRH_HeadLen+C_TPH_HeadLen+A_TPB_Data;
		buffptr += 2+C_HRH_HeadLen+C_TPH_HeadLen+A_TPB_Data;
		DbgParseTele(buffptr, tellen);
		if (DirSend)
			buf_printf("TX: ");
		else
			buf_printf("RX: ");
		for (unsigned i = 0; i < tellen; ++i)
		{
			if (i) buf_printf(" ");
			buf_printf("%02X", buffptr[i]);
		}
		buf_printf("\r\n");
		Split_CdcEnqueue(&(line[0]), strlen(line));
		return 1;
	}
	return 0;
}
