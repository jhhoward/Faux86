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

#include "VM.h"
#include "DisneySoundSource.h"
#include "MemUtils.h"

using namespace Faux86;

int16_t DisneySoundSource::generateSample() 
{
	return (ssourcecursample);
}

void DisneySoundSource::tick() 
{
	uint8_t rotatefifo;
	if ( (ssourceptr==0) || (!ssourceactive) ) 
	{
		ssourcecursample = 0;
		return;
	}
	ssourcecursample = ssourcebuf[0];
	for (rotatefifo = 1; rotatefifo < 16; rotatefifo++)
	{
		ssourcebuf[rotatefifo - 1] = ssourcebuf[rotatefifo];
	}
	ssourceptr--;
	vm.ports.portram[0x379] = 0;
}

void DisneySoundSource::putssourcebyte (uint8_t value) 
{
	if (ssourceptr == bufferLength)
	{
		return;
	}

	ssourcebuf[ssourceptr++] = value;

	if (ssourceptr == bufferLength)
	{
		vm.ports.portram[0x379] = 0x40;
	}
}

uint8_t DisneySoundSource::ssourcefull()
{
	if (ssourceptr == bufferLength) 
		return (0x40);
	else 
		return (0x00);
}

bool DisneySoundSource::portWriteHandler(uint16_t portnum, uint8_t value) 
{
	static uint8_t last37a = 0;
	switch (portnum) 
	{
		case 0x378:
			putssourcebyte (value);
			return true;
		case 0x37A:
			if ( (value & 4) && ! (last37a & 4) ) 
				putssourcebyte (vm.ports.portram[0x378]);
			last37a = value;
			return true;
	}
	return false;
}

bool DisneySoundSource::portReadHandler(uint16_t portnum, uint8_t& outValue)
{
	switch (portnum)
	{
		case 0x379:
			outValue = (DisneySoundSource::ssourcefull());
			return true;
	}
	return false;
}

DisneySoundSource::DisneySoundSource(VM& inVM)
	: vm(inVM)
{
}

void DisneySoundSource::init()
{
	MemUtils::memset(ssourcebuf, 0, bufferLength);

	if (vm.config.useDisneySoundSource)
	{
		vm.ports.setPortRedirector(0x378, 0x378, this);
		vm.ports.setPortRedirector(0x37A, 0x37A, this);
		vm.ports.setPortRedirector(0x379, 0x379, this);
	}
}

