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
#include <Windows.h>
#include <stdio.h>
#include <new>
#include "../../src/faux86/VM.h"
#include "../../src/faux86/DriveManager.h"
#include "SDLInterface.h"

#include "../../data/asciivga.h"
#include "../../data/pcxtbios.h"
#include "../../data/videorom.h"

int main(int argc, char *argv[])
{
	Faux86::SDLHostSystemInterface hostInterface;

	Faux86::Config vmConfig(&hostInterface);
	
	//vmConfig.biosFile = new Faux86::EmbeddedDisk(pcxtbios, sizeof(pcxtbios));
	//vmConfig.videoRomFile = new Faux86::EmbeddedDisk(videorom, sizeof(videorom));
	//vmConfig.asciiFile = new Faux86::EmbeddedDisk(asciivga, sizeof(asciivga));

	vmConfig.parseCommandLine(argc, argv);
	//vmConfig.singleThreaded = false;

	uint8_t* allocSpace = new uint8_t[sizeof(Faux86::VM)];
	for (size_t n = 0; n < sizeof(Faux86::VM); n++)
	{
		allocSpace[n] = 0xff;
	}

	Faux86::VM* f86 = new (allocSpace) Faux86::VM(vmConfig);

	if (f86->init())
	{
		while (f86->simulate())
		{
			hostInterface.tick(*f86);
			//Sleep(0);
		}
	}

	delete f86;

	return 0;
}
