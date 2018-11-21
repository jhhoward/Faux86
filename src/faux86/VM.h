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

#include "Config.h"
#include "CPU.h"
#include "RAM.h"
#include "PIT.h"
#include "Ports.h"
#include "PIC.h"
#include "DMA.h"
#include "SerialMouse.h"
#include "Audio.h"
#include "SoundBlaster.h"
#include "Adlib.h"
#include "DisneySoundSource.h"
#include "PCSpeaker.h"
#include "DriveManager.h"
#include "Video.h"
#include "Renderer.h"
#include "InputManager.h"
#include "Timing.h"
#include "TaskManager.h"

namespace Faux86
{
	class Debugger;

	class VM
	{
	public:
		VM(Config& inConfig);
		~VM();

		bool init();
		bool simulate();

		Config config;

		CPU cpu;
		Memory memory;
		Ports ports;
		PIC pic;
		PIT pit;
		DMA dma;
		DriveManager drives;

		Video video;
		Audio audio;

		Adlib adlib;
		SoundBlaster blaster;
		DisneySoundSource soundSource;
		PCSpeaker pcSpeaker;

		SerialMouse mouse;

		Renderer renderer;
		InputManager input;
		TimingScheduler timing;
		TaskManager taskManager;

		Debugger* debugger = nullptr;

		bool running;

	private:

//		bool running;
	};
}

// TEMPORARY
//extern Faux86::VM vm;