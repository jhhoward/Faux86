/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "config.h"
#ifdef DISK_CONTROLLER_ATA
#include <stdint.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>

uint8_t idATA[512];

#define ATA_STATUS_ERROR 0x01
#define ATA_STATUS_DRQ   0x08
#define ATA_STATUS_SRV   0x10
#define ATA_STATUS_FAULT 0x20
#define ATA_STATUS_READY 0x40
#define ATA_STATUS_BUSY  0x80

#define flip16(x) ((x&255) | (x>>8))
#define flip32(x) ((x>>24) | (((x>>16)&255)<<8) | (((x>>8)&255)<<16) | ((x&255)<<24))

uint8_t statusreg = 0, errorreg = 0, drivesel = 0, databuf[512];
uint16_t dataptr = 512;

void bufclear() {
	memset (databuf, 0, 512);
}

void bufcook() {
	uint16_t i;
	uint8_t cc;
	for (i=0; i<512; i+=2) {
			cc = databuf[i];
			databuf[i] = databuf[i+1];
			databuf[i+1] = cc;
		}
}

void bufwrite8 (uint16_t bufpos, uint8_t value) {
	databuf[bufpos] = value;
}

void bufwrite16 (uint16_t bufpos, uint16_t value) {
	databuf[bufpos] = (uint8_t) value;
	databuf[bufpos+1] = (uint8_t) (value>>8);
}

void flipstring (uint8_t *dest, uint8_t *src) {
	uint16_t i;
	uint8_t cc;
	strcpy (dest, src);
	for (i=0; i<strlen (dest); i+=2) {
			cc = databuf[i];
			databuf[i] = databuf[i+1];
			databuf[i+1] = cc;
		}
}

void cmdATA (uint8_t value) {
	switch (value) {
			case 0x91: //INITIALIZE DEVICE PARAMETERS
				dataptr = 512;
				break;
			case 0xEC: //IDENTIFY
				memset (&idATA, 0, sizeof (idATA) );
				memcpy (databuf, &idATA, 512);
				dataptr = 0;
				break;
		}
}

void outATA (uint16_t portnum, uint8_t value) {
	//printf("[DEBUG] ATA port %Xh write: %02X\n", portnum, value);
	//getch();
	switch (portnum) {
			case 0x1F6:
				drivesel = (value >> 4) & 1;
				break;
			case 0x1F7: //command register
				cmdATA (value);
				break;
		}
}

uint8_t inATA (uint16_t portnum) {
	//if (portnum != 0x1F0) printf("[DEBUG] ATA port %Xh read\n", portnum);
	//getch();
	switch (portnum) {
			case 0x1F0: //data read
				if (dataptr < 512) {
						//printf("%c", databuf[dataptr]);
						return (databuf[dataptr++]);
					}
				else return (0);
			case 0x1F1: //error register
				if (drivesel == 1) return (1);
				else return (0);
			case 0x1F7: //status register
				statusreg = ATA_STATUS_READY;
				if (drivesel == 1) statusreg |= ATA_STATUS_ERROR;
				if (dataptr < 512) statusreg |= ATA_STATUS_DRQ;
				return (statusreg);
		}
	return (0);
}
#endif
