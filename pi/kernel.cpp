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
#include "kernel.h"
#include <circle/string.h>
#include "CircleHostInterface.h"
#include "VM.h"
#include "MMCDisk.h"

// Embedded disks / ROMS
#if USE_EMBEDDED_BOOT_FLOPPY
#include "../data/dosboot.h"
#endif
#include "../data/asciivga.h"
#include "../data/pcxtbios.h"
#include "../data/videorom.h"

#define REQUIRED_KERNEL_MEMORY (0x2000000)

#if KERNEL_MAX_SIZE < REQUIRED_KERNEL_MEMORY
//#error "Not enough kernel space: change KERNEL_MAX_SIZE in circle/sysconfig.h to at least 32 megabytes and recompile circle"
#endif

static const char FromKernel[] = "kernel";

using namespace Faux86;

CKernel::CKernel (void) :
//:	m_Screen (256, 256, true),//(m_Options.GetWidth (), m_Options.GetHeight (), true),
#if !USE_BCM_FRAMEBUFFER
	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
#endif
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_DWHCI (&m_Interrupt, &m_Timer)
	
#if USE_MMC_MOUNTING
	,m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED)
#endif
{
	m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;
	
	//m_pFrameBuffer = new CBcmFrameBuffer (OUTPUT_DISPLAY_WIDTH, OUTPUT_DISPLAY_HEIGHT, 8);
	//	
	//for(int n = 0; n < 16; n++)
	//{
	//	m_pFrameBuffer->SetPalette32 (n, palettecga[n]);  
	//}
	//m_pFrameBuffer->SetPalette (0, 0x0000);  // black
	//m_pFrameBuffer->SetPalette (1, 0x0010);  // blue
	//m_pFrameBuffer->SetPalette (2, 0x8000);  // red
	//m_pFrameBuffer->SetPalette (3, 0x8010);  // magenta
	//m_pFrameBuffer->SetPalette (4, 0x0400);  // green
	//m_pFrameBuffer->SetPalette (5, 0x0410);  // cyan
	//m_pFrameBuffer->SetPalette (6, 0x8400);  // yellow
	//m_pFrameBuffer->SetPalette (7, 0x8410);  // white
	//m_pFrameBuffer->SetPalette (8, 0x0000);  // black
	//m_pFrameBuffer->SetPalette (9, 0x001F);  // bright blue
	//m_pFrameBuffer->SetPalette (10, 0xF800); // bright red
	//m_pFrameBuffer->SetPalette (11, 0xF81F); // bright magenta
	//m_pFrameBuffer->SetPalette (12, 0x07E0); // bright green
	//m_pFrameBuffer->SetPalette (13, 0x07FF); // bright cyan
	//m_pFrameBuffer->SetPalette (14, 0xFFE0); // bright yellow
	//m_pFrameBuffer->SetPalette (15, 0xFFFF); // bright white

	//if (!m_pFrameBuffer->Initialize()) {
	//	return FALSE;
	//}
	
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

	if(bOK)
	{
		HostInterface = new CircleHostInterface(m_DeviceNameService);
		vmConfig = new Config(HostInterface);
        
		vmConfig->biosFile = new EmbeddedDisk(pcxtbios, sizeof(pcxtbios));
		vmConfig->videoRomFile = new EmbeddedDisk(videorom, sizeof(videorom));
		vmConfig->asciiFile = new EmbeddedDisk(asciivga, sizeof(asciivga));
#if USE_EMBEDDED_BOOT_FLOPPY
		vmConfig->diskDriveA = new EmbeddedDisk(dosboot, sizeof(dosboot));
#endif
#if USE_MMC_MOUNTING
		vmConfig->diskDriveC = new MMCDisk(&m_EMMC);
#endif
        
		// TODO
		//vmConfig->parseCommandLine(argc, argv);
		
		vm = new VM(*vmConfig);
		
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

	//while(1)
	//{
	//	m_Timer.MsDelay (100);
	//}
	
#if !USE_BCM_FRAMEBUFFER
	unsigned int nCount = 0;
#endif
	
	while (vm->simulate())
	{
		HostInterface->tick(*vm);
		
#if !USE_BCM_FRAMEBUFFER
		m_Screen.Rotor (0, nCount++);
#endif
		//m_Timer.MsDelay (0);
	}
	
	return ShutdownHalt;
	
	/*
	CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice ("ukbd1", FALSE);
	
	if(pKeyboard)
	{
		pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);
		//	pKeyboard->RegisterKeyPressedHandler (KeyPressedHandler);
	}
	else return ShutdownHalt;
	
	char screentext[80*25+1];
	int cursorX, cursorY;
	int firstRow = 5;
	int count = 0;
	uint32_t lastRefreshTime = CTimer::GetClockTicks();
	const int refreshRate = 60;
	const int refreshDelay = CLOCKHZ / refreshRate;
	
	while(simulatefake86())
	{
		if(pKeyboard)
			pKeyboard->UpdateLEDs ();
		
		// TODO: This should really be based on screen refresh rate / vsync interrupt
		uint32_t currentTime = CTimer::GetClockTicks();
		uint32_t timeElapsedSinceRefresh = (currentTime >= lastRefreshTime) ? (currentTime - lastRefreshTime) : (0xffffffff - lastRefreshTime) + (currentTime);
		
		if(timeElapsedSinceRefresh >= refreshDelay)
		{
			uint32_t* palettePtr;
			int paletteSize;
			
			getactivepalette((uint8_t**)&palettePtr, &paletteSize);
			
			for(int n = 0; n < paletteSize; n++)
			{
				m_pFrameBuffer->SetPalette32 (n, palettePtr[n]);
			}
			m_pFrameBuffer->UpdatePalette();
			
			drawfake86((uint8_t*) m_pFrameBuffer->GetBuffer());
			lastRefreshTime = currentTime;
			//m_Timer.MsDelay (1);
		}
		
		while(m_InputBufferSize > 0)
		{
			if(m_InputBuffer[m_InputBufferPos].eventType == EventType::KeyPress)
			{
				handlekeydownraw(m_InputBuffer[m_InputBufferPos].scancode);
			}
			else
			{
				handlekeyupraw(m_InputBuffer[m_InputBufferPos].scancode);
			}
			m_InputBufferPos++;
			m_InputBufferSize --;
			if(m_InputBufferPos >= INPUT_BUFFER_SIZE)
			{
				m_InputBufferPos = 0;
			}
		}
	}*/

	return ShutdownHalt;
}

