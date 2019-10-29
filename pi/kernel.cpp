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
#include "kernel.h"
#include <circle/string.h>
#include "CircleHostInterface.h"
#include "VM.h"
#include "CircleDeviceDisk.h"

// Embedded disks / ROMS
#if USE_EMBEDDED_BOOT_FLOPPY
#include "../data/dosboot.h"
#endif
#if USE_EMBEDDED_ROM_IMAGES
#include "../data/asciivga.h"
#include "../data/pcxtbios.h"
#include "../data/videorom.h"
#endif

#define REQUIRED_KERNEL_MEMORY (0x2000000)

#if KERNEL_MAX_SIZE < REQUIRED_KERNEL_MEMORY
//#error "Not enough kernel space: change KERNEL_MAX_SIZE in circle/sysconfig.h to at least 32 megabytes and recompile circle"
#endif

#define AUDIO_SAMPLE_RATE 44100

static const char FromKernel[] = "kernel";

using namespace Faux86;

CKernel::CKernel (void) :
//:	m_Screen (256, 256, true),//(m_Options.GetWidth (), m_Options.GetHeight (), true),
	m_ActLED(false),
#if !USE_BCM_FRAMEBUFFER
	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
#endif
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_DWHCI (&m_Interrupt, &m_Timer),
#if USE_MMC_MOUNTING
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED),
#endif
	m_VCHIQ (&m_Memory, &m_Interrupt)
{
	m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;
	
#if !USE_BCM_FRAMEBUFFER
	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}
#endif

	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
#if !USE_BCM_FRAMEBUFFER
			pTarget = &m_Screen;
#else
			pTarget = &m_Serial;
#endif
		}

		bOK = m_Logger.Initialize (pTarget);
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}

	if (bOK)
	{
		bOK = m_DWHCI.Initialize ();
	}

#if USE_MMC_MOUNTING	
	if (bOK)
	{
		bOK = m_EMMC.Initialize ();
	}
#endif

#if USE_VCHIQ_SOUND
	if (bOK)
	{
		bOK = m_VCHIQ.Initialize ();
	}
#endif

	if(bOK)
	{
		bOK = (f_mount (&m_FileSystem, "SD:", 1) == FR_OK);
	}

	if(bOK)
	{
		HostInterface = new CircleHostInterface(m_DeviceNameService, m_Interrupt, m_VCHIQ);
		vmConfig = new Config(HostInterface);
		
		vmConfig->audio.sampleRate = 44100;

		log(Log, "Attempting to load system files");
		
		vmConfig->biosFile = HostInterface->openFile("SD:/pcxtbios.bin");
		vmConfig->videoRomFile = HostInterface->openFile("SD:/videorom.bin");
		vmConfig->asciiFile = HostInterface->openFile("SD:/asciivga.dat");
		vmConfig->diskDriveA = HostInterface->openFile("SD:/dosboot.img");

#if USE_EMBEDDED_ROM_IMAGES
		if(!vmConfig->biosFile)
			vmConfig->biosFile = new EmbeddedDisk(pcxtbios, sizeof(pcxtbios));
		if(!vmConfig->videoRomFile)
			vmConfig->videoRomFile = new EmbeddedDisk(videorom, sizeof(videorom));
		if(!vmConfig->asciiFile)
			vmConfig->asciiFile = new EmbeddedDisk(asciivga, sizeof(asciivga));
#endif
#if USE_EMBEDDED_BOOT_FLOPPY
		if(!vmConfig->diskDriveA)
			vmConfig->diskDriveA = new EmbeddedDisk(dosboot, sizeof(dosboot));
#endif

#if USE_MMC_MOUNTING
		vmConfig->diskDriveC = new CircleDeviceDisk(&m_EMMC);
#endif
		
		CDevice *pUMSD1 = m_DeviceNameService.GetDevice ("umsd1", TRUE);
		if(pUMSD1)
		{
			vmConfig->diskDriveD = new CircleDeviceDisk(pUMSD1);
		}
        
		// TODO
		//vmConfig->parseCommandLine(argc, argv);
		
		log(Log, "Creating VM");
		
		vm = new VM(*vmConfig);

		log(Log, "Init VM");
		
		bOK = vm->init();
	}
	
	return bOK;
}

#define TEST_STATIC_ARRAY 0
#define TEST_DYNAMIC_ARRAY 0

#if TEST_STATIC_ARRAY
constexpr int bigArraySize = 8 * 1024 * 1024;
unsigned char bigArray[bigArraySize] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
#endif


TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

	m_Logger.Write(FromKernel, LogNotice, "Begin mem test");
	
#if TEST_DYNAMIC_ARRAY
	m_Logger.Write(FromKernel, LogNotice, "Alloc");
	unsigned long int size = 32 * 1024 * 1024;
	unsigned char* test = new unsigned char[size];
	assert(test);
	
	m_Logger.Write(FromKernel, LogNotice, "Write mem");
	for(unsigned long n = 0; n < size; n++)
	{
		test[n] = 0xFE;
	}

	m_Logger.Write(FromKernel, LogNotice, "Read mem");
	for(unsigned long n = 0; n < size; n++)
	{
		assert(test[n] == 0xFE);
	}

	m_Logger.Write(FromKernel, LogNotice, "Free");
	delete[] test;
#endif

#if TEST_STATIC_ARRAY
	m_Logger.Write(FromKernel, LogNotice, "Write static mem");
	for(unsigned long n = 0; n < bigArraySize; n++)
	{
		bigArray[n] = 0xFE;
	}

	m_Logger.Write(FromKernel, LogNotice, "Read static mem");
	for(unsigned long n = 0; n < bigArraySize; n++)
	{
		assert(bigArray[n] == 0xFE);
	}
#endif
	m_Logger.Write(FromKernel, LogNotice, "End mem test");

#if !USE_BCM_FRAMEBUFFER
	unsigned int nCount = 0;
#endif
	
	while (vm->simulate())
	{
		HostInterface->tick(*vm);
		
#if !USE_BCM_FRAMEBUFFER
		m_Screen.Rotor (0, nCount++);
#endif
		m_Scheduler.Yield ();
		//m_Timer.MsDelay (0);
	}
	
	return ShutdownHalt;
}

