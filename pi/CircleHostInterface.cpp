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
#include <circle/bcmframebuffer.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/input/mouse.h>
#include <circle/devicenameservice.h>
#include <circle/string.h>
#include <circle/interrupt.h>
#include "CircleHostInterface.h"
#include "FatFsDisk.h"
#include "Keymap.h" 
#include "VM.h"
#include "kernel.h"
#include "PWMSound.h"
#include "VCHIQSound.h"

using namespace Faux86;

#define FAKE_FRAMEBUFFER (!USE_BCM_FRAMEBUFFER)

CircleHostInterface* CircleHostInterface::instance;

#if FAKE_FRAMEBUFFER
uint8_t* fakeFrameBuffer = nullptr;
#endif

void Faux86::log(Faux86::LogChannel channel, const char* message, ...)
{
#if (USE_SERIAL_LOGGING || FAKE_FRAMEBUFFER)
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

	surface = new RenderSurface();
	surface->width = frameBuffer->GetWidth();
	surface->height = frameBuffer->GetHeight();
	surface->pitch = frameBuffer->GetPitch();
	surface->pixels = (uint8_t*) frameBuffer->GetBuffer();
#else
	log(LogVerbose, "Creating temporary buffer");
	surface = RenderSurface::create(640, 400);
	log(LogVerbose, "Created at %x", (uint32_t)surface->pixels);
	
	for(int n = 0; n < 640 * 400; n++)
	{
		surface->pixels[n] = 0xcd;
	}
	log(LogVerbose, "Cleared!");
#endif
}

void CircleFrameBufferInterface::resize(uint32_t desiredWidth, uint32_t desiredHeight)
{
	if(surface->width == desiredWidth && surface->height == desiredHeight)
		return;
#if !FAKE_FRAMEBUFFER
	delete frameBuffer;
	
	CTimer::Get()->SimpleMsDelay(1000);
	
	frameBuffer = new CBcmFrameBuffer (desiredWidth, desiredHeight, 8);
	frameBuffer->Initialize();
	
	surface->width = frameBuffer->GetWidth();
	surface->height = frameBuffer->GetHeight();
	surface->pitch = frameBuffer->GetPitch();
	surface->pixels = (uint8_t*) frameBuffer->GetBuffer();
#endif
}

RenderSurface* CircleFrameBufferInterface::getSurface()
{ 
	return surface;
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
	//pwmSound = new PWMSound(vm.audio, &interruptSystem, vm.audio.sampleRate);
	//pwmSound->Start();
#if USE_VCHIQ_SOUND
	vchiqSound = new VCHIQSound(vm.audio, &vchiqDevice, vm.audio.sampleRate);
	vchiqSound->Start();
#endif
}

void CircleAudioInterface::shutdown()
{
	if(pwmSound)
	{
		pwmSound->Cancel();
		delete pwmSound;
	}
	if(vchiqSound)
	{
		vchiqSound->Cancel();
		delete vchiqSound;
	}
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
	return FatFsDisk::open(filename);
} 

CircleHostInterface::CircleHostInterface(CDeviceNameService& deviceNameService, CInterruptSystem& interruptSystem, CVCHIQDevice& inVchiqDevice)
: audio(interruptSystem, inVchiqDevice)
{
	instance = this;
	
	keyboard = (CUSBKeyboardDevice *) deviceNameService.GetDevice ("ukbd1", FALSE);
	
	if(keyboard)
	{
		keyboard->RegisterKeyStatusHandlerRaw (keyStatusHandlerRaw);
	}
	
	CMouseDevice* mouse = (CMouseDevice *) deviceNameService.GetDevice ("mouse1", FALSE);
	
	if(mouse)
	{
		mouse->RegisterStatusHandler(mouseStatusHandler);
	}
}

void CircleHostInterface::tick(VM& vm)
{
	while(inputBufferSize > 0)
	{
		InputEvent& event = inputBuffer[inputBufferPos];
		
		switch(event.eventType)
		{
			case EventType::KeyPress:
			{
				vm.input.handleKeyDown(event.scancode);
			}
			break;
			case EventType::KeyRelease:
			{
				vm.input.handleKeyUp(event.scancode);
			}
			break;
			case EventType::MousePress:
			{
				vm.mouse.handleButtonDown(event.mouseButton);
			}
			break;
			case EventType::MouseRelease:
			{
				vm.mouse.handleButtonUp(event.mouseButton);
			}
			break;
			case EventType::MouseMove:
			{
				vm.mouse.handleMove(event.mouseMotionX, event.mouseMotionY);
			}
			break;
		}
		
		inputBufferPos++;
		inputBufferSize --;
		if(inputBufferPos >= MaxInputBufferSize)
		{
			inputBufferPos = 0;
		}
	}
}

void CircleHostInterface::queueEvent(InputEvent& inEvent)
{
	if(inputBufferSize < MaxInputBufferSize)
	{
		int writePos = (inputBufferPos + inputBufferSize) % MaxInputBufferSize;
		inputBuffer[writePos] = inEvent;
		instance->inputBufferSize ++;
	}
}

void CircleHostInterface::queueEvent(EventType eventType, u16 scancode)
{
	if(scancode != 0)
	{
		InputEvent newEvent;
		newEvent.eventType = eventType;
		newEvent.scancode = scancode;
		queueEvent(newEvent);
	}
}

void CircleHostInterface::mouseStatusHandler (unsigned nButtons, int nDisplacementX, int nDisplacementY)
{
	InputEvent newEvent;
	
	// Mouse presses
	if((nButtons & MOUSE_BUTTON_LEFT) && !(instance->lastMouseButtons & MOUSE_BUTTON_LEFT))
	{
		newEvent.eventType = EventType::MousePress;
		newEvent.mouseButton = SerialMouse::ButtonType::Left;
		instance->queueEvent(newEvent);
	}
	if((nButtons & MOUSE_BUTTON_RIGHT) && !(instance->lastMouseButtons & MOUSE_BUTTON_RIGHT))
	{
		newEvent.eventType = EventType::MousePress;
		newEvent.mouseButton = SerialMouse::ButtonType::Right;
		instance->queueEvent(newEvent);
	}
	
	// Mouse releases
	if(!(nButtons & MOUSE_BUTTON_LEFT) && (instance->lastMouseButtons & MOUSE_BUTTON_LEFT))
	{
		newEvent.eventType = EventType::MouseRelease;
		newEvent.mouseButton = SerialMouse::ButtonType::Left;
		instance->queueEvent(newEvent);
	}
	if(!(nButtons & MOUSE_BUTTON_RIGHT) && (instance->lastMouseButtons & MOUSE_BUTTON_RIGHT))
	{
		newEvent.eventType = EventType::MouseRelease;
		newEvent.mouseButton = SerialMouse::ButtonType::Right;
		instance->queueEvent(newEvent);
	}

	// Motion events	
	if(nDisplacementX != 0 || nDisplacementY != 0)
	{
		newEvent.eventType = EventType::MouseMove;
		newEvent.mouseMotionX = (s8) nDisplacementX;
		newEvent.mouseMotionY = (s8) nDisplacementY;
		instance->queueEvent(newEvent);
	}
	
	instance->lastMouseButtons = nButtons;
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
