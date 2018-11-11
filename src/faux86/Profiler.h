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
#pragma once

#include "Timing.h"

namespace Faux86
{
	class ProfileBlock
	{
	private:
		const char* name;
		TimingScheduler& scheduler;
		uint64_t startTime;

	public:
		ProfileBlock(TimingScheduler& timing, const char* inName) :
			name(inName),
			scheduler(timing),
			startTime(timing.getTicks())
		{
		}

		~ProfileBlock()
		{
			float elapsedTime = (float)(scheduler.getTicks() - startTime);
			elapsedTime = elapsedTime * 1000.0f / scheduler.getHostFreq();
			log(Log, "%s took %f ms", name, elapsedTime);
		}

	};
}