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

namespace Faux86
{
	class VM;

	class DisneySoundSource : public SoundCardInterface, PortInterface
	{
	public:
		DisneySoundSource(VM& inVM);

		void init() override;
		void tick() override;

		int16_t generateSample() override;

		virtual bool portWriteHandler(uint16_t portnum, uint8_t value) override;
		virtual bool portReadHandler(uint16_t portnum, uint8_t& outValue) override;

	private:
		static constexpr int bufferLength = 16;

		uint8_t ssourcefull();
		void putssourcebyte(uint8_t value);

		VM& vm;

		uint8_t ssourcebuf[bufferLength];
		uint8_t ssourceptr = 0;
		uint8_t ssourceactive = 0;
		int16_t ssourcecursample = 0;
	};
}

