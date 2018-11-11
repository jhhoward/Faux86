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

#include "VM.h"
#include "SerialMouse.h"
#include "MemUtils.h"

using namespace Faux86;

void SerialMouse::bufsermousedata (uint8_t value) 
{
	if (bufptr == 16) return;
	if (bufptr == 0 ) vm.pic.doirq (vm.config.mouse.irq);
	buf[bufptr++] = value;
}

bool SerialMouse::portWriteHandler(uint16_t portnum, uint8_t value)
{
	uint8_t oldreg;
	//printf("[DEBUG] Serial mouse, port %X out: %02X\n", portnum, value);
	portnum &= 7;
	oldreg = reg[portnum];
	reg[portnum] = value;
	switch (portnum) 
	{
		case 4: //modem control register
			if ( (value & 1) != (oldreg & 1) ) { //software toggling of this register
					bufptr = 0; //causes the mouse to reset and fill the buffer
					bufsermousedata ('M'); //with a bunch of ASCII 'M' characters.
					bufsermousedata ('M'); //this is intended to be a way for
					bufsermousedata ('M'); //drivers to verify that there is
					bufsermousedata ('M'); //actually a mouse connected to the port.
					bufsermousedata ('M');
					bufsermousedata ('M');
				}
			break;
	}
	return true;
}

bool SerialMouse::portReadHandler(uint16_t portnum, uint8_t& outValue) 
{
	//printf("[DEBUG] Serial mouse, port %X in\n", portnum);
	portnum &= 7;
	switch (portnum) 
	{
		case 0: //data receive
			outValue = buf[0];
			MemUtils::memmove (buf, &buf[1], 15);
			bufptr--;
			if (bufptr < 0) 
				bufptr = 0;
			if (bufptr > 0) 
				vm.pic.doirq (vm.config.mouse.irq);
			reg[4] = ~reg[4] & 1;
			return true;
		case 5: //line status register (read-only)
			if (bufptr > 0) 
				outValue = 1;
			else 
				outValue = 0;
			outValue = 0x1;
			//outValue |= 0x60; // TODO
			return true;
	}
	outValue = (reg[portnum & 7]);
	return true;
}


void SerialMouse::triggerEvent (uint8_t buttons, int8_t xrel, int8_t yrel) 
{
	uint8_t highbits = 0;
	if (xrel < 0) highbits = 3;
	else highbits = 0;
	if (yrel < 0) highbits |= 12;
	bufsermousedata (0x40 | (buttons << 4) | highbits);
	bufsermousedata (xrel & 63);
	bufsermousedata (yrel & 63);
}

SerialMouse::SerialMouse(VM& inVM)
	: bufptr(0)
	, vm(inVM)
{
}

void SerialMouse::init()
{
	uint16_t basePort = vm.config.mouse.port;
	vm.ports.setPortRedirector(basePort, basePort + 7, this);
}

void SerialMouse::handleButtonDown(ButtonType button)
{
	switch (button)
	{
	case ButtonType::Left:
		buttonState |= 2;
		break;
	case ButtonType::Right:
		buttonState |= 1;
		break;
	}

	triggerEvent(buttonState, 0, 0);
}

void SerialMouse::handleButtonUp(ButtonType button)
{
	switch (button)
	{
	case ButtonType::Left:
		buttonState &= ~2;
		break;
	case ButtonType::Right:
		buttonState &= ~1;
		break;
	}

	triggerEvent(buttonState, 0, 0);
}

void SerialMouse::handleMove(int8_t xrel, int8_t yrel)
{
	triggerEvent(buttonState, xrel, yrel);
}
