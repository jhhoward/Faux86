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
/* Functions to emulate a Creative Labs Sound Blaster Pro. */

#include "VM.h"
#include "SoundBlaster.h"
#include "MemUtils.h"

using namespace Faux86;

void SoundBlaster::bufNewData (uint8_t value) 
{
	if (memptr >= sizeof (mem) ) return;
	mem[memptr] = value;
	memptr++;
}

void SoundBlaster::setsampleticks() 
{
	if (samplerate == 0) 
	{
		sampleticks = 0;
		return;
	}
	sampleticks = vm.timing.getHostFreq() / (uint64_t) samplerate;
}

void SoundBlaster::cmd (uint8_t value) 
{
	uint8_t recognized = 1;
	if (waitforarg) 
	{
			switch (lastcmdval) {
					case 0x10: //direct 8-bit sample output
						sample = value;
						break;
					case 0x14: //8-bit single block DMA output
					case 0x24:
					case 0x91:
						if (waitforarg == 2) {
								blocksize = (blocksize & 0xFF00) | (uint32_t) value;
								waitforarg = 3;
								return;
							}
						else {
								blocksize = (blocksize & 0x00FF) | ( (uint32_t) value << 8);
#ifdef DEBUG_BLASTER
								printf ("[NOTICE] Sound Blaster DSP block transfer size set to %u\n", blocksize);
#endif
								usingdma = 1;
								blockstep = 0;
								useautoinit = 0;
								paused8 = 0;
								speakerstate = 1;
							}
						break;
					case 0x40: //set time constant
						samplerate = (uint16_t) ( (uint32_t) 1000000 / (uint32_t) (256 - (uint32_t) value) );
						setsampleticks();
#ifdef DEBUG_BLASTER
						printf ("[DEBUG] Sound Blaster time constant received, sample rate = %u\n", samplerate);
#endif
						break;
					case 0x48: //set DSP block transfer size
						if (waitforarg == 2) {
								blocksize = (blocksize & 0xFF00) | (uint32_t) value;
								waitforarg = 3;
								return;
							}
						else {
								blocksize = (blocksize & 0x00FF) | ( (uint32_t) value << 8);
								//if (blocksize == 0) blocksize = 65536;
								blockstep = 0;
#ifdef DEBUG_BLASTER
								printf ("[NOTICE] Sound Blaster DSP block transfer size set to %u\n", blocksize);
#endif
							}
						break;
					case 0xE0: //DSP identification for Sound Blaster 2.0 and newer (invert each bit and put in read buffer)
						bufNewData (~value);
						break;
					case 0xE4: //DSP write test, put data value into read buffer
						bufNewData (value);
						lasttestval = value;
						break;
					default:
						recognized = 0;
				}
			waitforarg--; // = 0;
			//if (recognized) return;
			//waitforarg = 0;
		}

	switch (value) {
			case 0x10:
			case 0x40:
			case 0xE0:
			case 0xE4:
				waitforarg = 1;
				break;

			case 0x14: //8-bit single block DMA output
			case 0x24:
			case 0x48:
			case 0x91:
				waitforarg = 2;
				break;

			case 0x1C: //8-bit auto-init DMA output
			case 0x2C:
				usingdma = 1;
				blockstep = 0;
				useautoinit = 1;
				paused8 = 0;
				speakerstate = 1;
				break;

			case 0xD0: //pause 8-bit DMA I/O
				paused8 = 1;
			case 0xD1: //speaker output on
				speakerstate = 1;
				break;
			case 0xD3: //speaker output off
				speakerstate = 0;
				break;
			case 0xD4: //continue 8-bit DMA I/O
				paused8 = 0;
				break;
			case 0xD8: //get speaker status
				if (speakerstate) bufNewData (0xFF);
				else bufNewData (0x00);
				break;
			case 0xDA: //exit 8-bit auto-init DMA I/O mode
				usingdma = 0;
				break;
			case 0xE1: //get DSP version info
				memptr = 0;
				bufNewData (dspmaj);
				bufNewData (dspmin);
				break;
			case 0xE8: //DSP read test
				memptr = 0;
				bufNewData (lasttestval);
				break;
			case 0xF2: //force 8-bit IRQ
				vm.pic.doirq (sbirq);
				break;
			case 0xF8: //undocumented command, clears in-buffer and inserts a null byte
				memptr = 0;
				bufNewData (0);
				break;
			default:
				if (!recognized)
				{
					log(Log, "[NOTICE] Sound Blaster received unhandled command %02Xh\n", value);
				}
				break;
		}
}

bool SoundBlaster::portWriteHandler(uint16_t portnum, uint8_t value)
{
#ifdef DEBUG_BLASTER
	printf ("[DEBUG] outBlaster: port %Xh, value %02X\n", portnum, value);
#endif
	portnum &= 0xF;
	switch (portnum) {
			case 0x0:
			case 0x8:
				return adlib.portWriteHandler(0x388, value);
			case 0x1:
			case 0x9:
				return adlib.portWriteHandler(0x389, value);
			case 0x4: //mixer address port
				mixerindex = value;
				break;
			case 0x5: //mixer data
				mixer[mixerindex] = value;
				break;
			case 0x6: //reset port
				if ( (value == 0x00) && (lastresetval == 0x01) ) {
						speakerstate = 0;
						sample = 128;
						waitforarg = 0;
						memptr = 0;
						usingdma = 0;
						blocksize = 65535;
						blockstep = 0;
						bufNewData (0xAA);
						MemUtils::memset (mixer, 0xEE, sizeof (mixer) );
#ifdef DEBUG_BLASTER
						printf ("[DEBUG] Sound Blaster received reset!\n");
#endif
					}
				lastresetval = value;
				break;
			case 0xC: //write command/data
				cmd (value);
				if (waitforarg != 3) lastcmdval = value;
				break;
		}

	return true;
}

bool SoundBlaster::portReadHandler (uint16_t portnum, uint8_t& ret) 
{
#ifdef DEBUG_BLASTER
	static uint16_t lastread = 0;
#endif
#ifdef DEBUG_BLASTER
	//if (lastread != portnum) printf ("[DEBUG] inBlaster: port %Xh, value ", portnum);
#endif
	portnum &= 0xF;
	switch (portnum) {
			case 0x0:
			case 0x8:
				return adlib.portReadHandler(0x388, ret);
			case 0x1:
			case 0x9:
				return adlib.portReadHandler(0x389, ret);
			case 0x5: //mixer data
				ret = mixer[mixerindex];
				break;
			case 0xA: //read data
				if (memptr == 0) {
						ret = 0;
					}
				else {
						ret = mem[0];
						MemUtils::memmove (&mem[0], &mem[1], sizeof (mem) - 1);
						memptr--;
					}
				break;
			case 0xE: //read-buffer status
				if (memptr > 0) ret = 0x80;
				else ret = 0x00;
				break;
			default:
				ret = 0x00;
		}
#ifdef DEBUG_BLASTER
	//if (lastread != portnum) printf ("%02X\n", ret);
	//lastread = portnum;
#endif
	return true;
}

void SoundBlaster::tick() 
{
	if (!usingdma) return;
	sample = vm.dma.read (sbdma);
	blockstep++;
	if (blockstep > blocksize) 
	{
		vm.pic.doirq (sbirq);
#ifdef DEBUG_BLASTER
		printf ("[NOTICE] Sound Blaster did IRQ\n");
#endif
		if (useautoinit) 
		{
			blockstep = 0;
		}
		else 
		{
			usingdma = 0;
		}
	}
}

int16_t SoundBlaster::generateSample() 
{
	if (speakerstate == 0) return (0);
	else return ( (int16_t) sample - 128);
}

SoundBlaster::SoundBlaster(VM& inVM, Adlib& inAdlib)
	: vm(inVM), adlib(inAdlib)
{
}

void SoundBlaster::init()
{
	MemUtils::memset(&mem, 0, 1024);
	MemUtils::memset(&mixer, 0, 256);

	dspmaj = 2; //emulate a Sound Blaster 2.0
	//dspmaj = 3; //emulate a Sound Blaster Pro
	dspmin = 0;
	sbirq = vm.config.blaster.irq;
	sbdma = 1;

	//mixer.reg[0x22] = mixer.reg[0x26] = mixer.reg[0x04] = (4 << 5) | (4 << 1);
	mixer[0x22] = mixer[0x26] = mixer[0x04] = (4 << 5) | (4 << 1);

	uint16_t baseport = vm.config.blaster.port;
	vm.ports.setPortRedirector(baseport, baseport + 0xE, this);
}
