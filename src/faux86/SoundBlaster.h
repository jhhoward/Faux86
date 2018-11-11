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
#include "Audio.h"
#include "Ports.h"

namespace Faux86
{
	class VM;
	class Adlib;

	class SoundBlaster : public SoundCardInterface, PortInterface
	{
	public:
		SoundBlaster(VM& inVM, Adlib& inAdlib);

		void init() override;
		void tick() override;

		int16_t generateSample() override;

		uint64_t sampleticks = 0;
		uint16_t samplerate = 0;

		virtual bool portWriteHandler(uint16_t portnum, uint8_t value) override;
		virtual bool portReadHandler(uint16_t portnum, uint8_t& outValue) override;

	private:
		void cmd(uint8_t value);
		void bufNewData(uint8_t value);
		void setsampleticks();

		VM& vm;
		Adlib& adlib;

		uint8_t mem[1024];
		uint16_t memptr = 0;
		uint8_t dspmaj = 0;
		uint8_t dspmin = 0;
		uint8_t speakerstate = 0;
		uint8_t lastresetval = 0;
		uint8_t lastcmdval = 0;
		uint8_t lasttestval = 0;
		uint8_t waitforarg = 0;
		uint8_t paused8 = 0;
		uint8_t paused16 = 0;
		uint8_t sample = 0;
		uint8_t sbirq = 0;
		uint8_t sbdma = 0;
		uint8_t usingdma = 0;
		uint8_t maskdma = 0;
		uint8_t useautoinit = 0;
		uint32_t blocksize = 0;
		uint32_t blockstep = 0;

		uint8_t mixer[256];
		uint8_t mixerindex = 0;

		/*struct {
			uint8_t index = 0;
			uint8_t reg[256];
		} mixer;*/
	};
}


