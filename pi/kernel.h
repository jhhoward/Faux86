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
#ifndef _kernel_h
#define _kernel_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <SDCard/emmc.h>
//#include <circle/fs/fat/fatfs.h>
#include <circle/types.h>
#include <circle/usb/dwhcidevice.h>

#define USE_BCM_FRAMEBUFFER 1
#define USE_EMBEDDED_BOOT_FLOPPY 1
#define USE_MMC_MOUNTING 1
#define USE_SERIAL_LOGGING 0

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

namespace Faux86
{
	class VM;
	class Config;
	class CircleHostInterface;
}

class CKernel
{
public:
	CKernel (void);
	~CKernel (void);

	boolean Initialize (void);

	TShutdownMode Run (void);


private:
	// do not change this order
	CMemorySystem		m_Memory;
	CActLED			m_ActLED;
	CKernelOptions		m_Options;
	CDeviceNameService	m_DeviceNameService;
#if !USE_BCM_FRAMEBUFFER
	CScreenDevice		m_Screen;
#endif
	CSerialDevice		m_Serial;
	CExceptionHandler	m_ExceptionHandler;
	CInterruptSystem	m_Interrupt;
	CTimer			m_Timer;
	CLogger			m_Logger;
	CDWHCIDevice		m_DWHCI;

#if USE_MMC_MOUNTING
	CEMMCDevice		m_EMMC;
#endif
	
	//CFATFileSystem		m_FileSystem;
	Faux86::CircleHostInterface* HostInterface;
	Faux86::Config* vmConfig;
	Faux86::VM* vm;
};

#endif
