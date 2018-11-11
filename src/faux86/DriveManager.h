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

#include "Types.h"

namespace Faux86
{
	class VM;

	class DiskInterface
	{
	public:
		virtual ~DiskInterface() {}
		virtual int read(uint8_t *buffer, unsigned count) = 0;
		virtual int write(const uint8_t *buffer, unsigned count) = 0;

		virtual uint64_t seek(uint64_t offset) = 0;
		virtual uint64_t getSize() = 0;

		virtual bool isValid() = 0;
	};

	class EmbeddedDisk : public DiskInterface
	{
	public:
		EmbeddedDisk(uint8_t* inData, uint64_t inLength) :
			data(inData), length(inLength), position(0) {}

		virtual int read(uint8_t *buffer, unsigned count) override;
		virtual int write(const uint8_t *buffer, unsigned count) override;

		virtual uint64_t seek(uint64_t offset) override;
		virtual uint64_t getSize() override { return length; }

		virtual bool isValid() override { return true; }

	private:
		uint8_t* data;
		uint64_t length;
		uint64_t position;
	};

	enum DriveTarget : uint8_t
	{
		DRIVE_A = 0,
		DRIVE_B = 1,
		DRIVE_C = 0x80,
		DRIVE_D = 0x81
	};

	class DriveManager
	{
	public:
		DriveManager(VM& inVM);
		~DriveManager();

		void handleDiskInterrupt();
		bool insertDisk(DriveTarget drive, DiskInterface* disk);
		void ejectDisk(DriveTarget drive);
		void readDisk(DriveTarget drive, uint16_t dstseg, uint16_t dstoff, uint16_t cyl, uint16_t sect, uint16_t head, uint16_t sectcount);
		void writeDisk(DriveTarget drive, uint16_t dstseg, uint16_t dstoff, uint16_t cyl, uint16_t sect, uint16_t head, uint16_t sectcount);

		bool isDiskInserted(DriveTarget drive)
		{
			return drives[drive].disk != nullptr;
		}

		uint8_t hdcount = 0;

	private:
		struct Drive
		{
			DiskInterface* disk = nullptr;
			uint16_t cyls;
			uint16_t sects;
			uint16_t heads;
		};

		Drive drives[256];
		uint8_t sectorbuffer[512];
		VM& vm;
	};
}

