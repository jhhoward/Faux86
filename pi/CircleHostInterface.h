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

#include "HostSystemInterface.h"

namespace Faux86
{
	class CircleFrameBufferInterface : public FrameBufferInterface
	{
	public:
		virtual void init(uint32_t desiredWidth, uint32_t desiredHeight) override;
		virtual uint32_t getWidth() override;
		virtual uint32_t getHeight() override;
		virtual uint32_t getPitch() override;
		virtual uint8_t* getPixels() override;
		
		virtual void setPalette(Palette* palette) override;

		virtual bool lock() override { return true; }
		virtual void unlock() override {}

	private:
		class CBcmFrameBuffer* frameBuffer = nullptr;
	};

	class CircleAudioInterface : public AudioInterface
	{
	public:
		virtual void init(VM& vm) override;
		virtual void shutdown() override;

	private:
	};
	
	class CircleTimerInterface : public TimerInterface
	{
	public:
		CircleTimerInterface();
		virtual uint64_t getHostFreq() override;
		virtual uint64_t getTicks() override;

	private:		
		uint64_t currentTick = 0;
		uint64_t lastTimerSample = 0;
	};
	
	class CircleHostInterface : public HostSystemInterface
	{
	public:
		CircleHostInterface(class CDeviceNameService& deviceNameService);
		
		void tick(VM& vm);
		
		virtual FrameBufferInterface& getFrameBuffer() override { return frameBuffer; }
		virtual TimerInterface& getTimer() override { return timer; }
		virtual AudioInterface& getAudio() override { return audio; }
		virtual DiskInterface* openFile(const char* filename) override;

	private:
		enum class EventType
		{
			KeyPress,
			KeyRelease
		};

		struct InputEvent
		{
			EventType eventType;
			u16 scancode;
		};
	
		static void keyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]);
		void queueEvent(EventType eventType, u16 scancode);

		CircleFrameBufferInterface frameBuffer;
		CircleAudioInterface audio;
		CircleTimerInterface timer;

		static CircleHostInterface* instance;
		
		class CUSBKeyboardDevice* keyboard; 
		
		static constexpr int MaxInputBufferSize = 16;
		
		uint8_t lastModifiers;
		uint8_t lastRawKeys[6];
		InputEvent inputBuffer[MaxInputBufferSize];
		int inputBufferPos = 0;
		int inputBufferSize = 0;

	};
}
