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
#include "PCSpeaker.h"

using namespace Faux86;

int16_t PCSpeaker::generateSample() 
{
	int16_t speakervalue;

	speakerfullstep = (uint64_t) ( (float) vm.timing.gensamplerate / (float) vm.pit.chanfreq[2]);
	if (speakerfullstep < 2)
	{
		speakerfullstep = 2;
	}
	speakerhalfstep = speakerfullstep >> 1;
	if (speakercurstep < speakerhalfstep) 
	{
		speakervalue = 32;
	}
	else 
	{
		speakervalue = -32;
	}
	speakercurstep = (speakercurstep + 1) % speakerfullstep;
	return (speakervalue);
}

PCSpeaker::PCSpeaker(VM& inVM)
	: vm(inVM)
{
}
