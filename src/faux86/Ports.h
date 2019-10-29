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
#pragma once

#include "Types.h"

namespace Faux86
{
	class VM;

	class PortInterface
	{
	public:
		virtual bool portWriteHandler(uint16_t portnum, uint8_t value) { return false; }
		virtual bool portReadHandler(uint16_t portnum, uint8_t& outValue) { return false; }
	};

	class Ports
	{
		static constexpr int NumPorts = 0x10000;

	public:
		Ports(VM& inVM);

		void reset();

		void outByte(uint16_t portnum, uint8_t value);
		void outWord(uint16_t portnum, uint16_t value);
		uint8_t	inByte(uint16_t portnum);
		uint16_t inWord(uint16_t portnum);

		void setPortRedirector(uint16_t startPort, uint16_t endPort, PortInterface* redirector);

		uint8_t portram[NumPorts];

	private:
		VM& vm;

		PortInterface* portHandlers[NumPorts];
	};
}
