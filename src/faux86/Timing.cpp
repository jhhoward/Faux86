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

#include "VM.h"
#include "Timing.h"

using namespace Faux86;

TimingScheduler::TimingScheduler(VM& inVM)
	: vm(inVM)
{
}


void TimingScheduler::init()
{
	curtick = getTicks();

	lasti8253tick = lastblastertick = lastadlibtick = lastssourcetick = lastsampletick = lastscanlinetick = lasttick = curtick;
	scanlinetiming = getHostFreq() / 31500;
	ssourceticks = getHostFreq() / 8000;
	adlibticks = getHostFreq() / 48000;
	if (vm.config.enableAudio) sampleticks = getHostFreq() / gensamplerate;
	else sampleticks = -1;
	i8253tickgap = getHostFreq() / 119318;
}

void TimingScheduler::tick() 
{
	uint8_t i8253chan;

	curtick = getTicks();

	if (curtick >= (lastscanlinetick + scanlinetiming) ) {
			curscanline = (curscanline + 1) % 525;
			if (curscanline > 479) vm.video.port3da = 8;
			else vm.video.port3da = 0;
			if (curscanline & 1) vm.video.port3da |= 1;
			pit0counter++;
			lastscanlinetick = curtick;
		}

	if (vm.pit.active[0]) { //timer interrupt channel on i8253
			if (curtick >= (lasttick + tickgap) ) {
					lasttick = curtick;
					vm.pic.doirq (0);
				}
		}

	if (curtick >= (lasti8253tick + i8253tickgap) ) {
			for (i8253chan=0; i8253chan<3; i8253chan++) {
					if (vm.pit.active[i8253chan]) {
							if (vm.pit.counter[i8253chan] < 10) vm.pit.counter[i8253chan] = vm.pit.chandata[i8253chan];
							vm.pit.counter[i8253chan] -= 10;
						}
				}
			lasti8253tick = curtick;
		}

	if (curtick >= (lastssourcetick + ssourceticks) ) {
			vm.soundSource.tick();
			lastssourcetick = curtick - (curtick - (lastssourcetick + ssourceticks) );
		}

	if (vm.blaster.samplerate > 0) {
			if (curtick >= (lastblastertick + vm.blaster.sampleticks) ) {
					vm.blaster.tick();
					lastblastertick = curtick - (curtick - (lastblastertick + vm.blaster.sampleticks) );
				}
		}

	if (curtick >= (lastsampletick + sampleticks) ) {
			vm.audio.tick();
			if (vm.config.slowSystem) {
					vm.audio.tick();
					vm.audio.tick();
					vm.audio.tick();
				}
			lastsampletick = curtick - (curtick - (lastsampletick + sampleticks) );
		}

	if (curtick >= (lastadlibtick + adlibticks) ) {
			vm.adlib.tick();
			lastadlibtick = curtick - (curtick - (lastadlibtick + adlibticks) );
		}
}

uint64_t TimingScheduler::getTicks()
{
	return vm.config.hostSystemInterface->getTimer().getTicks();
}

uint64_t TimingScheduler::getHostFreq()
{
	return vm.config.hostSystemInterface->getTimer().getHostFreq();
}

uint64_t TimingScheduler::getElapsed(uint64_t prevTick)
{
	return getTicks() - prevTick;
}

uint64_t TimingScheduler::getElapsedMS(uint64_t prevTick)
{
	return getElapsed(prevTick) * 1000 / getHostFreq();
}

uint64_t TimingScheduler::getMS()
{
	return (getTicks() * 1000) / getHostFreq();
}
