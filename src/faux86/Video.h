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
#include "Ports.h"

namespace Faux86
{
	class VM;

	class Palette
	{
	public:
		struct Entry
		{
			inline void set(uint8_t inR, uint8_t inG, uint8_t inB)
			{
				r = inR; g = inG; b = inB;
			}
			uint8_t r, g, b, a;
		};

		Palette();

		inline void set(int index, uint8_t r, uint8_t g, uint8_t b)
		{
			colours[index].r = r;
			colours[index].g = g;
			colours[index].b = b;
			colours[index].a = 0xff;
		}

		inline void set(int index, const Entry& colour)
		{
			colours[index].r = colour.r;
			colours[index].g = colour.g;
			colours[index].b = colour.b;
			colours[index].a = colour.a;
		}

		Entry colours[256];
	};

	class Video : public PortInterface
	{
	public:
		Video(VM& inVM);

		uint16_t	VGA_SC[0x100], VGA_CRTC[0x100], VGA_ATTR[0x100], VGA_GC[0x100];
		uint8_t	vidmode;
		uint8_t updatedscreen;
		uint32_t usefullscreen;
		
		static constexpr int VRAMSize = 0x40000;
		uint8_t VRAM[VRAMSize];

		uint8_t cgabg, blankattr, vidgfxmode, vidcolor;
		uint16_t cols = 80; 
		uint16_t rows = 25;
		uint16_t vgapage, cursorposition, cursorvisible;
		uint8_t fontcga[32768];
		//uint32_t palettecga[16], palettevga[256];
		uint8_t clocksafe, port3da, port6;
		uint32_t videobase = 0xB8000;
		static constexpr uint32_t textbase = 0xB8000;
		uint16_t vtotal = 0;

		void handleInterrupt();
		uint8_t readVGA(uint32_t addr32);
		void writeVGA(uint32_t addr32, uint8_t value);

		virtual bool portWriteHandler(uint16_t portnum, uint8_t value) override;
		virtual bool portReadHandler(uint16_t portnum, uint8_t& outValue) override;

		Palette* getCurrentPalette() { return currentPalette; }

	private:
		uint32_t rgb(uint32_t r, uint32_t g, uint32_t b);

		VM& vm;
		uint8_t lastmode = 0;
		uint8_t latchRGB = 0, latchPal = 0, VGA_latch[4], stateDAC = 0;
		uint8_t latchReadRGB = 0, latchReadPal = 0;
		Palette::Entry tempRGB;

		Palette* currentPalette;

		Palette paletteVGA;
		Palette paletteCGA;
	};
}
