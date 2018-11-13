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

#include "../../src/faux86/HostSystemInterface.h"
#include "../../src/faux86/Renderer.h"

struct SDL_Surface;

namespace Faux86
{
	class VM;
	
	class SDLFrameBufferInterface : public FrameBufferInterface
	{
	public:
		virtual void init(uint32_t desiredWidth, uint32_t desiredHeight) override;

		virtual RenderSurface* getSurface() override;

		virtual void setPalette(Palette* palette) override;

		virtual bool lock() override;
		virtual void unlock() override;

	private:
		SDL_Surface* surface;
		RenderSurface renderSurface;
	};

	class SDLTimerInterface : public TimerInterface
	{
	public:
		virtual uint64_t getHostFreq() override;
		virtual uint64_t getTicks() override;
	};

	class SDLAudioInterface : public AudioInterface
	{
	public:
		virtual void init(VM& vm) override;
		virtual void shutdown() override;

	private:
		static void fillAudioBuffer(void *udata, uint8_t *stream, int len);
	};

	class SDLHostSystemInterface : public HostSystemInterface
	{
	public:
		SDLHostSystemInterface();
		virtual ~SDLHostSystemInterface();

		virtual AudioInterface& getAudio() override { return audioInterface; }
		virtual FrameBufferInterface& getFrameBuffer() override { return frameBufferInterface; }
		virtual TimerInterface& getTimer() override { return timerInterface;  }
		virtual DiskInterface* openFile(const char* filename) override;

		void tick(VM& vm);

	private:
		uint8_t translatescancode(uint16_t keyval);

		SDLAudioInterface audioInterface;
		SDLFrameBufferInterface frameBufferInterface;
		SDLTimerInterface timerInterface;
	};

};
