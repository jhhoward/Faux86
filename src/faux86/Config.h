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
#pragma once

#define BUILD_STRING "Faux86"

//be sure to only define ONE of the CPU_* options at any given time, or
//you will likely get some unexpected/bad results!
//#define CPU_8086
//#define CPU_186
#define CPU_V20
//#define CPU_286

#if defined(CPU_8086)
	#define CPU_CLEAR_ZF_ON_MUL
	#define CPU_ALLOW_POP_CS
#else
	#define CPU_ALLOW_ILLEGAL_OP_EXCEPTION
	#define CPU_LIMIT_SHIFT_COUNT
#endif

#if defined(CPU_V20)
	#define CPU_NO_SALC
#endif

#if defined(CPU_286) || defined(CPU_386)
	#define CPU_286_STYLE_PUSH_SP
#else
	#define CPU_SET_HIGH_FLAGS
#endif

#define TIMING_INTERVAL 15

//when USE_PREFETCH_QUEUE is defined, Faux86's CPU emulator uses a 6-byte
//read-ahead cache for opcode fetches just as a real 8086/8088 does.
//by default, i just leave this disabled because it wastes a very very
//small amount of CPU power. however, for the sake of more accurate
//emulation, it can be enabled by uncommenting the line below and recompiling.
//#define USE_PREFETCH_QUEUE

//#define CPU_ADDR_MODE_CACHE

//when compiled with network support, faux86 needs libpcap/winpcap.
//if it is disabled, the ethernet card is still emulated, but no actual
//communication is possible -- as if the ethernet cable was unplugged.
#define NETWORKING_OLDCARD //planning to support an NE2000 in the future

//when DISK_CONTROLLER_ATA is defined, faux86 will emulate a true IDE/ATA1 controller
//card. if it is disabled, emulated disk access is handled by directly intercepting
//calls to interrupt 13h.
//*WARNING* - the ATA controller is not currently complete. do not use!
//#define DISK_CONTROLLER_ATA

#define AUDIO_DEFAULT_SAMPLE_RATE 48000
#define AUDIO_DEFAULT_LATENCY 100

//#define OUTPUT_FRAMEBUFFER_WIDTH 320
//#define OUTPUT_FRAMEBUFFER_HEIGHT 200
#define OUTPUT_FRAMEBUFFER_WIDTH 640
#define OUTPUT_FRAMEBUFFER_HEIGHT 400

#define DEFAULT_RAM_SIZE 0x100000
						 
//#define DEBUG_BLASTER
//#define DEBUG_DMA

//#define BENCHMARK_BIOS

#include "Types.h"
#include "HostSystemInterface.h"

namespace Faux86
{
	enum class CpuType
	{
		Cpu8086,
		Cpu186,
		CpuV20,
		Cpu286,
		Cpu386
	};

	class DiskInterface;

	struct Config
	{
		Config(HostSystemInterface* inHostInterface) : hostSystemInterface(inHostInterface) {}

		void parseCommandLine(int argc, char *argv[]);

		HostSystemInterface* hostSystemInterface;

		uint32_t ramSize = DEFAULT_RAM_SIZE;
		CpuType cpuType = CpuType::Cpu286;

		DiskInterface* biosFile = nullptr;
		DiskInterface* ideControllerFile = nullptr;
		DiskInterface* romBasicFile = nullptr;
		DiskInterface* videoRomFile = nullptr;
		DiskInterface* asciiFile = nullptr;

		DiskInterface* diskDriveA = nullptr;
		DiskInterface* diskDriveB = nullptr;
		DiskInterface* diskDriveC = nullptr;
		DiskInterface* diskDriveD = nullptr;

		uint8_t bootDrive = 0;

		struct  
		{
			uint16_t port = 0x3F8;
			uint8_t irq = 4;
		} mouse;

		struct  
		{
			uint16_t port = 0x220;
			uint8_t irq = 7;
		} blaster;

		struct  
		{
			uint16_t port = 0x388;
		} adlib; 

		bool useDisneySoundSource = false;

		struct 
		{
			int32_t sampleRate = AUDIO_DEFAULT_SAMPLE_RATE;
			int32_t latency = AUDIO_DEFAULT_LATENCY;
		} audio;
		
		bool verbose = false;
		bool useFullScreen = false;
		bool renderBenchmark = false;
		bool noSmooth = true;
		bool noScale = false;
		bool enableAudio = true;
		bool enableConsole = false;
		bool singleThreaded = true;
		bool slowSystem = false;
		bool enableDebugger = false;
		
		uint32_t speed = 0;
		uint32_t frameDelay = 20;

	};
}
