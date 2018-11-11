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

#include "VM.h"
#include "DriveManager.h"

using namespace Faux86;

extern uint8_t renderbenchmark;

extern void VideoThread();

#ifdef CPU_ADDR_MODE_CACHE
extern uint64_t cached_access_count, uncached_access_count;
#endif

uint64_t starttick, endtick;

uint8_t cgaonly = 0, useconsole = 0;

extern void isa_ne2000_init (uint16_t baseport, uint8_t irq);

#ifdef NETWORKING_ENABLED
extern void initpcap();
extern void dispatch();
#endif

void inithardware() {
#ifdef NETWORKING_ENABLED
	if (ethif != 254) initpcap();
#endif
#ifndef NETWORKING_OLDCARD
	printf ("  - Novell NE2000 ethernet adapter: ");
	isa_ne2000_init (0x300, 6);
	printf ("OK\n");
#endif
}

class MainEmulationTask : public Task
{
	VM& vm;

public:
	MainEmulationTask(VM& inVM) : vm(inVM) {}

	int update() override
	{
		int delay = 0;

		//log(LogVerbose, ".");

		if (!vm.config.speed)
		{
			vm.cpu.exec86(10000);
		}
		else 
		{
			vm.cpu.exec86(vm.config.speed / 100);
			while (!vm.audio.isAudioBufferFilled()) 
			{
				vm.timing.tick();
				vm.audio.tick();
			}
			delay = 10;
		}

		return delay;
	}
};

#ifdef _WIN32
void runconsole (void *dummy);
#else
void *runconsole (void *dummy);
#endif

VM::VM(Config& inConfig)
	: config(inConfig)
	, cpu(*this)
	, memory(*this)
	, ports(*this)
	, pic(*this)
	, pit(*this)
	, dma(*this)
	, drives(*this)
	, video(*this)
	, audio(*this)
	, adlib(*this)
	, blaster(*this, adlib)
	, soundSource(*this)
	, pcSpeaker(*this)
	, mouse(*this)
	, renderer(*this)
	, input(*this)
	, timing(*this)
	, taskManager(*this)
	, running(true)
{
}

bool VM::init()
{
	log(Log, "%s (c)2010-2013 Mike Chambers\n", BUILD_STRING);
	log(Log, "[A portable, open-source 8086 PC emulator]\n\n");

	renderer.init();
	audio.init();

	adlib.init();
	soundSource.init();
	blaster.init();
	dma.init();

	mouse.init();

	timing.init();

	if (!config.biosFile || !config.biosFile->isValid())
	{
		log(LogFatal, "Could not load BIOS file!");
		return false;
	}

	if (!config.asciiFile || !config.asciiFile->isValid())
	{
		log(LogFatal, "Could not load ASCII file!");
		return false;
	}

	uint32_t biosSize = (uint32_t) config.biosFile->getSize();
	memory.loadBinary((uint32_t)(DEFAULT_RAM_SIZE - biosSize), config.biosFile, 1);
	
	//memory.loadBinary(0xA0000UL, config.asciiFile, 1);

#ifdef DISK_CONTROLLER_ATA
	if (!memory.loadBinary(0xD0000UL, config.ideControllerFile, 1))
	{
		return false;
	}
#endif
	if (biosSize <= 8192) 
	{
		memory.loadBinary(0xF6000UL, config.romBasicFile, 0);
		if (!memory.loadBinary(0xC0000UL, config.videoRomFile, 1))
		{
			log(LogFatal, "Could not load video rom file!");
			return false;
		}
	}

	drives.insertDisk(DRIVE_A, config.diskDriveA);
	drives.insertDisk(DRIVE_B, config.diskDriveB);
	drives.insertDisk(DRIVE_C, config.diskDriveC);
	drives.insertDisk(DRIVE_D, config.diskDriveD);

	if (config.bootDrive == 254)
	{
		if (drives.isDiskInserted(DRIVE_C))
			config.bootDrive = DRIVE_C;
		else if (drives.isDiskInserted(DRIVE_A))
			config.bootDrive = DRIVE_A;
		else
			config.bootDrive = 0xFF; //ROM BASIC fallback
	}

	log(Log, "\nInitializing CPU... ");
	running = true;
	cpu.reset86();
	log(Log, "OK!\n");

	inithardware();

#ifdef WITH_DEBUG_CONSOLE
	if (useconsole) {
#ifdef _WIN32
		_beginthread(runconsole, 0, (void*) this);
#else
		pthread_create(&consolethread, NULL, (void *)runconsole, (void*) this);
#endif
	}
#endif

	taskManager.addTask(new MainEmulationTask(*this));
	
	return true;
}

VM::~VM()
{
}

bool VM::simulate()
{
	input.tick();

#ifdef NETWORKING_ENABLED
	if (ethif < 254) dispatch();
#endif

	taskManager.tick();

	return running;
}

