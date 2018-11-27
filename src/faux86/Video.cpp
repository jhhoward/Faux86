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

#include "Types.h"
#include "VM.h"
#include "Video.h"
#include "MemUtils.h"

using namespace Faux86;

uint32_t Video::rgb(uint32_t r, uint32_t g, uint32_t b) 
{
#ifdef __BIG_ENDIAN__
	return ( (r<<24) | (g<<16) | (b<<8) );
#else
	return (r | (g<<8) | (b<<16) );
#endif
}

void Video::handleInterrupt() 
{
	uint8_t* RAM = vm.memory.RAM;
	uint8_t* portram = vm.ports.portram;
	uint32_t tempcalc, memloc, n;
	union CPU::_bytewordregs_& regs = vm.cpu.regs;
	uint16_t* segregs = vm.cpu.segregs;

	updatedscreen = 1;
	switch (regs.byteregs[regah]) { //what video interrupt function?
			case 0: //set video mode
				log(LogVerbose, "Set video mode %02Xh\n", regs.byteregs[regal]);
				VGA_SC[0x4] = 0; //VGA modes are in chained mode by default after a mode switch
				//regs.byteregs[regal] = 3;
				switch (regs.byteregs[regal] & 0x7F) {
						case 0: //40x25 mono text
							currentPalette = &paletteCGA;
							videobase = textbase;
							cols = 40;
							rows = 25;
							vidcolor = 0;
							vidgfxmode = 0;
							blankattr = 7;
							for (tempcalc = videobase; tempcalc<videobase+16384; tempcalc+=2) {
									RAM[tempcalc] = 0;
									RAM[tempcalc+1] = blankattr;
								}
							break;
						case 1: //40x25 color text
							currentPalette = &paletteCGA;
							videobase = textbase;
							cols = 40;
							rows = 25;
							vidcolor = 1;
							vidgfxmode = 0;
							blankattr = 7;
							for (tempcalc = videobase; tempcalc<videobase+16384; tempcalc+=2) {
									RAM[tempcalc] = 0;
									RAM[tempcalc+1] = blankattr;
								}
							portram[0x3D8] = portram[0x3D8] & 0xFE;
							break;
						case 2: //80x25 mono text
							currentPalette = &paletteCGA;
							videobase = textbase;
							cols = 80;
							rows = 25;
							vidcolor = 1;
							vidgfxmode = 0;
							blankattr = 7;
							for (tempcalc = videobase; tempcalc<videobase+16384; tempcalc+=2) {
									RAM[tempcalc] = 0;
									RAM[tempcalc+1] = blankattr;
								}
							portram[0x3D8] = portram[0x3D8] & 0xFE;
							break;
						case 3: //80x25 color text
							currentPalette = &paletteCGA;
							videobase = textbase;
							cols = 80;
							rows = 25;
							vidcolor = 1;
							vidgfxmode = 0;
							blankattr = 7;
							for (tempcalc = videobase; tempcalc<videobase+16384; tempcalc+=2) {
									RAM[tempcalc] = 0;
									RAM[tempcalc+1] = blankattr;
								}
							portram[0x3D8] = portram[0x3D8] & 0xFE;
							break;
						case 4:
						case 5: //40x25 color text
							currentPalette = &paletteCGA;
							videobase = textbase;
							cols = 40;
							rows = 25;
							vidcolor = 1;
							vidgfxmode = 1;
							blankattr = 7;
							for (tempcalc = videobase; tempcalc<videobase+16384; tempcalc+=2) {
									RAM[tempcalc] = 0;
									RAM[tempcalc+1] = blankattr;
								}
							if (regs.byteregs[regal]==4) portram[0x3D9] = 48;
							else portram[0x3D9] = 0;
							break;
						case 6:
							currentPalette = &paletteCGA;
							videobase = textbase;
							cols = 80;
							rows = 25;
							vidcolor = 0;
							vidgfxmode = 1;
							blankattr = 7;
							for (tempcalc = videobase; tempcalc<videobase+16384; tempcalc+=2) {
									RAM[tempcalc] = 0;
									RAM[tempcalc+1] = blankattr;
								}
							portram[0x3D8] = portram[0x3D8] & 0xFE;
							break;
						case 7: // TODO: 80x25 mono text
							currentPalette = &paletteCGA;
							videobase = textbase;
							cols = 80;
							rows = 25;
							//vidcolor = 0;
							vidcolor = 1;	// should actually be mono
							vidgfxmode = 0;
							blankattr = 7;
							for (tempcalc = videobase; tempcalc<videobase + 16384; tempcalc += 2) {
								RAM[tempcalc] = 0;
								RAM[tempcalc + 1] = blankattr;
							}
							portram[0x3D8] = portram[0x3D8] & 0xFE;
							break;
						case 127:
							currentPalette = &paletteCGA;
							videobase = 0xB8000;
							cols = 90;
							rows = 25;
							vidcolor = 0;
							vidgfxmode = 1;
							for (tempcalc = videobase; tempcalc<videobase+16384; tempcalc++) {
									RAM[tempcalc] = 0;
								}
							portram[0x3D8] = portram[0x3D8] & 0xFE;
							break;
						case 0x9: //320x200 16-color
							currentPalette = &paletteCGA;
							videobase = 0xB8000;
							cols = 40;
							rows = 25;
							vidcolor = 1;
							vidgfxmode = 1;
							blankattr = 0;
							if ( (regs.byteregs[regal]&0x80) ==0) for (tempcalc = videobase; tempcalc<videobase+65535; tempcalc+=2) {
										RAM[tempcalc] = 0;
										RAM[tempcalc+1] = blankattr;
									}
							portram[0x3D8] = portram[0x3D8] & 0xFE;
							break;
						case 0xD: //320x200 16-color
							currentPalette = &paletteCGA;
							videobase = 0xA0000;
							cols = 40;
							rows = 25;
							vidcolor = 1;
							vidgfxmode = 1;
							blankattr = 0;
							for (tempcalc = videobase; tempcalc<videobase + 65535; tempcalc += 2) {
								RAM[tempcalc] = 0;
								RAM[tempcalc + 1] = blankattr;
							}
							portram[0x3D8] = portram[0x3D8] & 0xFE;
							break;
						case 0x12: //640x480 16-color
						case 0x13: //320x200 256-color
							currentPalette = &paletteVGA;
							videobase = 0xA0000;
							cols = 40;
							rows = 25;
							vidcolor = 1;
							vidgfxmode = 1;
							blankattr = 0;
							for (tempcalc = videobase; tempcalc<videobase+65535; tempcalc+=2) {
									RAM[tempcalc] = 0;
									RAM[tempcalc+1] = blankattr;
								}
							portram[0x3D8] = portram[0x3D8] & 0xFE;
							break;
					}
				vidmode = regs.byteregs[regal] & 0x7F;
				RAM[0x449] = vidmode;
				RAM[0x44A] = (uint8_t) cols;
				RAM[0x44B] = 0;
				RAM[0x484] = (uint8_t) (rows - 1);
				vm.renderer.setCursorPosition(0, 0);
				if ( (regs.byteregs[regal] & 0x80) == 0x00) {
						MemUtils::memset (&RAM[0xA0000], 0, 0x1FFFF);
						MemUtils::memset (VRAM, 0, VRAMSize);
					}
				switch (vidmode) {
						case 127: //hercules
							vm.renderer.markScreenModeChanged(720, 348);
							break;
						case 0x12:
							vm.renderer.markScreenModeChanged(640, 480);
							break;
						case 0x13:
						case 0x4:
						case 0x5:
						case 0xD:
							vm.renderer.markScreenModeChanged(320, 200);
							break;
						default:
							vm.renderer.markScreenModeChanged(640, 400);
							break;
					}
				break;
			case 0x10: //VGA DAC functions
				switch (regs.byteregs[regal]) {
						case 0x10: //set individual DAC register
							paletteVGA.set(getreg16(regbx), (regs.byteregs[regdh] & 63) << 2, (regs.byteregs[regch] & 63) << 2, (regs.byteregs[regcl] & 63) << 2);
							//palettevga[getreg16 (regbx) ] = rgb((regs.byteregs[regdh] & 63) << 2, (regs.byteregs[regch] & 63) << 2, (regs.byteregs[regcl] & 63) << 2);
							break;
						case 0x12: //set block of DAC registers
							memloc = segregs[reges]*16+getreg16 (regdx);
							for (n=getreg16 (regbx); n< (uint32_t) (getreg16 (regbx) +getreg16 (regcx) ); n++) 
							{
								paletteVGA.set(n, vm.memory.readByte(memloc) << 2, vm.memory.readByte(memloc + 1) << 2, vm.memory.readByte(memloc + 2) << 2);
								//palettevga[n] = rgb(vm.memory.readByte(memloc) << 2, vm.memory.readByte(memloc + 1) << 2, vm.memory.readByte(memloc + 2) << 2);
								memloc += 3;
							}
					}
				break;
			case 0x1A: //get display combination code (ps, vga/mcga)
				regs.byteregs[regal] = 0x1A;
				regs.byteregs[regbl] = 0x8;
				break;
		}
}

Video::Video(VM& inVM)
	: vm(inVM)
{
	if (vm.config.asciiFile && vm.config.asciiFile->isValid())
	{
		fontcga = new uint8_t[FontSize];
		vm.config.asciiFile->read(fontcga, FontSize);
	}
	else
	{
		// Error!
	}

	vm.ports.setPortRedirector(0x3B0, 0x3DA, this);

	currentPalette = &paletteCGA;

	paletteCGA.set(0, 0, 0, 0);
	paletteCGA.set(1, 0, 0, 0xAA);
	paletteCGA.set(2, 0, 0xAA, 0);
	paletteCGA.set(3, 0, 0xAA, 0xAA);
	paletteCGA.set(4, 0xAA, 0, 0);
	paletteCGA.set(5, 0xAA, 0, 0xAA);
	paletteCGA.set(6, 0xAA, 0x55, 0);
	paletteCGA.set(7, 0xAA, 0xAA, 0xAA);
	paletteCGA.set(8, 0x55, 0x55, 0x55);
	paletteCGA.set(9, 0x55, 0x55, 0xFF);
	paletteCGA.set(10, 0x55, 0xFF, 0x55);
	paletteCGA.set(11, 0x55, 0xFF, 0xFF);
	paletteCGA.set(12, 0xFF, 0x55, 0x55);
	paletteCGA.set(13, 0xFF, 0x55, 0xFF);
	paletteCGA.set(14, 0xFF, 0xFF, 0x55);
	paletteCGA.set(15, 0xFF, 0xFF, 0xFF);

	paletteVGA.set(0, 0, 0, 0);
	paletteVGA.set(1, 0, 0, 169);
	paletteVGA.set(2, 0, 169, 0);
	paletteVGA.set(3, 0, 169, 169);
	paletteVGA.set(4, 169, 0, 0);
	paletteVGA.set(5, 169, 0, 169);
	paletteVGA.set(6, 169, 169, 0);
	paletteVGA.set(7, 169, 169, 169);
	paletteVGA.set(8, 0, 0, 84);
	paletteVGA.set(9, 0, 0, 255);
	paletteVGA.set(10, 0, 169, 84);
	paletteVGA.set(11, 0, 169, 255);
	paletteVGA.set(12, 169, 0, 84);
	paletteVGA.set(13, 169, 0, 255);
	paletteVGA.set(14, 169, 169, 84);
	paletteVGA.set(15, 169, 169, 255);
	paletteVGA.set(16, 0, 84, 0);
	paletteVGA.set(17, 0, 84, 169);
	paletteVGA.set(18, 0, 255, 0);
	paletteVGA.set(19, 0, 255, 169);
	paletteVGA.set(20, 169, 84, 0);
	paletteVGA.set(21, 169, 84, 169);
	paletteVGA.set(22, 169, 255, 0);
	paletteVGA.set(23, 169, 255, 169);
	paletteVGA.set(24, 0, 84, 84);
	paletteVGA.set(25, 0, 84, 255);
	paletteVGA.set(26, 0, 255, 84);
	paletteVGA.set(27, 0, 255, 255);
	paletteVGA.set(28, 169, 84, 84);
	paletteVGA.set(29, 169, 84, 255);
	paletteVGA.set(30, 169, 255, 84);
	paletteVGA.set(31, 169, 255, 255);
	paletteVGA.set(32, 84, 0, 0);
	paletteVGA.set(33, 84, 0, 169);
	paletteVGA.set(34, 84, 169, 0);
	paletteVGA.set(35, 84, 169, 169);
	paletteVGA.set(36, 255, 0, 0);
	paletteVGA.set(37, 255, 0, 169);
	paletteVGA.set(38, 255, 169, 0);
	paletteVGA.set(39, 255, 169, 169);
	paletteVGA.set(40, 84, 0, 84);
	paletteVGA.set(41, 84, 0, 255);
	paletteVGA.set(42, 84, 169, 84);
	paletteVGA.set(43, 84, 169, 255);
	paletteVGA.set(44, 255, 0, 84);
	paletteVGA.set(45, 255, 0, 255);
	paletteVGA.set(46, 255, 169, 84);
	paletteVGA.set(47, 255, 169, 255);
	paletteVGA.set(48, 84, 84, 0);
	paletteVGA.set(49, 84, 84, 169);
	paletteVGA.set(50, 84, 255, 0);
	paletteVGA.set(51, 84, 255, 169);
	paletteVGA.set(52, 255, 84, 0);
	paletteVGA.set(53, 255, 84, 169);
	paletteVGA.set(54, 255, 255, 0);
	paletteVGA.set(55, 255, 255, 169);
	paletteVGA.set(56, 84, 84, 84);
	paletteVGA.set(57, 84, 84, 255);
	paletteVGA.set(58, 84, 255, 84);
	paletteVGA.set(59, 84, 255, 255);
	paletteVGA.set(60, 255, 84, 84);
	paletteVGA.set(61, 255, 84, 255);
	paletteVGA.set(62, 255, 255, 84);
	paletteVGA.set(63, 255, 255, 255);
	paletteVGA.set(64, 255, 125, 125);
	paletteVGA.set(65, 255, 157, 125);
	paletteVGA.set(66, 255, 190, 125);
	paletteVGA.set(67, 255, 222, 125);
	paletteVGA.set(68, 255, 255, 125);
	paletteVGA.set(69, 222, 255, 125);
	paletteVGA.set(70, 190, 255, 125);
	paletteVGA.set(71, 157, 255, 125);
	paletteVGA.set(72, 125, 255, 125);
	paletteVGA.set(73, 125, 255, 157);
	paletteVGA.set(74, 125, 255, 190);
	paletteVGA.set(75, 125, 255, 222);
	paletteVGA.set(76, 125, 255, 255);
	paletteVGA.set(77, 125, 222, 255);
	paletteVGA.set(78, 125, 190, 255);
	paletteVGA.set(79, 125, 157, 255);
	paletteVGA.set(80, 182, 182, 255);
	paletteVGA.set(81, 198, 182, 255);
	paletteVGA.set(82, 218, 182, 255);
	paletteVGA.set(83, 234, 182, 255);
	paletteVGA.set(84, 255, 182, 255);
	paletteVGA.set(85, 255, 182, 234);
	paletteVGA.set(86, 255, 182, 218);
	paletteVGA.set(87, 255, 182, 198);
	paletteVGA.set(88, 255, 182, 182);
	paletteVGA.set(89, 255, 198, 182);
	paletteVGA.set(90, 255, 218, 182);
	paletteVGA.set(91, 255, 234, 182);
	paletteVGA.set(92, 255, 255, 182);
	paletteVGA.set(93, 234, 255, 182);
	paletteVGA.set(94, 218, 255, 182);
	paletteVGA.set(95, 198, 255, 182);
	paletteVGA.set(96, 182, 255, 182);
	paletteVGA.set(97, 182, 255, 198);
	paletteVGA.set(98, 182, 255, 218);
	paletteVGA.set(99, 182, 255, 234);
	paletteVGA.set(100, 182, 255, 255);
	paletteVGA.set(101, 182, 234, 255);
	paletteVGA.set(102, 182, 218, 255);
	paletteVGA.set(103, 182, 198, 255);
	paletteVGA.set(104, 0, 0, 113);
	paletteVGA.set(105, 28, 0, 113);
	paletteVGA.set(106, 56, 0, 113);
	paletteVGA.set(107, 84, 0, 113);
	paletteVGA.set(108, 113, 0, 113);
	paletteVGA.set(109, 113, 0, 84);
	paletteVGA.set(110, 113, 0, 56);
	paletteVGA.set(111, 113, 0, 28);
	paletteVGA.set(112, 113, 0, 0);
	paletteVGA.set(113, 113, 28, 0);
	paletteVGA.set(114, 113, 56, 0);
	paletteVGA.set(115, 113, 84, 0);
	paletteVGA.set(116, 113, 113, 0);
	paletteVGA.set(117, 84, 113, 0);
	paletteVGA.set(118, 56, 113, 0);
	paletteVGA.set(119, 28, 113, 0);
	paletteVGA.set(120, 0, 113, 0);
	paletteVGA.set(121, 0, 113, 28);
	paletteVGA.set(122, 0, 113, 56);
	paletteVGA.set(123, 0, 113, 84);
	paletteVGA.set(124, 0, 113, 113);
	paletteVGA.set(125, 0, 84, 113);
	paletteVGA.set(126, 0, 56, 113);
	paletteVGA.set(127, 0, 28, 113);
	paletteVGA.set(128, 56, 56, 113);
	paletteVGA.set(129, 68, 56, 113);
	paletteVGA.set(130, 84, 56, 113);
	paletteVGA.set(131, 97, 56, 113);
	paletteVGA.set(132, 113, 56, 113);
	paletteVGA.set(133, 113, 56, 97);
	paletteVGA.set(134, 113, 56, 84);
	paletteVGA.set(135, 113, 56, 68);
	paletteVGA.set(136, 113, 56, 56);
	paletteVGA.set(137, 113, 68, 56);
	paletteVGA.set(138, 113, 84, 56);
	paletteVGA.set(139, 113, 97, 56);
	paletteVGA.set(140, 113, 113, 56);
	paletteVGA.set(141, 97, 113, 56);
	paletteVGA.set(142, 84, 113, 56);
	paletteVGA.set(143, 68, 113, 56);
	paletteVGA.set(144, 56, 113, 56);
	paletteVGA.set(145, 56, 113, 68);
	paletteVGA.set(146, 56, 113, 84);
	paletteVGA.set(147, 56, 113, 97);
	paletteVGA.set(148, 56, 113, 113);
	paletteVGA.set(149, 56, 97, 113);
	paletteVGA.set(150, 56, 84, 113);
	paletteVGA.set(151, 56, 68, 113);
	paletteVGA.set(152, 80, 80, 113);
	paletteVGA.set(153, 89, 80, 113);
	paletteVGA.set(154, 97, 80, 113);
	paletteVGA.set(155, 105, 80, 113);
	paletteVGA.set(156, 113, 80, 113);
	paletteVGA.set(157, 113, 80, 105);
	paletteVGA.set(158, 113, 80, 97);
	paletteVGA.set(159, 113, 80, 89);
	paletteVGA.set(160, 113, 80, 80);
	paletteVGA.set(161, 113, 89, 80);
	paletteVGA.set(162, 113, 97, 80);
	paletteVGA.set(163, 113, 105, 80);
	paletteVGA.set(164, 113, 113, 80);
	paletteVGA.set(165, 105, 113, 80);
	paletteVGA.set(166, 97, 113, 80);
	paletteVGA.set(167, 89, 113, 80);
	paletteVGA.set(168, 80, 113, 80);
	paletteVGA.set(169, 80, 113, 89);
	paletteVGA.set(170, 80, 113, 97);
	paletteVGA.set(171, 80, 113, 105);
	paletteVGA.set(172, 80, 113, 113);
	paletteVGA.set(173, 80, 105, 113);
	paletteVGA.set(174, 80, 97, 113);
	paletteVGA.set(175, 80, 89, 113);
	paletteVGA.set(176, 0, 0, 64);
	paletteVGA.set(177, 16, 0, 64);
	paletteVGA.set(178, 32, 0, 64);
	paletteVGA.set(179, 48, 0, 64);
	paletteVGA.set(180, 64, 0, 64);
	paletteVGA.set(181, 64, 0, 48);
	paletteVGA.set(182, 64, 0, 32);
	paletteVGA.set(183, 64, 0, 16);
	paletteVGA.set(184, 64, 0, 0);
	paletteVGA.set(185, 64, 16, 0);
	paletteVGA.set(186, 64, 32, 0);
	paletteVGA.set(187, 64, 48, 0);
	paletteVGA.set(188, 64, 64, 0);
	paletteVGA.set(189, 48, 64, 0);
	paletteVGA.set(190, 32, 64, 0);
	paletteVGA.set(191, 16, 64, 0);
	paletteVGA.set(192, 0, 64, 0);
	paletteVGA.set(193, 0, 64, 16);
	paletteVGA.set(194, 0, 64, 32);
	paletteVGA.set(195, 0, 64, 48);
	paletteVGA.set(196, 0, 64, 64);
	paletteVGA.set(197, 0, 48, 64);
	paletteVGA.set(198, 0, 32, 64);
	paletteVGA.set(199, 0, 16, 64);
	paletteVGA.set(200, 32, 32, 64);
	paletteVGA.set(201, 40, 32, 64);
	paletteVGA.set(202, 48, 32, 64);
	paletteVGA.set(203, 56, 32, 64);
	paletteVGA.set(204, 64, 32, 64);
	paletteVGA.set(205, 64, 32, 56);
	paletteVGA.set(206, 64, 32, 48);
	paletteVGA.set(207, 64, 32, 40);
	paletteVGA.set(208, 64, 32, 32);
	paletteVGA.set(209, 64, 40, 32);
	paletteVGA.set(210, 64, 48, 32);
	paletteVGA.set(211, 64, 56, 32);
	paletteVGA.set(212, 64, 64, 32);
	paletteVGA.set(213, 56, 64, 32);
	paletteVGA.set(214, 48, 64, 32);
	paletteVGA.set(215, 40, 64, 32);
	paletteVGA.set(216, 32, 64, 32);
	paletteVGA.set(217, 32, 64, 40);
	paletteVGA.set(218, 32, 64, 48);
	paletteVGA.set(219, 32, 64, 56);
	paletteVGA.set(220, 32, 64, 64);
	paletteVGA.set(221, 32, 56, 64);
	paletteVGA.set(222, 32, 48, 64);
	paletteVGA.set(223, 32, 40, 64);
	paletteVGA.set(224, 44, 44, 64);
	paletteVGA.set(225, 48, 44, 64);
	paletteVGA.set(226, 52, 44, 64);
	paletteVGA.set(227, 60, 44, 64);
	paletteVGA.set(228, 64, 44, 64);
	paletteVGA.set(229, 64, 44, 60);
	paletteVGA.set(230, 64, 44, 52);
	paletteVGA.set(231, 64, 44, 48);
	paletteVGA.set(232, 64, 44, 44);
	paletteVGA.set(233, 64, 48, 44);
	paletteVGA.set(234, 64, 52, 44);
	paletteVGA.set(235, 64, 60, 44);
	paletteVGA.set(236, 64, 64, 44);
	paletteVGA.set(237, 60, 64, 44);
	paletteVGA.set(238, 52, 64, 44);
	paletteVGA.set(239, 48, 64, 44);
	paletteVGA.set(240, 44, 64, 44);
	paletteVGA.set(241, 44, 64, 48);
	paletteVGA.set(242, 44, 64, 52);
	paletteVGA.set(243, 44, 64, 60);
	paletteVGA.set(244, 44, 64, 64);
	paletteVGA.set(245, 44, 60, 64);
	paletteVGA.set(246, 44, 52, 64);
	paletteVGA.set(247, 44, 48, 64);
	paletteVGA.set(248, 0, 0, 0);
	paletteVGA.set(249, 0, 0, 0);
	paletteVGA.set(250, 0, 0, 0);
	paletteVGA.set(251, 0, 0, 0);
	paletteVGA.set(252, 0, 0, 0);
	paletteVGA.set(253, 0, 0, 0);
	paletteVGA.set(254, 0, 0, 0);
	paletteVGA.set(255, 0, 0, 0);
}

bool Video::portWriteHandler(uint16_t portnum, uint8_t value)
{
	uint8_t* portram = vm.ports.portram;
	static uint8_t oldah, oldal;
	uint8_t flip3c0 = 0;
	updatedscreen = 1;
	union CPU::_bytewordregs_& regs = vm.cpu.regs;

	switch (portnum) 
	{
		case 0x3B8: //hercules support
			if ( ( (value & 2) == 2) && (vidmode != 127) ) 
			{
				oldah = regs.byteregs[regah];
				oldal = regs.byteregs[regal];
				regs.byteregs[regah] = 0;
				regs.byteregs[regal] = 127;
				handleInterrupt();
				regs.byteregs[regah] = oldah;
				regs.byteregs[regal] = oldal;
			}
			if (value & 0x80) 
				videobase = 0xB8000;
			else 
				videobase = 0xB0000;
			break;
		case 0x3C0:
			if (flip3c0) 
			{
				flip3c0 = 0;
				portram[0x3C0] = value & 255;
				break;
			}
			else 
			{
				flip3c0 = 1;
				VGA_ATTR[portram[0x3C0]] = value & 255;
				break;
			}
		case 0x3C4: //sequence controller index
			portram[0x3C4] = value & 255;
			//if (portout16) VGA_SC[value & 255] = value >> 8;
			break;
		case 0x3C5: //sequence controller data
			VGA_SC[portram[0x3C4]] = value & 255;
			/*if (portram[0x3C4] == 2) {
			printf("VGA_SC[2] = %02X\n", value);
			}*/
			break;
		case 0x3D4: //CRT controller index
			portram[0x3D4] = value & 255;
			//if (portout16) VGA_CRTC[value & 255] = value >> 8;
			break;
		case 0x3C7: //color index register (read operations)
			latchReadPal = value & 255;
			latchReadRGB = 0;
			stateDAC = 0;
			break;
		case 0x3C8: //color index register (write operations)
			latchPal = value & 255;
			latchRGB = 0;
			tempRGB.set(0, 0, 0);
			stateDAC = 3;
			break;
		case 0x3C9: //RGB data register
			value = value & 63;
			switch (latchRGB) 
			{
				case 0: //red
					tempRGB.r = value << 2;
					break;
				case 1: //green
					tempRGB.g = value << 2;
					break;
				case 2: //blue
					tempRGB.b = value << 2;
					paletteVGA.set(latchPal, tempRGB);
					latchPal = latchPal + 1;
					break;
			}
			latchRGB = (latchRGB + 1) % 3;
			break;
		case 0x3D5: //cursor position latch

			/* weird hack to detect color monitor?
			if (portram[0x3D4] == 0xF && value >= 25)
			{
				value = 24;
			}
			*/

			VGA_CRTC[portram[0x3D4]] = value & 255;
			if (portram[0x3D4]==0xE) 
				cursorposition = (cursorposition&0xFF) | (value<<8);
			else if (portram[0x3D4]==0xF) 
				cursorposition = (cursorposition&0xFF00) |value;
			vm.renderer.setCursorPosition(cursorposition%cols, cursorposition / cols);
			if (portram[0x3D4] == 6) 
			{
				vtotal = value | ( ( (uint16_t) VGA_GC[7] & 1) << 8) | ( ( (VGA_GC[7] & 32) ? 1 : 0) << 9);
				//printf("Vertical total: %u\n", vtotal);
			}
			vgapage = ((uint32_t)VGA_CRTC[0xC] << 8) + (uint32_t)VGA_CRTC[0xD];

			break;
		case 0x3CF:
			VGA_GC[portram[0x3CE]] = value;
			break;
		default:
			portram[portnum] = value;
			break;
	}

	return true;
}

bool Video::portReadHandler(uint16_t portnum, uint8_t& outValue)
{
	switch (portnum) 
	{
		case 0x3C1:
			outValue = ( (uint8_t) VGA_ATTR[vm.ports.portram[0x3C0]]);
			return true;
		case 0x3C5:
			outValue = ( (uint8_t) VGA_SC[vm.ports.portram[0x3C4]]);
			return true;
		case 0x3D5:
			// This is a hack to detect the display as colour (not sure why)
			// If still booting and in display mode 20h or 30h, return zero cursor location
			//if ((vidmode == 0x20 || vidmode == 0x30) && (vm.ports.portram[0x3D4] == 0xE || vm.ports.portram[0x3D4] == 0xF))
			//{
			//	outValue = 0;
			//	return true;
			//}
			outValue = ( (uint8_t) VGA_CRTC[vm.ports.portram[0x3D4]]);
			return true;
		case 0x3C7: //DAC state
			outValue = (stateDAC);
			return true;
		case 0x3C8: //palette index
			outValue = (latchReadPal);
			return true;
		case 0x3C9: //RGB data register
			switch (latchReadRGB++) 
			{
				case 0: //red
					outValue = paletteVGA.colours[latchReadPal].r >> 2;
					return true;
				case 1: //green
					outValue = paletteVGA.colours[latchReadPal].g >> 2;
					return true;
				case 2: //blue
					latchReadRGB = 0;
					outValue = paletteVGA.colours[latchReadPal].b >> 2;
					latchReadPal++;
					return true;
			}
			break;
		case 0x3DA:
			outValue = (port3da);
			return true;
	}

	return false;
}

#define shiftVGA(value) {\
	for (cnt=0; cnt<(VGA_GC[3] & 7); cnt++) {\
		value = (value >> 1) | ((value & 1) << 7);\
	}\
}

#define logicVGA(curval, latchval) {\
	switch ((VGA_GC[3]>>3) & 3) {\
		   case 1: curval &= latchval; break;\
		   case 2: curval |= latchval; break;\
		   case 3: curval ^= latchval; break;\
	}\
}

void Video::writeVGA (uint32_t addr32, uint8_t value) 
{
	uint32_t planesize;
	uint8_t curval, tempand, cnt;
	updatedscreen = 1;
	planesize = 0x10000;
	//if (lastmode != VGA_GC[5] & 3) printf("write mode %u\n", VGA_GC[5] & 3);
	//lastmode = VGA_GC[5] & 3;

	if (addr32 >= planesize)
	{
		//log(Log, "%x:%x ", addr32, value);
		//return;
		addr32 &= (planesize - 1);
	}
	//addr32 = addr32 & (planesize - 1);

	switch (VGA_GC[5] & 3) { //get write mode
			case 0:
				shiftVGA (value);
				if (VGA_SC[2] & 1) {
						if (VGA_GC[1] & 1)
							if (VGA_GC[0] & 1) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[0]);
						//curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[0]);
						curval = (VGA_GC[8] & curval) | (~VGA_GC[8] & VGA_latch[0]);
						VRAM[addr32] = curval;
					}
				if (VGA_SC[2] & 2) {
						if (VGA_GC[1] & 2)
							if (VGA_GC[0] & 2) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[1]);
						//curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[1]);
						curval = (VGA_GC[8] & curval) | (~VGA_GC[8] & VGA_latch[1]);
						VRAM[addr32+planesize] = curval;
					}
				if (VGA_SC[2] & 4) {
						if (VGA_GC[1] & 4)
							if (VGA_GC[0] & 4) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[2]);
						//curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[2]);
						curval = (VGA_GC[8] & curval) | (~VGA_GC[8] & VGA_latch[2]);
						VRAM[addr32+planesize*2] = curval;
					}
				if (VGA_SC[2] & 8) {
						if (VGA_GC[1] & 8)
							if (VGA_GC[0] & 8) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[3]);
						//curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[3]);
						curval = (VGA_GC[8] & curval) | (~VGA_GC[8] & VGA_latch[3]);
						VRAM[addr32+planesize*3] = curval;
					}
				break;
			case 1:
				if (VGA_SC[2] & 1) VRAM[addr32] = VGA_latch[0];
				if (VGA_SC[2] & 2) VRAM[addr32+planesize] = VGA_latch[1];
				if (VGA_SC[2] & 4) VRAM[addr32+planesize*2] = VGA_latch[2];
				if (VGA_SC[2] & 8) VRAM[addr32+planesize*3] = VGA_latch[3];
				break;
			case 2:
				if (VGA_SC[2] & 1) {
					
					curval = value & 1 ? 0xff : 0;
					logicVGA(curval, VGA_latch[0]);
					curval = (VGA_GC[8] & curval) | (~VGA_GC[8] & VGA_latch[0]);
					VRAM[addr32] = curval;
					
					//if (VGA_GC[1] & 1)
						//	if (value & 1) curval = 255;
						//	else curval = 0;
						//else curval = value;
						//logicVGA (curval, VGA_latch[0]);
						//curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[0]);
						//VRAM[addr32] = curval;


					}
				if (VGA_SC[2] & 2) {
										
					curval = value & 2 ? 0xff : 0;
					logicVGA(curval, VGA_latch[1]);
					curval = (VGA_GC[8] & curval) | (~VGA_GC[8] & VGA_latch[1]);
					VRAM[addr32 + planesize] = curval;

						//if (VGA_GC[1] & 2)
						//	if (value & 2) curval = 255;
						//	else curval = 0;
						//else curval = value;
						//logicVGA (curval, VGA_latch[1]);
						//curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[1]);
						//
						//						VRAM[addr32+planesize] = curval;
					}
				if (VGA_SC[2] & 4) {

					curval = value & 4 ? 0xff : 0;
					logicVGA(curval, VGA_latch[2]);
					curval = (VGA_GC[8] & curval) | (~VGA_GC[8] & VGA_latch[2]);
					VRAM[addr32 + 2 * planesize] = curval;

						//if (VGA_GC[1] & 4)
						//	if (value & 4) curval = 255;
						//	else curval = 0;
						//else curval = value;
						//logicVGA (curval, VGA_latch[2]);
						//curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[2]);
						//
						//VRAM[addr32+planesize*2] = curval;
					}
				if (VGA_SC[2] & 8) {


					curval = value & 8 ? 0xff : 0;
					logicVGA(curval, VGA_latch[1]);
					curval = (VGA_GC[8] & curval) | (~VGA_GC[8] & VGA_latch[3]);
					VRAM[addr32 + 3 * planesize] = curval;

						//if (VGA_GC[1] & 8)
						//	if (value & 8) curval = 255;
						//	else curval = 0;
						//else curval = value;
						//logicVGA (curval, VGA_latch[3]);
						//curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[3]);
						//
						//VRAM[addr32+planesize*3] = curval;
					}
				break;
			case 3:
				tempand = value & VGA_GC[8];
				shiftVGA (value);
				if (VGA_SC[2] & 1) {
						if (VGA_GC[0] & 1) curval = 255;
						else curval = 0;
						//logicVGA (curval, VGA_latch[0]);
						curval = (~tempand & curval) | (tempand & VGA_latch[0]);
						VRAM[addr32] = curval;
					}
				if (VGA_SC[2] & 2) {
						if (VGA_GC[0] & 2) curval = 255;
						else curval = 0;
						//logicVGA (curval, VGA_latch[1]);
						curval = (~tempand & curval) | (tempand & VGA_latch[1]);
						VRAM[addr32+planesize] = curval;
					}
				if (VGA_SC[2] & 4) {
						if (VGA_GC[0] & 4) curval = 255;
						else curval = 0;
						//logicVGA (curval, VGA_latch[2]);
						curval = (~tempand & curval) | (tempand & VGA_latch[2]);
						VRAM[addr32+planesize*2] = curval;
					}
				if (VGA_SC[2] & 8) {
						if (VGA_GC[0] & 8) curval = 255;
						else curval = 0;
						//logicVGA (curval, VGA_latch[3]);
						curval = (~tempand & curval) | (tempand & VGA_latch[3]);
						VRAM[addr32+planesize*3] = curval;
					}
				break;
		}
}

uint8_t Video::readVGA (uint32_t addr32) 
{
	uint32_t planesize;
	planesize = 0x10000;

	VGA_latch[0] = VRAM[addr32];
	VGA_latch[1] = VRAM[addr32+planesize];
	VGA_latch[2] = VRAM[addr32+planesize*2];
	VGA_latch[3] = VRAM[addr32+planesize*3];
	if (VGA_SC[2] & 1) return (VRAM[addr32]);
	if (VGA_SC[2] & 2) return (VRAM[addr32+planesize]);
	if (VGA_SC[2] & 4) return (VRAM[addr32+planesize*2]);
	if (VGA_SC[2] & 8) return (VRAM[addr32+planesize*3]);
	return (0); //this won't be reached, but without it some compilers give a warning
}

Palette::Palette()
{
	MemUtils::memset(&colours, 0, sizeof(Palette::Entry) * 256);
}
