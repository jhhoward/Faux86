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

#define regax 0
#define regcx 1
#define regdx 2
#define regbx 3
#define regsp 4
#define regbp 5
#define regsi 6
#define regdi 7
#define reges 0
#define regcs 1
#define regss 2
#define regds 3

#ifdef __BIG_ENDIAN__
#define regal 1
#define regah 0
#define regcl 3
#define regch 2
#define regdl 5
#define regdh 4
#define regbl 7
#define regbh 6
#else
#define regal 0
#define regah 1
#define regcl 2
#define regch 3
#define regdl 4
#define regdh 5
#define regbl 6
#define regbh 7
#endif

#define StepIP(x)	ip += x
#define getmem8(x, y)	vm.memory.readByte(segbase(x) + (y))
#define getmem16(x, y)	vm.memory.readWord(segbase(x) + (y))
#define putmem8(x, y, z)	vm.memory.writeByte(segbase(x) + (y), z)
#define putmem16(x, y, z)	vm.memory.writeWord(segbase(x) + (y), z)
#define signext(value)	(int16_t)(int8_t)(value)
#define signext32(value)	(int32_t)(int16_t)(value)
#define getreg16(regid)	regs.wordregs[regid]
#define getreg8(regid)	regs.byteregs[byteregtable[regid]]
#define putreg16(regid, writeval)	regs.wordregs[regid] = writeval
#define putreg8(regid, writeval)	regs.byteregs[byteregtable[regid]] = writeval
#define getsegreg(regid)	segregs[regid]
#define putsegreg(regid, writeval)	segregs[regid] = writeval
#define segbase(x)	((uint32_t) x << 4)
#define segaddr(seg, x) (segbase(seg) + ((x) & 0xFFFF))

#define makeflagsword() \
	( \
	2 | (uint16_t) cf | ((uint16_t) pf << 2) | ((uint16_t) af << 4) | ((uint16_t) zf << 6) | ((uint16_t) sf << 7) | \
	((uint16_t) tf << 8) | ((uint16_t) ifl << 9) | ((uint16_t) df << 10) | ((uint16_t) of << 11) \
	)

#define decodeflagsword(x) { \
	temp16 = x; \
	cf = temp16 & 1; \
	pf = (temp16 >> 2) & 1; \
	af = (temp16 >> 4) & 1; \
	zf = (temp16 >> 6) & 1; \
	sf = (temp16 >> 7) & 1; \
	tf = (temp16 >> 8) & 1; \
	ifl = (temp16 >> 9) & 1; \
	df = (temp16 >> 10) & 1; \
	of = (temp16 >> 11) & 1; \
	}
