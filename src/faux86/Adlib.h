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
#include "Audio.h"
#include "Ports.h"
#include "opl3.h"

namespace Faux86
{
	class VM;

	class Adlib : public SoundCardInterface, PortInterface
	{
	public:
		Adlib(VM& inVM);

		void init() override;
		void tick() override;

		int16_t generateSample() override;

		// on the Sound Blaster Pro, ports (base+0) and (base+1) are for
		// the OPL FM music chips, and are also mirrored at (base+8) (base+9)
		// as well as 0x388 and 0x389 to remain compatible with the older adlib cards

		virtual bool portWriteHandler(uint16_t portnum, uint8_t value) override;
		virtual bool portReadHandler(uint16_t portnum, uint8_t& outValue) override;

	private:
		VM& vm;

		opl3_chip opl3;
		uint8_t targetRegister = 0;
		uint8_t timerRegister = 0;
	};
}

