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

/* Functions to emulate the Intel 8259 prioritized interrupt controller.
   note: this is not a very complete 8259 implementation, but for the purposes
   of a PC, it's all we need. */

#include "PIC.h"
#include "VM.h"

using namespace Faux86;

bool PIC::portReadHandler(uint16_t portnum, uint8_t& outValue)
{
	outValue = 0;

	switch (portnum & 1) 
	{
		   case 0:
			   if (readmode == 0)
			   {
				   outValue = irr;
			   }
			   else
			   {
				   outValue = isr;
			   }
		   case 1: //read mask register
				outValue = imr;
	}

	return true;
}

bool PIC::portWriteHandler(uint16_t portnum, uint8_t value)
{
	 uint8_t i;
	 switch (portnum & 1) {
		case 0:
		 if (value & 0x10) { //begin initialization sequence
			icwstep = 1;
			imr = 0; //clear interrupt mask register
			icw[icwstep++] = value;
			return true;
		 }
		 if ((value & 0x98)==8) { //it's an OCW3
			if (value & 2) readmode = value & 2;
		 }
		 if (value & 0x20) { //EOI command
			vm.input.markKeyEventHandled();
			for (i=0; i<8; i++)
			if ((isr >> i) & 1) {
			   isr ^= (1 << i);
			   if (i == 0 && vm.cpu.makeupticks > 0) 
			   { 
				   vm.cpu.makeupticks = 0; 
				   irr |= 1; 
			   }
			   return true;
			}
		 }
		 break;
		case 1:
		 if ((icwstep==3) && (icw[1] & 2)) icwstep = 4; //single mode, so don't read ICW3
		 if (icwstep<5) { icw[icwstep++] = value; return true; }
		 //if we get to this point, this is just a new IMR value
		 imr = value;
		 break;
	 }

	 return true;
}

uint8_t PIC::nextintr() 
{
	uint8_t i, tmpirr;
	tmpirr = irr & (~imr); //XOR request register with inverted mask register
	for (i = 0; i < 8; i++)
	{
		if ((tmpirr >> i) & 1)
		{
			irr ^= (1 << i);
			isr |= (1 << i);
			return(icw[2] + i);
		}
	}
	return(0); //this won't be reached, but without it the compiler gives a warning
}

void PIC::doirq(uint8_t irqnum) 
{
	 irr |= (1 << irqnum);
}

PIC::PIC(VM& inVM) 
	: imr(0)		//mask register
	, irr(0)		//request register
	, isr(0)		//service register
	, icwstep(0)	//used during initialization to keep track of which ICW we're at
	, icw {0, 0, 0, 0, 0}
	, intoffset(0)	//interrupt vector offset
	, priority(0)	//which IRQ has highest priority
	, autoeoi(0)	//automatic EOI mode
	, readmode(0)	//remember what to return on read register from OCW3
	, enabled(0)
	, vm(inVM)
{
	vm.ports.setPortRedirector(0x20, 0x21, this);
}

