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
#include <circle/bcmframebuffer.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/devicenameservice.h>
#include <circle/string.h>
#include "CircleHostInterface.h"
#include "Keymap.h"
#include "VM.h"
#include "kernel.h"

using namespace Faux86;

#define FAKE_FRAMEBUFFER (!USE_BCM_FRAMEBUFFER)

CircleHostInterface* CircleHostInterface::instance;

#if FAKE_FRAMEBUFFER
uint8_t* fakeFrameBuffer = nullptr;
#endif

void Faux86::log(Faux86::LogChannel channel, const char* message, ...)
{
#if USE_SERIAL_LOGGING
	va_list myargs;
	va_start(myargs, message);
	
	//if(channel == Faux86::LogChannel::LogRaw)
	//{
	//	CString Message;
	//	Message.Format (message, myargs);
	//	CLogger::Get()->GetTarget()->Write ((const char *) Message, Message.GetLength ());
	//}
	//else
	{
		CLogger::Get()->WriteV("Faux86", LogNotice, message, myargs);
	}
	va_end(myargs);
#endif
}

void CircleFrameBufferInterface::init(uint32_t desiredWidth, uint32_t desiredHeight)
{
#if !FAKE_FRAMEBUFFER
	frameBuffer = new CBcmFrameBuffer (desiredWidth, desiredHeight, 8);

	frameBuffer->Initialize();
#else
	log(LogVerbose, "Creating temporary buffer");
	fakeFrameBuffer = new uint8_t[640 * 400];
	log(LogVerbose, "Created at %x", (uint32_t)fakeFrameBuffer);
	
	for(int n = 0; n < 640 * 400; n++)
	{
		fakeFrameBuffer[n] = 0xcd;
	}
	log(LogVerbose, "Cleared!");
#endif
}

uint32_t CircleFrameBufferInterface::getWidth()
{ 
#if FAKE_FRAMEBUFFER
	return 640;
#else
	return frameBuffer->GetWidth();
#endif
}

uint32_t CircleFrameBufferInterface::getHeight()
{
#if FAKE_FRAMEBUFFER
	return 400;
#else
	return frameBuffer->GetHeight();
#endif
}

uint32_t CircleFrameBufferInterface::getPitch()
{
#if FAKE_FRAMEBUFFER
	return 640;
#else
	return frameBuffer->GetPitch();
#endif
}

uint8_t* CircleFrameBufferInterface::getPixels()
{
#if FAKE_FRAMEBUFFER
	return fakeFrameBuffer;
#else
	return (uint8_t*) frameBuffer->GetBuffer();
#endif
}

void CircleFrameBufferInterface::setPalette(Palette* palette)
{
#if !FAKE_FRAMEBUFFER
	for(int n = 0; n < 256; n++)
	{
		uint32_t colour = (0xff << 24) | (palette->colours[n].r) | (palette->colours[n].g << 8) | (palette->colours[n].b << 16);
		frameBuffer->SetPalette32 (n, colour);
	}
	frameBuffer->UpdatePalette();
#endif
}

void CircleAudioInterface::init(VM& vm)
{
	// TODO
}

void CircleAudioInterface::shutdown()
{
	// TODO
}

CircleTimerInterface::CircleTimerInterface()
{
	lastTimerSample = CTimer::GetClockTicks();
}

uint64_t CircleTimerInterface::getHostFreq()
{
	return CLOCKHZ;
}

uint64_t CircleTimerInterface::getTicks()
{
	uint32_t timerSample = CTimer::GetClockTicks();
	uint32_t delta = timerSample >= lastTimerSample ? (timerSample - lastTimerSample) : (0xffffffff - lastTimerSample) + timerSample;
	lastTimerSample = timerSample;
	currentTick += delta;
	return currentTick;
}

DiskInterface* CircleHostInterface::openFile(const char* filename)
{
	// TODO
	return nullptr;
} 

CircleHostInterface::CircleHostInterface(class CDeviceNameService& deviceNameService)
{
	instance = this;
	
	keyboard = (CUSBKeyboardDevice *) deviceNameService.GetDevice ("ukbd1", FALSE);
	
	if(keyboard)
	{
		keyboard->RegisterKeyStatusHandlerRaw (keyStatusHandlerRaw);
	}
}

void CircleHostInterface::tick(VM& vm)
{
	while(inputBufferSize > 0)
	{
		if(inputBuffer[inputBufferPos].eventType == EventType::KeyPress)
		{
			vm.input.handleKeyDown(inputBuffer[inputBufferPos].scancode);
		}
		else
		{
			vm.input.handleKeyUp(inputBuffer[inputBufferPos].scancode);
		}
		inputBufferPos++;
		inputBufferSize --;
		if(inputBufferPos >= MaxInputBufferSize)
		{
			inputBufferPos = 0;
		}
	}
}

void CircleHostInterface::queueEvent(EventType eventType, u16 scancode)
{
	if(inputBufferSize < MaxInputBufferSize && scancode != 0)
	{
		int writePos = (inputBufferPos + inputBufferSize) % MaxInputBufferSize;
		inputBuffer[writePos].eventType = eventType;
		inputBuffer[writePos].scancode = scancode;
		instance->inputBufferSize ++;
	}
}

void CircleHostInterface::keyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
	for(int n = 0; n < 8; n++)
	{
		int mask = 1 << n;
		bool wasPressed = (instance->lastModifiers & mask) != 0;
		bool isPressed = (ucModifiers & mask) != 0;
		if(!wasPressed && isPressed)
		{
			instance->queueEvent(EventType::KeyPress, modifier2xtMapping[n]);
		}
		else if(wasPressed && !isPressed)
		{
			instance->queueEvent(EventType::KeyRelease, modifier2xtMapping[n]);
		}
	}
		
	for(int n = 0; n < 6; n++)
	{
		if(instance->lastRawKeys[n] != 0)
		{
			bool inNewBuffer = false;
			
			for(int i = 0; i < 6; i++)
			{
				if(instance->lastRawKeys[n] == RawKeys[i])
				{
					inNewBuffer = true;
					break;
				}
			}
			
			if(!inNewBuffer && instance->inputBufferSize < MaxInputBufferSize)
			{
				instance->queueEvent(EventType::KeyRelease, usb2xtMapping[instance->lastRawKeys[n]]);
			}
		}
	}

	for(int n = 0; n < 6; n++)
	{
		if(RawKeys[n] != 0)
		{
			bool inLastBuffer = false;
			
			for(int i = 0; i < 6; i++)
			{
				if(instance->lastRawKeys[i] == RawKeys[n])
				{
					inLastBuffer = true;
					break;
				}
			}
			
			if(!inLastBuffer && instance->inputBufferSize < MaxInputBufferSize)
			{
				instance->queueEvent(EventType::KeyPress, usb2xtMapping[RawKeys[n]]);
			}
		}
	}

	for(int n = 0; n < 6; n++)
	{
		instance->lastRawKeys[n] = RawKeys[n];
	}

	instance->lastModifiers = ucModifiers;
}
