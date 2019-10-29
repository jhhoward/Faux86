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

	class DMA : public PortInterface
	{
	public:
		DMA(VM& inVM);

		void init();
		uint8_t read(uint8_t channel);

		virtual bool portWriteHandler(uint16_t portnum, uint8_t value) override;
		virtual bool portReadHandler(uint16_t portnum, uint8_t& outValue) override;

	private:

		struct Channel
		{
			uint32_t page;
			uint32_t addr;
			uint32_t reload;
			uint32_t count;
			uint8_t direction;
			uint8_t autoinit;
			uint8_t writemode;
			uint8_t masked;
		};

		static constexpr int NumDMAChannels = 4;

		Channel channels[NumDMAChannels];
		VM& vm;
		uint8_t flipflop = 0;
	};

}

