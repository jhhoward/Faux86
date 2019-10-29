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

#include "../../src/faux86/DriveManager.h"

#include <stdio.h>
namespace Faux86
{
	class StdioDiskInterface : public DiskInterface
	{
	public:
		StdioDiskInterface(const char* filename);
		virtual ~StdioDiskInterface();
		virtual int read(uint8_t *buffer, unsigned count) override;
		virtual int write(const uint8_t *buffer, unsigned count) override;

		virtual uint64_t seek(uint64_t offset) override;
		virtual uint64_t getSize() override;

		virtual bool isValid() override { return diskFile != nullptr; }

	private:
		FILE* diskFile;
		uint64_t diskSize;
	};
}
