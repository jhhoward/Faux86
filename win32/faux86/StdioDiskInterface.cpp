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
#include "StdioDiskInterface.h"

using namespace Faux86;

StdioDiskInterface::StdioDiskInterface(const char* filename)
{
	fopen_s(&diskFile, filename, "r+b");

	if (diskFile)
	{
		fseek(diskFile, 0L, SEEK_END);
		diskSize = ftell(diskFile);
		fseek(diskFile, 0L, SEEK_SET);
	}
	else
	{
		fprintf(stderr, "Error loading file %s\n", filename);
		diskSize = 0;
	}
}

StdioDiskInterface::~StdioDiskInterface()
{
	if (diskFile)
	{
		fclose(diskFile);
	}
}

int StdioDiskInterface::read(uint8_t *buffer, unsigned count)
{
	return fread(buffer, 1, count, diskFile);
}

int StdioDiskInterface::write(const uint8_t *buffer, unsigned count)
{
	int result = fwrite(buffer, 1, count, diskFile);
	fflush(diskFile);
	return result;
}

uint64_t StdioDiskInterface::seek(uint64_t offset)
{
	return fseek(diskFile, (long)offset, SEEK_SET);
}

uint64_t StdioDiskInterface::getSize()
{
	return diskSize;
}
