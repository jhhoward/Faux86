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
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif
#include <SDL.h>
#include "SDLInterface.h"
#include "StdioDiskInterface.h"
#include "../../src/faux86/VM.h"
#include "../../pi/Keymap.h"

using namespace Faux86;

SDLHostSystemInterface::SDLHostSystemInterface()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
}

void SDLAudioInterface::init(VM& vm)
{
	SDL_AudioSpec wanted;

	log(Log, "Initializing audio stream... ");

	wanted.freq = vm.config.audio.sampleRate;
	wanted.format = AUDIO_U8;
	wanted.channels = 1;
	wanted.samples = (uint16_t)((vm.config.audio.sampleRate / 1000) * vm.config.audio.latency) >> 1;
	wanted.callback = fillAudioBuffer;
	wanted.userdata = &vm;

	if (SDL_OpenAudio(&wanted, NULL) <0) {
		log(Log, "Error: %s\n", SDL_GetError());
	}
	else {
		log(Log, "OK! (%lu Hz, %lu ms, %lu sample latency)\n", vm.config.audio.sampleRate, vm.config.audio.latency, wanted.samples);
	}

	SDL_PauseAudio(0);
}

void SDLAudioInterface::shutdown()
{
	SDL_PauseAudio(1);
}

void SDLAudioInterface::fillAudioBuffer(void *udata, uint8_t *stream, int len)
{
	VM* vm = (VM*)(udata);
	vm->audio.fillAudioBuffer(stream, len);
}

SDLHostSystemInterface::~SDLHostSystemInterface()
{
	SDL_Quit();
}

DiskInterface* SDLHostSystemInterface::openFile(const char* filename)
{
	return new StdioDiskInterface(filename);
}


void SDLFrameBufferInterface::init(uint32_t desiredWidth, uint32_t desiredHeight)
{
	desiredWidth = 800;
	desiredHeight = 600;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	SDL_CreateWindowAndRenderer(desiredWidth, desiredHeight, SDL_WINDOW_RESIZABLE, &appWindow, &appRenderer);
	SDL_SetWindowTitle(appWindow, "Faux86");
	SDL_RenderSetLogicalSize(appRenderer, desiredWidth, desiredHeight);

	surface = SDL_CreateRGBSurface(0, desiredWidth, desiredHeight, 8, 0, 0, 0, 0);
	screenSurface = SDL_CreateRGBSurface(0, desiredWidth, desiredHeight, 32, 0, 0, 0, 0);
	screenTexture = SDL_CreateTexture(appRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, screenSurface->w, screenSurface->h);

	renderSurface.width = surface->w;
	renderSurface.pitch = surface->pitch;
	renderSurface.height = surface->h;
	renderSurface.pixels = (uint8_t*) surface->pixels;
}

void SDLFrameBufferInterface::resize(uint32_t desiredWidth, uint32_t desiredHeight)
{
	if (renderSurface.width == desiredWidth && renderSurface.height == desiredHeight)
		return;

	SDL_FreeSurface(surface);
	SDL_FreeSurface(screenSurface);
	SDL_DestroyTexture(screenTexture);

	surface = SDL_CreateRGBSurface(0, desiredWidth, desiredHeight, 8, 0, 0, 0, 0);
	screenSurface = SDL_CreateRGBSurface(0, desiredWidth, desiredHeight, 32, 0, 0, 0, 0);
	screenTexture = SDL_CreateTexture(appRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, screenSurface->w, screenSurface->h);

	renderSurface.width = surface->w;
	renderSurface.pitch = surface->pitch;
	renderSurface.height = surface->h;
	renderSurface.pixels = (uint8_t*)surface->pixels;
}

RenderSurface* SDLFrameBufferInterface::getSurface()
{
	return &renderSurface;
}

void SDLFrameBufferInterface::setPalette(Palette* palette)
{
	SDL_Color colours[256];

	for (int n = 0; n < 256; n++)
	{
		colours[n].r = palette->colours[n].r;
		colours[n].g = palette->colours[n].g;
		colours[n].b = palette->colours[n].b;
	}
	SDL_SetPaletteColors(surface->format->palette, colours, 0, 256);
}

void SDLFrameBufferInterface::present() 
{
	SDL_BlitSurface(surface, nullptr, screenSurface, nullptr);

	void* pixels;
	int pitch;
	SDL_LockTexture(screenTexture, nullptr, &pixels, &pitch);
	SDL_ConvertPixels(screenSurface->w, screenSurface->h, screenSurface->format->format, screenSurface->pixels, screenSurface->pitch, SDL_PIXELFORMAT_RGBA8888, pixels, pitch);
	SDL_UnlockTexture(screenTexture);

	SDL_RenderCopy(appRenderer, screenTexture, nullptr, nullptr);
	SDL_RenderPresent(appRenderer);
}

uint64_t SDLTimerInterface::getHostFreq() 
{
#ifdef _WIN32
	LARGE_INTEGER queryperf;
	QueryPerformanceFrequency(&queryperf);
	return queryperf.QuadPart;
#else
	return 1000000;
#endif
}

uint64_t SDLTimerInterface::getTicks() 
{
#ifdef _WIN32
	LARGE_INTEGER queryperf;
	QueryPerformanceCounter(&queryperf);
	return queryperf.QuadPart;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)tv.tv_sec * (uint64_t)1000000 + (uint64_t)tv.tv_usec;
#endif
}

uint8_t SDLHostSystemInterface::translatescancode(uint16_t keyval)
{
	switch (keyval) {
	case 0x1B:
		return (1);
		break; //Esc
	case 0x30:
		return (0xB);
		break; //zero
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
		return (keyval - 0x2F);
		break; //other number keys
	case 0x2D:
		return (0xC);
		break; //-_
	case 0x3D:
		return (0xD);
		break; //=+
	case 0x8:
		return (0xE);
		break; //backspace
	case 0x9:
		return (0xF);
		break; //tab
	case 0x71:
		return (0x10);
		break;
	case 0x77:
		return (0x11);
		break;
	case 0x65:
		return (0x12);
		break;
	case 0x72:
		return (0x13);
		break;
	case 0x74:
		return (0x14);
		break;
	case 0x79:
		return (0x15);
		break;
	case 0x75:
		return (0x16);
		break;
	case 0x69:
		return (0x17);
		break;
	case 0x6F:
		return (0x18);
		break;
	case 0x70:
		return (0x19);
		break;
	case 0x5B:
		return (0x1A);
		break;
	case 0x5D:
		return (0x1B);
		break;
	case 0xD:
	case 0x10F:
		return (0x1C);
		break; //enter
	case 0x131:
	case 0x132:
		return (0x1D);
		break; //ctrl
	case 0x61:
		return (0x1E);
		break;
	case 0x73:
		return (0x1F);
		break;
	case 0x64:
		return (0x20);
		break;
	case 0x66:
		return (0x21);
		break;
	case 0x67:
		return (0x22);
		break;
	case 0x68:
		return (0x23);
		break;
	case 0x6A:
		return (0x24);
		break;
	case 0x6B:
		return (0x25);
		break;
	case 0x6C:
		return (0x26);
		break;
	case 0x3B:
		return (0x27);
		break;
	case 0x27:
		return (0x28);
		break;
	case 0x60:
		return (0x29);
		break;
	case 0x130:
		return (0x2A);
		break; //left shift
	case 0x5C:
		return (0x2B);
		break;
	case 0x7A:
		return (0x2C);
		break;
	case 0x78:
		return (0x2D);
		break;
	case 0x63:
		return (0x2E);
		break;
	case 0x76:
		return (0x2F);
		break;
	case 0x62:
		return (0x30);
		break;
	case 0x6E:
		return (0x31);
		break;
	case 0x6D:
		return (0x32);
		break;
	case 0x2C:
		return (0x33);
		break;
	case 0x2E:
		return (0x34);
		break;
	case 0x2F:
		return (0x35);
		break;
	case 0x12F:
		return (0x36);
		break; //right shift
	case 0x13C:
		return (0x37);
		break; //print screen
	case 0x133:
	case 0x134:
		return (0x38);
		break; //alt
	case 0x20:
		return (0x39);
		break; //space
	case 0x12D:
		return (0x3A);
		break; //caps lock
	case 0x11A:
	case 0x11B:
	case 0x11C:
	case 0x11D:
	case 0x11E:
	case 0x11F:
	case 0x120:
	case 0x121:
	case 0x122:
	case 0x123:
		return (keyval - 0x11A + 0x3B);
		break; //F1 to F10
	case 0x12C:
		return (0x45);
		break; //num lock
	case 0x12E:
		return (0x46);
		break; //scroll lock
	case 0x116:
	case 0x107:
		return (0x47);
		break; //home
	case 0x111:
	case 0x108:
		return (0x48);
		break; //up
	case 0x118:
	case 0x109:
		return (0x49);
		break; //pgup
	case 0x10D:
		return (0x4A);
		break; //keypad -
	case 0x114:
	case 0x104:
		return (0x4B);
		break; //left
	case 0x105:
		return (0x4C);
		break; //center
	case 0x113:
	case 0x106:
		return (0x4D);
		break; //right
	case 0x10E:
		return (0x4E);
		break; //keypad +
	case 0x117:
	case 0x101:
		return (0x4F);
		break; //end
	case 0x112:
	case 0x102:
		return (0x50);
		break; //down
	case 0x119:
	case 0x103:
		return (0x51);
		break; //pgdn
	case 0x115:
	case 0x100:
		return (0x52);
		break; //ins
	case 0x7F:
	case 0x10A:
		return (0x53);
		break; //del
	default:
		return (0);
	}
}

void SDLHostSystemInterface::tick(VM& vm)
{
	SDL_Event event;
	int mx = 0, my = 0;

	if (SDL_PollEvent(&event)) 
	{
		switch (event.type) {
		case SDL_KEYDOWN:
			vm.input.handleKeyDown(usb2xtMapping[event.key.keysym.scancode]);

			if (event.key.keysym.sym == SDLK_TAB)
			{
				SDL_SetWindowGrab(frameBufferInterface.appWindow, SDL_FALSE);
				SDL_ShowCursor(SDL_ENABLE);
			}
			break;
		case SDL_KEYUP:
			vm.input.handleKeyUp(usb2xtMapping[event.key.keysym.scancode]);
			break;
		case SDL_MOUSEBUTTONDOWN:
			SDL_SetWindowGrab(frameBufferInterface.appWindow, SDL_TRUE);
			SDL_ShowCursor(SDL_DISABLE);
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				vm.mouse.handleButtonDown(SerialMouse::ButtonType::Left);
			}
			else if(event.button.button == SDL_BUTTON_RIGHT)
			{
				vm.mouse.handleButtonDown(SerialMouse::ButtonType::Right);
			}
			// TODO grab mouse
			break;
		case SDL_MOUSEBUTTONUP:
			if (SDL_GetWindowGrab(frameBufferInterface.appWindow) == SDL_FALSE) break;
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				vm.mouse.handleButtonUp(SerialMouse::ButtonType::Left);
			}
			else if (event.button.button == SDL_BUTTON_RIGHT)
			{
				vm.mouse.handleButtonUp(SerialMouse::ButtonType::Right);
			}
			break;
		case SDL_MOUSEMOTION:
			if (SDL_GetWindowGrab(frameBufferInterface.appWindow) == SDL_FALSE) break;
			SDL_GetRelativeMouseState(&mx, &my);
			vm.mouse.handleMove((int8_t)mx, (int8_t)my);
			//SDL_WarpMouse (frameBufferInterface.getWidth() / 2, frameBufferInterface.getHeight() / 2);
			break;
		case SDL_QUIT:
			vm.running = false;
			break;
		default:
			break;
		}
	}

}

void Faux86::log(Faux86::LogChannel channel, const char* message, ...)
{
	const bool enableLogRaw = false;

	if (channel == LogRaw && !enableLogRaw)
		return;

	va_list myargs;
	va_start(myargs, message);
	vprintf(message, myargs);
	va_end(myargs);

	if (channel != LogRaw)
	{
		printf("\n");
	}
}
