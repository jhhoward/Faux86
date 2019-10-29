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
#include "Ports.h"

namespace Faux86
{
	class VM;

	// Intel 8253 Programmable Interval Timer
	class PIT : public PortInterface
	{
	public:
		PIT(VM& inVM);

		uint16_t chandata[3];
		uint8_t accessmode[3];
		uint8_t bytetoggle[3];
		uint32_t effectivedata[3];
		float chanfreq[3];
		uint8_t active[3];
		uint16_t counter[3];

		virtual bool portWriteHandler(uint16_t portnum, uint8_t value) override;
		virtual bool portReadHandler(uint16_t portnum, uint8_t& outValue) override;

	private:
		enum Mode
		{
			LatchCount = 0,
			LoByte = 1,
			HiByte = 2,
			Toggle = 3
		};

		VM& vm;
	};
}

