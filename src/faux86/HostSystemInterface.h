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
	enum LogChannel
	{
		LogVerbose,
		LogFatal,
		Log,
		LogRaw
	};

	// Must be implemented by host system
	void log(LogChannel channel, const char* message, ...);

	class DiskInterface;
	class VM;
	class Palette;
	struct RenderSurface;

	class FrameBufferInterface
	{
	public:
		virtual void init(uint32_t desiredWidth, uint32_t desiredHeight) = 0;
		virtual void resize(uint32_t desiredWidth, uint32_t desiredHeight) {}
		virtual RenderSurface* getSurface() = 0;

		virtual void setPalette(Palette* palette) = 0;

		virtual void present() {}
	};

	class TimerInterface
	{
	public:
		virtual uint64_t getHostFreq() = 0;
		virtual uint64_t getTicks() = 0;
	};

	class AudioInterface
	{
	public:
		virtual void init(VM& vm) = 0;
		virtual void shutdown() = 0;
	};

	class HostSystemInterface
	{
	public:
		virtual FrameBufferInterface& getFrameBuffer() = 0;
		virtual TimerInterface& getTimer() = 0;
		virtual AudioInterface& getAudio() = 0;
		virtual DiskInterface* openFile(const char* filename) { return nullptr; }
	};
}
