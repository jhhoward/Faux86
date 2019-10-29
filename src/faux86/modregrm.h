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
#include "config.h"

#ifdef CPU_ADDR_MODE_CACHE
struct addrmodecache_s addrcache[0x100000];
uint8_t addrcachevalid[0x100000];

uint32_t addrdatalen, dataisvalid, setvalidptr;
uint64_t cached_access_count = 0, uncached_access_count = 0;
#define modregrm() { \
	tempaddr32 = (((uint32_t)savecs << 4) + ip) & 0xFFFFF; \
	if (addrcachevalid[tempaddr32]) { \
		switch (addrcache[tempaddr32].len) { \
			case 0: \
				dataisvalid = 1; \
				break; \
			case 1: \
				if (addrcachevalid[tempaddr32+1]) dataisvalid = 1; else dataisvalid = 0; \
				break; \
			case 2: \
				if (addrcachevalid[tempaddr32+1] && addrcachevalid[tempaddr32+2]) dataisvalid = 1; else dataisvalid = 0; \
				break; \
		} \
	} else dataisvalid = 0; \
	if (dataisvalid) { \
		cached_access_count++; \
		disp16 = addrcache[tempaddr32].disp16; \
		segregs[regcs] = addrcache[tempaddr32].exitcs; \
		ip = addrcache[tempaddr32].exitip; \
		mode = addrcache[tempaddr32].mode; \
		reg = addrcache[tempaddr32].reg; \
		rm = addrcache[tempaddr32].rm; \
		if ((!segoverride) && addrcache[tempaddr32].forcess) useseg = segregs[regss]; \
	} else { \
		uncached_access_count++; \
		addrbyte = getmem8(segregs[regcs], ip); \
		StepIP(1); \
		mode = addrbyte >> 6; \
		reg = (addrbyte >> 3) & 7; \
		rm = addrbyte & 7; \
		addrdatalen = 0; \
		addrcache[tempaddr32].forcess = 0; \
		switch(mode) \
		{ \
		case 0: \
		if(rm == 6) { \
		disp16 = getmem16(segregs[regcs], ip); \
		addrdatalen = 2; \
		StepIP(2); \
		} \
		if (((rm == 2) || (rm == 3))) { \
		if (!segoverride) useseg = segregs[regss]; \
		addrcache[tempaddr32].forcess = 1; \
		} \
		break; \
	\
		case 1: \
		disp16 = signext(getmem8(segregs[regcs], ip)); \
		addrdatalen = 1; \
		StepIP(1); \
		if (((rm == 2) || (rm == 3) || (rm == 6))) { \
		if (!segoverride) useseg = segregs[regss]; \
		addrcache[tempaddr32].forcess = 1; \
		} \
		break; \
	\
		case 2: \
		disp16 = getmem16(segregs[regcs], ip); \
		addrdatalen = 2; \
		StepIP(2); \
		if (((rm == 2) || (rm == 3) || (rm == 6))) { \
		if (!segoverride) useseg = segregs[regss]; \
		addrcache[tempaddr32].forcess = 1; \
		} \
		break; \
	\
		default: \
		disp16 = 0; \
		} \
		addrcache[tempaddr32].disp16 = disp16; \
		addrcache[tempaddr32].exitcs = segregs[regcs]; \
		addrcache[tempaddr32].exitip = ip; \
		addrcache[tempaddr32].mode = mode; \
		addrcache[tempaddr32].reg = reg; \
		addrcache[tempaddr32].rm = rm; \
		addrcache[tempaddr32].len = addrdatalen; \
		memset(&addrcachevalid[tempaddr32], 1, addrdatalen+1); \
	} \
}
#else
#define modregrm() { \
	addrbyte = getmem8(segregs[regcs], ip); \
	StepIP(1); \
	mode = addrbyte >> 6; \
	reg = (addrbyte >> 3) & 7; \
	rm = addrbyte & 7; \
	switch(mode) \
	{ \
	case 0: \
	if(rm == 6) { \
	disp16 = getmem16(segregs[regcs], ip); \
	StepIP(2); \
	} \
	if(((rm == 2) || (rm == 3)) && !segoverride) { \
	useseg = segregs[regss]; \
	} \
	break; \
 \
	case 1: \
	disp16 = signext(getmem8(segregs[regcs], ip)); \
	StepIP(1); \
	if(((rm == 2) || (rm == 3) || (rm == 6)) && !segoverride) { \
	useseg = segregs[regss]; \
	} \
	break; \
 \
	case 2: \
	disp16 = getmem16(segregs[regcs], ip); \
	StepIP(2); \
	if(((rm == 2) || (rm == 3) || (rm == 6)) && !segoverride) { \
	useseg = segregs[regss]; \
	} \
	break; \
 \
	default: \
	disp8 = 0; \
	disp16 = 0; \
	} \
}
#endif