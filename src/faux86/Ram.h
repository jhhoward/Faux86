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
#pragma once

#include "Types.h"

namespace Faux86
{
	class VM;
	class DiskInterface;

	class Memory
	{
	public:
		Memory(VM& inVM);
		~Memory();

		void reset();

		uint16_t readWord(uint32_t addr32);
		uint8_t readByte(uint32_t addr32);
		void writeWord(uint32_t addr32, uint16_t value);
		void writeByte(uint32_t addr32, uint8_t value);

		uint32_t loadBinary(uint32_t addr32, DiskInterface* file, uint8_t roflag, uint32_t debugFlags = 0);

		uint8_t* RAM;
		uint8_t* readonly;

	private:
		VM& vm;
	};
}


