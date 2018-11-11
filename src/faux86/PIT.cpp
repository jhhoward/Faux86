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

/* Functions to emulate the Intel 8253 programmable interval timer.
   these are required for the timer interrupt and PC speaker to be
   properly emulated! */

#include "VM.h"
#include "PIT.h"

using namespace Faux86;

bool PIT::portWriteHandler(uint16_t portnum, uint8_t value)
{
	uint8_t curbyte = 0;
	portnum &= 3;
	switch (portnum) 
	{
		case 0:
		case 1:
		case 2: //channel data
			if ( (accessmode[portnum] == Mode::LoByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 0) ) ) 
				curbyte = 0;
			else if ( (accessmode[portnum] == Mode::HiByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 1) ) ) 
				curbyte = 1;
			if (curbyte == 0) 
			{ //low byte
				chandata[portnum] = (chandata[portnum] & 0xFF00) | value;
			}
			else 
			{   //high byte
				chandata[portnum] = (chandata[portnum] & 0x00FF) | ( (uint16_t) value << 8);
			}
			if (chandata[portnum] == 0) 
				effectivedata[portnum] = 65536;
			else 
				effectivedata[portnum] = chandata[portnum];
			active[portnum] = 1;
			vm.timing.tickgap = (uint64_t) ( (float) vm.timing.getHostFreq() / (float) ( (float) 1193182 / (float) effectivedata[0]) );
			if (accessmode[portnum] == Mode::Toggle) 
				bytetoggle[portnum] = (~bytetoggle[portnum]) & 1;
			chanfreq[portnum] = (float) ( (uint32_t) ( ( (float) 1193182.0 / (float) effectivedata[portnum]) * (float) 1000.0) ) / (float) 1000.0;
			//printf("[DEBUG] PIT channel %u counter changed to %u (%f Hz)\n", portnum, chandata[portnum], chanfreq[portnum]);
			break;
		case 3: //mode/command
			accessmode[value>>6] = (value >> 4) & 3;
			if (accessmode[value>>6] == Mode::Toggle) 
				bytetoggle[value>>6] = 0;
			break;
	}

	return true;
}

bool PIT::portReadHandler(uint16_t portnum, uint8_t& outValue)
{
	outValue = 0;
	uint8_t curbyte = 0;
	portnum &= 3;
	switch (portnum) 
	{
		case 0:
		case 1:
		case 2: //channel data
			if ( (accessmode[portnum] == 0) || (accessmode[portnum] == Mode::LoByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 0) ) ) 
				curbyte = 0;
			else if ( (accessmode[portnum] == Mode::HiByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 1) ) ) 
				curbyte = 1;
			if ( (accessmode[portnum] == 0) || (accessmode[portnum] == Mode::LoByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 0) ) ) 
				curbyte = 0;
			else if ( (accessmode[portnum] == Mode::HiByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 1) ) ) 
				curbyte = 1;
			if ( (accessmode[portnum] == 0) || (accessmode[portnum] == Mode::Toggle) ) 
				bytetoggle[portnum] = (~bytetoggle[portnum]) & 1;
			if (curbyte == 0) 
			{ //low byte
				outValue = ( (uint8_t) counter[portnum]);
			}
			else 
			{   //high byte
				outValue = ( (uint8_t) (counter[portnum] >> 8) );
			}
			return true;
	}
	return true;
}

PIT::PIT(VM& inVM)
	: chandata { 0, 0, 0 }
	, accessmode { 0, 0, 0 }
	, bytetoggle { 0, 0, 0 }
	, effectivedata { 0, 0, 0 }
	, chanfreq { 0, 0, 0 }
	, active { 0, 0, 0 }
	, counter { 0, 0, 0 }
	, vm(inVM)
{
	vm.ports.setPortRedirector(0x40, 0x43, this);
}

