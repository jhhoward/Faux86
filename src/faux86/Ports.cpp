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

/* Functions to handle port I/O from the CPU module, as well
   as functions for emulated hardware components to register their
   read/write callback functions across the port address range. */

#include "VM.h"
#include "Ports.h"
#include "cpu.h"
#include "audio.h"

using namespace Faux86;

Ports::Ports(VM& inVM) : vm(inVM)
{
	reset();
}

void Ports::reset()
{
	for (int n = 0; n < NumPorts; n++)
	{
		portram[n] = 0;
		portHandlers[n] = nullptr;
	}
}

void Ports::outByte (uint16_t portnum, uint8_t value) 
{
	portram[portnum] = value;
	//if (verbose) printf("portout(0x%X, 0x%02X);\n", portnum, value);
	switch (portnum) 
	{
		case 0x61:
			if ((value & 3) == 3)
			{
				vm.pcSpeaker.enabled = true;
			}
			else
			{
				vm.pcSpeaker.enabled = false;
			}
			return;
	}

	if (portHandlers[portnum] && portHandlers[portnum]->portWriteHandler(portnum, value))
	{
		return;
	}
}

uint8_t Ports::inByte (uint16_t portnum) 
{
	//if (verbose) printf("portin(0x%X);\n", portnum);
	switch (portnum) 
	{
		case 0x62:
			return (0x00);
		case 0x60:
		case 0x61:
		case 0x63:
		case 0x64:
			return (portram[portnum]);
	}

	uint8_t result = 0xFF;
	if (portHandlers[portnum] && portHandlers[portnum]->portReadHandler(portnum, result))
	{
		return result;
	}

	return (0xFF);
}

void Ports::outWord (uint16_t portnum, uint16_t value) 
{
	outByte (portnum, (uint8_t) value);
	outByte (portnum + 1, (uint8_t) (value >> 8) );
}

uint16_t Ports::inWord(uint16_t portnum) {
	uint16_t ret;

	ret = (uint16_t) inByte (portnum);
	ret |= (uint16_t) inByte (portnum+1) << 8;
	return (ret);
}

void Ports::setPortRedirector(uint16_t startPort, uint16_t endPort, PortInterface* redirector)
{
	for (uint16_t i = startPort; i <= endPort; i++)
	{
		portHandlers[i] = redirector;
	}
}

