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

#include "VM.h"
#include "DriveManager.h"

using namespace Faux86;

DriveManager::DriveManager(VM& inVM) : vm(inVM)
{
}

DriveManager::~DriveManager() 
{
	for (int n = 0; n < 256; n++)
	{
		if (drives[n].disk)
		{
			delete drives[n].disk;
		}
	}
}

bool DriveManager::insertDisk(DriveTarget targetDrive, DiskInterface* disk)
{
	if (!disk)
		return false;

	if (!disk->isValid())
	{
		delete disk;
		return false;
	}

	Drive& drive = drives[targetDrive];

	if(drive.disk != nullptr)
	{
		ejectDisk(targetDrive);
	}

	drive.disk = disk;
	uint64_t diskSize = disk->getSize();

	if (targetDrive >= 0x80) 
	{ 
		// TODO: read all partitions from MBR

		disk->read(sectorbuffer, 512);
		uint32_t partitionEntry = 0x1BE;

		if (sectorbuffer[partitionEntry] == 0x80)
		{
			uint16_t partitionHeadEnd = sectorbuffer[partitionEntry + 5];
			uint16_t partitionSectorEnd = sectorbuffer[partitionEntry + 6] & 0x3f;
			uint16_t partitionCylinderEnd = sectorbuffer[partitionEntry + 7] | (sectorbuffer[partitionEntry + 6] << 8);

			drive.sects = partitionSectorEnd;
			drive.heads = partitionHeadEnd + 1;
			drive.cyls = partitionCylinderEnd;
		}
		else
		{
			//it's a hard disk image
			drive.sects = 63;
			drive.heads = 16;
			//disk[drivenum].cyls = disk[drivenum].filesize / (disk[drivenum].sects * disk[drivenum].heads * 512);
			drive.cyls = (uint16_t)(diskSize / (64 * 16 * 512));
		}

		hdcount++;
	}
	else 
	{
		//it's a floppy image
		drive.cyls = 80;
		drive.sects = 18;
		drive.heads = 2;
		if (diskSize <= 1228800) 
			drive.sects = 15;
		if (diskSize <= 737280) 
			drive.sects = 9;
		if (diskSize <= 368640) 
		{
			drive.cyls = 40;
			drive.sects = 9;
		}
		if (diskSize <= 163840)
		{
			drive.cyls = 40;
			drive.sects = 8;
			drive.heads = 1;
		}
	}

	return true;
}

void DriveManager::ejectDisk (DriveTarget drive)
{
	if (drives[drive].disk != nullptr)
	{
		delete drives[drive].disk;
		drives[drive].disk = nullptr;
	}
}

void DriveManager::readDisk (DriveTarget targetDrive, uint16_t dstseg, uint16_t dstoff, uint16_t cyl, uint16_t sect, uint16_t head, uint16_t sectcount)
{
	Drive& drive = drives[targetDrive];
	uint32_t memdest, lba, fileoffset, cursect, sectoffset;
	if (!sect || !drive.disk) return;
	lba = ((uint32_t)cyl * (uint32_t)drive.heads + (uint32_t)head) * (uint32_t)drive.sects + (uint32_t)sect - 1;
	fileoffset = lba * 512;
	if (fileoffset>drive.disk->getSize()) return;

	drive.disk->seek(fileoffset);
	memdest = ((uint32_t)dstseg << 4) + (uint32_t)dstoff;
	//for the readdisk function, we need to use write86 instead of directly fread'ing into
	//the RAM array, so that read-only flags are honored. otherwise, a program could load
	//data from a disk over BIOS or other ROM code that it shouldn't be able to.
	for (cursect = 0; cursect<sectcount; cursect++) {
		if (drive.disk->read(sectorbuffer, 512) < 512) break;
		for (sectoffset = 0; sectoffset<512; sectoffset++) {
			vm.memory.writeByte(memdest++, sectorbuffer[sectoffset]);
		}
	}

	vm.cpu.regs.byteregs[regal] = cursect;
	vm.cpu.cf = 0;
	vm.cpu.regs.byteregs[regah] = 0;
}

void DriveManager::writeDisk (DriveTarget driveTarget, uint16_t dstseg, uint16_t dstoff, uint16_t cyl, uint16_t sect, uint16_t head, uint16_t sectcount) 
{
	Drive& drive = drives[driveTarget];
	uint32_t memdest, lba, fileoffset, cursect, sectoffset;
	if (!sect || !drive.disk) return;
	lba = ((uint32_t)cyl * (uint32_t)drive.heads + (uint32_t)head) * (uint32_t)drive.sects + (uint32_t)sect - 1;
	fileoffset = lba * 512;
	if (fileoffset>drive.disk->getSize()) return;

	drive.disk->seek(fileoffset);
	memdest = ((uint32_t)dstseg << 4) + (uint32_t)dstoff;
	for (cursect = 0; cursect < sectcount; cursect++) 
	{
		for (sectoffset = 0; sectoffset < 512; sectoffset++) 
		{
			sectorbuffer[sectoffset] = vm.memory.readByte(memdest++);
		}
		drive.disk->write(sectorbuffer, 512);
	}

	vm.cpu.regs.byteregs[regal] = (uint8_t)sectcount;
	vm.cpu.cf = 0;
	vm.cpu.regs.byteregs[regah] = 0;
}

void DriveManager::handleDiskInterrupt() 
{
	static uint8_t lastdiskah[256], lastdiskcf[256];
	union CPU::_bytewordregs_& regs = vm.cpu.regs;
	uint16_t* segregs = vm.cpu.segregs;
	uint8_t& cf = vm.cpu.cf;

	switch (regs.byteregs[regah]) {
			case 0: //reset disk system
				regs.byteregs[regah] = 0;
				cf = 0; //useless function in an emulator. say success and return.
				break;
			case 1: //return last status
				regs.byteregs[regah] = lastdiskah[regs.byteregs[regdl]];
				cf = lastdiskcf[regs.byteregs[regdl]];
				return;
			case 2: //read sector(s) into memory
				if (drives[regs.byteregs[regdl]].disk) {
						readDisk ((DriveTarget)regs.byteregs[regdl], segregs[reges], getreg16 (regbx), regs.byteregs[regch] + (regs.byteregs[regcl]/64) *256, regs.byteregs[regcl] & 63, regs.byteregs[regdh], regs.byteregs[regal]);
						cf = 0;
						regs.byteregs[regah] = 0;
					}
				else {
						cf = 1;
						regs.byteregs[regah] = 1;
					}
				break;
			case 3: //write sector(s) from memory
				if (drives[regs.byteregs[regdl]].disk) {
						writeDisk ((DriveTarget)regs.byteregs[regdl], segregs[reges], getreg16 (regbx), regs.byteregs[regch] + (regs.byteregs[regcl]/64) *256, regs.byteregs[regcl] & 63, regs.byteregs[regdh], regs.byteregs[regal]);
						cf = 0;
						regs.byteregs[regah] = 0;
					}
				else {
						cf = 1;
						regs.byteregs[regah] = 1;
					}
				break;
			case 4:
			case 5: //format track
				cf = 0;
				regs.byteregs[regah] = 0;
				break;
			case 8: //get drive parameters
				if (drives[regs.byteregs[regdl]].disk) {
						cf = 0;
						regs.byteregs[regah] = 0;
						regs.byteregs[regch] = drives[regs.byteregs[regdl]].cyls - 1;
						regs.byteregs[regcl] = drives[regs.byteregs[regdl]].sects & 63;
						regs.byteregs[regcl] = regs.byteregs[regcl] + (drives[regs.byteregs[regdl]].cyls/256) *64;
						regs.byteregs[regdh] = drives[regs.byteregs[regdl]].heads - 1;
						if (regs.byteregs[regdl]<0x80) {
								regs.byteregs[regbl] = 4; //else regs.byteregs[regbl] = 0;
								regs.byteregs[regdl] = 2;
							}
						else regs.byteregs[regdl] = hdcount;
					}
				else {
						cf = 1;
						regs.byteregs[regah] = 0xAA;
					}
				break;
			default:
				cf = 1;
		}
	lastdiskah[regs.byteregs[regdl]] = regs.byteregs[regah];
	lastdiskcf[regs.byteregs[regdl]] = cf;
	if (regs.byteregs[regdl] & 0x80)
	{
		vm.memory.RAM[0x474] = regs.byteregs[regah];
	}
}

int EmbeddedDisk::read(uint8_t *buffer, unsigned count)
{
	int bytesRead = 0;

	while (position < length && count)
	{
		*buffer++ = data[position++];
		count--;
		bytesRead++;
	}

	return bytesRead;
}

int EmbeddedDisk::write(const uint8_t *buffer, unsigned count)
{
	int bytesWritten = 0;

	while (position < length && count)
	{
		data[position++] = *buffer++;
		count--;
		bytesWritten++;
	}

	return bytesWritten;
}

uint64_t EmbeddedDisk::seek(uint64_t offset)
{
	position = offset;
	return position;
}
