/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Base on Fake86
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

/* netcard.c: this is a simple custom ethernet adapter interface for Faux86.
   it's not modeled after a real-world ethernet adapter, thus i had to create
   own DOS packet driver for programs to make use of it. this packet driver
   is included with this source code, in the data/ directory. the filename is
   pd.com - inject this file into any floppy or hard disk image if needed! */

#include "config.h"
#ifdef NETWORKING_ENABLED
#ifdef NETWORKING_OLDCARD
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cpu.h"
#include "ram.h"
#include "packet.h"

struct netstruct {
	uint8_t enabled;
	uint8_t canrecv;
	uint16_t pktlen;
} net;

void nethandler() {
	uint32_t i;
	if (ethif==254) return; //networking not enabled
	switch (regs.byteregs[regah]) { //function number
			case 0x00: //enable packet reception
				net.enabled = 1;
				net.canrecv = 1;
				return;
			case 0x01: //send packet of CX at DS:SI
				if (verbose) {
						printf ("Sending packet of %u bytes.\n", regs.wordregs[regcx]);
					}
				sendpkt (&RAM[ ( (uint32_t) segregs[regds] << 4) + (uint32_t) regs.wordregs[regsi]], regs.wordregs[regcx]);
				return;
			case 0x02: //return packet info (packet buffer in DS:SI, length in CX)
				segregs[regds] = 0xD000;
				regs.wordregs[regsi] = 0x0000;
				regs.wordregs[regcx] = net.pktlen;
				return;
			case 0x03: //copy packet to final destination (given in ES:DI)
				memcpy (&RAM[ ( (uint32_t) segregs[reges] << 4) + (uint32_t) regs.wordregs[regdi]], &RAM[0xD0000], net.pktlen);
				return;
			case 0x04: //disable packets
				net.enabled = 0;
				net.canrecv = 0;
				return;
			case 0x05: //DEBUG: dump packet (DS:SI) of CX bytes to stdout
				for (i=0; i<regs.wordregs[regcx]; i++) {
						printf ("%c", RAM[ ( (uint32_t) segregs[regds] << 4) + (uint32_t) regs.wordregs[regsi] + i]);
					}
				return;
			case 0x06: //DEBUG: print milestone string
				//print("PACKET DRIVER MILESTONE REACHED\n");
				return;
		}
}
#endif
#endif
