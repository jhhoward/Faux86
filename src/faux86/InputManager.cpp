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
#include "SerialMouse.h"
#include "ports.h"
#include "cpu.h"
#include "SerialMouse.h"
#include "video.h"

using namespace Faux86;

void InputManager::tick() 
{
	if (!keyboardWaitAck && keyboardBufferSize > 0)
	{
		vm.ports.portram[0x60] = keyboardBuffer[keyboardBufferPos];
		vm.ports.portram[0x64] |= 2;
		vm.pic.doirq(1);
		keyboardWaitAck = true;

		keyboardBufferPos++;
		keyboardBufferSize--;
		if (keyboardBufferPos >= MaxKeyboardBufferSize)
		{
			keyboardBufferPos = 0;
		}
	}
}

void InputManager::handleKeyDown(uint16_t scancode)
{
	uint8_t extension = (uint8_t)(scancode >> 8);
	if (extension)
	{
		queueData(extension);
	}

	queueData((uint8_t)scancode);
}

void InputManager::handleKeyUp(uint16_t scancode)
{
	uint8_t extension = (uint8_t)(scancode >> 8);
	if (extension)
	{
		queueData(extension);
	}

	queueData((uint8_t)(scancode) | 0x80);
}

void InputManager::queueData(uint8_t data)
{
	if (keyboardBufferSize < MaxKeyboardBufferSize)
	{
		int writePos = (keyboardBufferPos + keyboardBufferSize) % MaxKeyboardBufferSize;
		keyboardBuffer[writePos] = data;
		keyboardBufferSize++;
	}
}

InputManager::InputManager(VM& inVM)
	: vm(inVM)
{

}
