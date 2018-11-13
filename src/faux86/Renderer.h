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
#include "TaskManager.h"
#include "mutex.h"

#ifdef _WIN32
#include <assert.h>
#endif

namespace Faux86
{
	class VM;

	struct RenderSurface
	{
		static RenderSurface* create(uint32_t inWidth, uint32_t inHeight);
		static void destroy(RenderSurface* surface);

		inline void set(uint32_t x, uint32_t y, uint8_t col)
		{
#ifdef _WIN32
			assert(x < width && y < height);
#endif
			pixels[y * pitch + x] = col;
		}

		inline uint8_t get(uint32_t x, uint32_t y)
		{
#ifdef _WIN32
			assert(x < width && y < height);
#endif
			return pixels[y * pitch + x];
		}

		uint8_t* pixels;
		uint32_t width, height, pitch;
	};

	class Renderer
	{
		friend class RenderTask;

	public:
		Renderer(VM& inVM);
		~Renderer();

		void init();
		void markScreenModeChanged(uint32_t newWidth, uint32_t newHeight);
		void draw();
		void onMemoryWrite(uint32_t address, uint8_t value);
		void setCursorPosition(uint32_t x, uint32_t y);

		RenderSurface* renderSurface = nullptr;
		RenderSurface* hostSurface = nullptr;

	private:
		void markTextDirty(uint32_t x, uint32_t y);
		void refreshTextMode();
		void renderTextMode();		
		void createScaleMap();

		void simpleBlit();
		void stretchBlit();
		void roughBlit();
		void doubleBlit();

		bool screenModeChanged = false;
		uint32_t nativeWidth = 640, nativeHeight = 400;
		//uint8_t prestretch[1024][1024];
		uint32_t *scalemap = nullptr;

		uint32_t cursorX = 0, cursorY = 0;
		bool cursorVisible = true;

		static constexpr unsigned MaxColumns = 80;
		static constexpr unsigned MaxRows = 25;
		uint8_t textModeDirtyFlag[MaxColumns * MaxRows];

		uint64_t totalframes = 0;
		char windowtitle[128];
		FrameBufferInterface* fb;

		static Mutex screenMutex;

		VM& vm;
	};

	class RenderTask : public Task
	{
	public:
		RenderTask(Renderer& inRenderer) : renderer(inRenderer), vm(inRenderer.vm) {}

		void begin() override;
		int update() override;

	private:
		uint32_t cursorprevtick, cursorcurtick;

		Renderer& renderer;
		VM& vm;
	};

}

// TODO
//void setwindowtitle(const char *extra);


