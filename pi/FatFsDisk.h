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

#include <fatfs/ff.h>
#include "DriveManager.h"

namespace Faux86
{
	class FatFsDisk : public DiskInterface
	{
	private:
		FIL file;
		FatFsDisk(FIL& inFile) : file(inFile) {}

	public:
		static FatFsDisk* open(const char* path)
		{
			FIL file;
			
			log(Log, "Attempting to open %s for read", path);
						
			if(f_open (&file, path, FA_READ | FA_OPEN_EXISTING) == FR_OK)
			{
				log(Log, "Success!");
				return new FatFsDisk(file);
			}
			
			log(Log, "Could not open file %s", path);
			
			return nullptr;
		}
		
		virtual int read (uint8_t *buffer, unsigned count) override
		{
			unsigned bytesRead;
			f_read(&file, buffer, count, &bytesRead);
			return bytesRead;
		}
		virtual int write (const uint8_t *buffer, unsigned count) override
		{
			unsigned bytesWritten;
			f_write(&file, buffer, count, &bytesWritten);
			return bytesWritten;
		}
		virtual uint64_t seek (uint64_t offset) override
		{
			f_lseek(&file, offset);
			return offset;
		}
		virtual uint64_t getSize() override
		{
			return f_size(&file);
		}
		virtual bool isValid() override
		{
			return true;
		}
	};
}
