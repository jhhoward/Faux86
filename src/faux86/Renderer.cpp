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
#include "Renderer.h"
#include "Profiler.h"

using namespace Faux86;

Mutex Renderer::screenMutex;

void setwindowtitle (const char *extra) 
{
	// TODO
	//char temptext[128];
	//sprintf (temptext, "%s%s", windowtitle, extra);
	//SDL_WM_SetCaption ( (const char *) temptext, NULL);
}

// TODO
//inline uint32_t mapRGB(uint8_t b, uint8_t g, uint8_t r)
//{
//	return (r | (g << 8) | (b << 16) | (0xff << 24));
//}

void Renderer::markScreenModeChanged(uint32_t newWidth, uint32_t newHeight)
{
	screenModeChanged = true;
	nativeWidth = newWidth;
	nativeHeight = newHeight;
}

Renderer::Renderer(VM& inVM)
	: vm(inVM)
{
	scalemap = nullptr;
}

Renderer::~Renderer()
{
	delete[] scalemap;
}

void Renderer::init()
{
	fb = &vm.config.hostSystemInterface->getFrameBuffer();
	fb->init(OUTPUT_FRAMEBUFFER_WIDTH, OUTPUT_FRAMEBUFFER_HEIGHT);
	createScaleMap();

	//sprintf (windowtitle, "%s", BUILD_STRING);
	//setwindowtitle ("");

	InitMutex(screenMutex);


	vm.taskManager.addTask(new RenderTask(*this));
}

void Renderer::createScaleMap()
{
	log(LogVerbose, "Creating scale map");
	if (!scalemap)
	{
		scalemap = new uint32_t[(fb->getWidth() + 1) * fb->getHeight()];
	}

	uint32_t srcx, srcy, dstx, dsty, scalemapptr;
	double xscale, yscale;

	xscale = (double) nativeWidth / (double) fb->getWidth();
	yscale = (double) nativeHeight / (double) fb->getHeight();
	scalemapptr = 0;
	for (dsty=0; dsty<(uint32_t)fb->getHeight(); dsty++) 
	{
		srcy = (uint32_t) ( (double) dsty * yscale);
		scalemap[scalemapptr++] = srcy;
		for (dstx=0; dstx<(uint32_t)fb->getWidth(); dstx++) 
		{
			srcx = (uint32_t) ( (double) dstx * xscale);
			scalemap[scalemapptr++] = srcx;
		}
	}
}

void RenderTask::begin()
{
	cursorprevtick = (uint32_t) vm.timing.getMS();
	vm.video.cursorvisible = 0;
}

int RenderTask::update()
{
	//ProfileBlock block(vm.timing, "RenderTask::update");

	cursorcurtick = (uint32_t)vm.timing.getMS();
	if ((cursorcurtick - cursorprevtick) >= 250)
	{
		vm.video.updatedscreen = 1;
		vm.video.cursorvisible = ~vm.video.cursorvisible & 1;
		cursorprevtick = cursorcurtick;
	}

	uint64_t drawStartTime = vm.timing.getTicks();

	//if (vm.video.updatedscreen || vm.config.renderBenchmark)
	//{
	//	vm.video.updatedscreen = 0;
	//	if (renderer.fb != nullptr)
	//	{
	//		renderer.draw();
	//	}
	//	renderer.totalframes++;
	//}

	if (vm.video.updatedscreen)
	{
		renderer.draw();
		renderer.fb->setPalette(vm.video.getCurrentPalette());
		vm.video.updatedscreen = false;
	}

	constexpr int targetTime = 16;		// 16 ms
	int delayTime = targetTime - (int)vm.timing.getElapsedMS(drawStartTime);

	if (delayTime < 1)
		delayTime = 1;
	if (delayTime > targetTime)
		delayTime = targetTime;

	return delayTime;
	

	//if (!vm.config.renderBenchmark)
	//{
	//	delaycalc = vm.config.frameDelay - (uint32_t)(vm.timing.getMS() - cursorcurtick);
	//	if (delaycalc > vm.config.frameDelay) 
	//		delaycalc = vm.config.frameDelay;
	//	return delaycalc;
	//}
	//
	//
	//return 0;
}

void Renderer::stretchBlit () 
{
	// TODO
	/*
	uint32_t srcx, srcy, dstx, dsty, lastx, lasty, r, g, b;
	uint32_t consecutivex, consecutivey = 0, limitx, limity, scalemapptr;
	uint32_t ofs;
	uint8_t *pixelrgb;

	limitx = (uint32_t)((double) nativeWidth / (double) fb->getWidth());
	limity = (uint32_t)((double) nativeHeight / (double) fb->getHeight());

	if (!fb->lock())
		return;

	lasty = 0;
	scalemapptr = 0;
	for (dsty=0; dsty<(uint32_t)fb->getHeight(); dsty++) {
			srcy = scalemap[scalemapptr++];
			ofs = dsty*fb->getPitch();
			consecutivex = 0;
			lastx = 0;
			if (srcy == lasty) consecutivey++;
			else consecutivey = 0;
			for (dstx=0; dstx<(uint32_t)fb->getWidth(); dstx++) {
					srcx = scalemap[scalemapptr++];
					pixelrgb = (uint8_t *) &prestretch[srcy][srcx];
					r = pixelrgb[0];
					g = pixelrgb[1];
					b = pixelrgb[2];
					if (srcx == lastx) consecutivex++;
					else consecutivex = 0;
					if ( (consecutivex > limitx) && (consecutivey > limity) ) {
							pixelrgb = (uint8_t *) &prestretch[srcy][srcx+1];
							r += pixelrgb[0];
							g += pixelrgb[1];
							b += pixelrgb[2];
							pixelrgb = (uint8_t *) &prestretch[srcy+1][srcx];
							r += pixelrgb[0];
							g += pixelrgb[1];
							b += pixelrgb[2];
							pixelrgb = (uint8_t *) &prestretch[srcy+1][srcx+1];
							r += pixelrgb[0];
							g += pixelrgb[1];
							b += pixelrgb[2];
							r = r >> 2;
							g = g >> 2;
							b = b >> 2;
							//r = 255; g = 0; b = 0;
						}
					else if (consecutivex > limitx) {
							pixelrgb = (uint8_t *) &prestretch[srcy][srcx+1];
							r += pixelrgb[0];
							r = r >> 1;
							g += pixelrgb[1];
							g = g >> 1;
							b += pixelrgb[2];
							b = b >> 1;
							//r = 0; g = 255; b = 0;
						}
					else if (consecutivey > limity) {
							pixelrgb = (uint8_t *) &prestretch[srcy+1][srcx];
							r += pixelrgb[0];
							r = r >> 1;
							g += pixelrgb[1];
							g = g >> 1;
							b += pixelrgb[2];
							b = b >> 1;
							//r = 0; g = 0; b = 255;
						}
					fb->getPixels() [ofs++] = mapRGB ((uint8_t) r, (uint8_t) g, (uint8_t) b);
					lastx = srcx;
				}
			lasty = srcy;
		}

	fb->unlock();
	*/
	roughBlit();
}

void Renderer::simpleBlit()
{
	if (!fb->lock())
		return;

	uint8_t* pixels = fb->getPixels();
	uint32_t pitch = fb->getPitch();
	uint32_t width = fb->getWidth();
	uint32_t height = fb->getHeight();

	for (uint32_t y = 0; y < height; y++)
	{
		for (uint32_t x = 0; x < width; x++)
		{
			pixels[y * pitch + x] = prestretch[y][x];
		}
	}

	fb->unlock();
}

void Renderer::roughBlit () 
{
	if (!fb->lock())
		return;


	uint32_t srcx, srcy, dstx, dsty, scalemapptr;
	uint8_t* pixels = fb->getPixels();
	uint32_t pitch = fb->getPitch();
	uint32_t width = fb->getWidth();
	uint32_t height = fb->getHeight();

	{
		scalemapptr = 0;
		for (dsty = 0; dsty < height; dsty++)
		{
			srcy = scalemap[scalemapptr++];
			uint8_t* dstPtr = pixels + dsty * pitch;

			for (dstx = 0; dstx < width; dstx++)
			{
				srcx = scalemap[scalemapptr++];
				*dstPtr++ = prestretch[srcy][srcx];
			}
		}
	}


	fb->unlock();
}

/* NOTE: doubleblit is only used when smoothing is not enabled, and the SDL window size
         is exactly double of native resolution for the current video mode. we can take
         advantage of the fact that every pixel is simply doubled both horizontally and
         vertically. this way, we do not need to waste mountains of CPU time doing
         floating point multiplication for each and every on-screen pixel. it makes the
         difference between games being smooth and playable, and being jerky on my old
         400 MHz PowerPC G3 iMac.
*/
void Renderer::doubleBlit () 
{
	uint32_t srcx, srcy, dstx, dsty, curcolor;
	int32_t ofs;
	uint8_t* pixels = fb->getPixels();
	uint32_t pitch = fb->getPitch();
	uint32_t width = fb->getWidth();
	uint32_t height = fb->getHeight();

	if(!fb->lock())
		return;

	for (dsty=0; dsty<height; dsty += 2) 
	{
		srcy = (uint32_t) (dsty >> 1);
		ofs = dsty * pitch;
		for (dstx=0; dstx < width; dstx += 2) 
		{
			srcx = (uint32_t) (dstx >> 1);
			curcolor = prestretch[srcy][srcx];
			pixels [ofs+pitch] = curcolor;
			pixels [ofs++] = curcolor;
			pixels [ofs+pitch] = curcolor;
			pixels [ofs++] = curcolor;
		}
	}

	fb->unlock();
}

void Renderer::draw () 
{
	//ProfileBlock block(vm.timing, "Renderer::draw");

	if (screenModeChanged)
	{
		createScaleMap();
		screenModeChanged = false;
	}

	{
		//ProfileBlock innerblock(vm.timing, "Renderer::draw inner");

	

	uint8_t* RAM = vm.memory.RAM;
	uint8_t* portram = vm.ports.portram;
	uint32_t planemode, vgapage, chary, charx, vidptr, divx, divy, curchar, curpixel, usepal, intensity, blockw, curheight, x1, y1;
	uint8_t color;
	uint32_t x, y;
	switch (vm.video.vidmode) {
			case 0:
			case 1:
			case 2: //text modes
			case 3:
			case 7:
			case 0x82:
				nativeWidth = 640;
				nativeHeight = 400;
				vgapage = ( (uint32_t) vm.video.VGA_CRTC[0xC]<<8) + (uint32_t)vm.video.VGA_CRTC[0xD];
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							if (vm.video.cols==80) {
									charx = x/8;
									divx = 1;
								}
							else {
									charx = x/16;
									divx = 2;
								}
							if ( (portram[0x3D8]==9) && (portram[0x3D4]==9) ) {
									chary = y/4;
									vidptr = vgapage + vm.video.videobase + chary*vm.video.cols*2 + charx*2;
									curchar = RAM[vidptr];
									color = vm.video.fontcga[curchar*128 + (y%4) *8 + ( (x/divx) %8) ];
								}
							else {
									chary = y/16;
									vidptr = vm.video.videobase + chary*vm.video.cols*2 + charx*2;
									curchar = RAM[vidptr];
									color = vm.video.fontcga[curchar*128 + (y%16) *8 + ( (x/divx) %8) ];
								}
							if (vm.video.vidcolor) {
									/*if (!color) if (portram[0x3D8]&128) color = palettecga[ (RAM[vidptr+1]/16) &7];
										else*/ if (!color) color = RAM[vidptr+1]/16; //high intensity background
									else color = RAM[vidptr+1]&15;
								}
							else {
									if ( (RAM[vidptr+1] & 0x70) ) {
											if (!color) color = 7;
											else color = 0;
										}
									else {
											if (!color) color = 0;
											else color = 7;
										}
								}
							prestretch[y][x] = color;
						}
				break;
			case 4:
			case 5:
				nativeWidth = 320;
				nativeHeight = 200;
				usepal = (portram[0x3D9]>>5) & 1;
				intensity = ( (portram[0x3D9]>>4) & 1) << 3;
				for (y=0; y<200; y++) {
						for (x=0; x<320; x++) {
								charx = x;
								chary = y;
								vidptr = vm.video.videobase + ( (chary>>1) * 80) + ( (chary & 1) * 8192) + (charx >> 2);
								curpixel = RAM[vidptr];
								switch (charx & 3) {
										case 3:
											curpixel = curpixel & 3;
											break;
										case 2:
											curpixel = (curpixel>>2) & 3;
											break;
										case 1:
											curpixel = (curpixel>>4) & 3;
											break;
										case 0:
											curpixel = (curpixel>>6) & 3;
											break;
									}
								if (vm.video.vidmode==4) {
										curpixel = curpixel * 2 + usepal + intensity;
										if (curpixel == (usepal + intensity) )  curpixel = vm.video.cgabg;
										color = curpixel;
										prestretch[y][x] = color;
									}
								else {
										curpixel = curpixel * 63;
										color = curpixel;
										prestretch[y][x] = color;
									}
							}
					}
				break;
			case 6:
				nativeWidth = 640;
				nativeHeight = 200;
				for (y=0; y<400; y+=2) {
						for (x=0; x<640; x++) {
								charx = x;
								chary = y >> 1;
								vidptr = vm.video.videobase + ( (chary>>1) * 80) + ( (chary&1) * 8192) + (charx>>3);
								curpixel = (RAM[vidptr]>> (7- (charx&7) ) ) &1;
								color = curpixel*15;
								prestretch[y][x] = color;
								prestretch[y+1][x] = color;
							}
					}
				break;
			case 127:
				nativeWidth = 720;
				nativeHeight = 348;
				for (y=0; y<348; y++) {
						for (x=0; x<720; x++) {
								charx = x;
								chary = y>>1;
								vidptr = vm.video.videobase + ( (y & 3) << 13) + (y >> 2) *90 + (x >> 3);
								curpixel = (RAM[vidptr]>> (7- (charx&7) ) ) &1;
#ifdef __BIG_ENDIAN__
								if (curpixel) color = 0xFF;
#else
								if (curpixel) color = 0xff;
#endif
								else color = 0x00;
								prestretch[y][x] = color;
							}
					}
				break;
			case 0x8: //160x200 16-color (PCjr)
				nativeWidth = 640; //fix this
				nativeHeight = 400; //part later
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							vidptr = 0xB8000 + (y>>2) *80 + (x>>3) + ( (y>>1) &1) *8192;
							if ( ( (x>>1) &1) ==0) color = RAM[vidptr] >> 4;
							else color = RAM[vidptr] & 15;
							prestretch[y][x] = color;
						}
				break;
			case 0x9: //320x200 16-color (Tandy/PCjr)
				nativeWidth = 640; //fix this
				nativeHeight = 400; //part later
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							vidptr = 0xB8000 + (y>>3) *160 + (x>>2) + ( (y>>1) &3) *8192;
							if ( ( (x>>1) &1) ==0) color = RAM[vidptr] >> 4;
							else color = RAM[vidptr] & 15;
							prestretch[y][x] = color;
						}
				break;
			case 0xD:
			case 0xE:
				nativeWidth = 640; //fix this
				nativeHeight = 400; //part later
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							divx = x>>1;
							divy = y>>1;
							vidptr = divy*40 + (divx>>3);
							x1 = 7 - (divx & 7);
							color = (vm.video.VRAM[vidptr] >> x1) & 1;
							color += ( ( (vm.video.VRAM[0x10000 + vidptr] >> x1) & 1) << 1);
							color += ( ( (vm.video.VRAM[0x20000 + vidptr] >> x1) & 1) << 2);
							color += ( ( (vm.video.VRAM[0x30000 + vidptr] >> x1) & 1) << 3);
							prestretch[y][x] = color;
						}
				break;
			case 0x10:
				nativeWidth = 640;
				nativeHeight = 350;
				for (y=0; y<350; y++)
					for (x=0; x<640; x++) {
							vidptr = y*80 + (x>>3);
							x1 = 7 - (x & 7);
							color = (vm.video.VRAM[vidptr] >> x1) & 1;
							color |= ( ( (vm.video.VRAM[0x10000 + vidptr] >> x1) & 1) << 1);
							color |= ( ( (vm.video.VRAM[0x20000 + vidptr] >> x1) & 1) << 2);
							color |= ( ( (vm.video.VRAM[0x30000 + vidptr] >> x1) & 1) << 3);
							prestretch[y][x] = color;
						}
				break;
			case 0x12:
				nativeWidth = 640;
				nativeHeight = 480;
				vgapage = ( (uint32_t)vm.video.VGA_CRTC[0xC]<<8) + (uint32_t)vm.video.VGA_CRTC[0xD];
				for (y=0; y<nativeHeight; y++)
					for (x=0; x<nativeWidth; x++) {
							vidptr = y*80 + (x/8);
							color  = (vm.video.VRAM[vidptr] >> (~x & 7) ) & 1;
							color |= ( (vm.video.VRAM[vidptr+0x10000] >> (~x & 7) ) & 1) << 1;
							color |= ( (vm.video.VRAM[vidptr+0x20000] >> (~x & 7) ) & 1) << 2;
							color |= ( (vm.video.VRAM[vidptr+0x30000] >> (~x & 7) ) & 1) << 3;
							prestretch[y][x] = color;
						}
				break;
			case 0x13:
				if (vm.video.vtotal == 11) { //ugly hack to show Flashback at the proper resolution
						nativeWidth = 256;
						nativeHeight = 224;
					}
				else {
						nativeWidth = 320;
						nativeHeight = 200;
					}
				if (vm.video.VGA_SC[4] & 6) planemode = 1;
				else planemode = 0;
				vgapage = ( (uint32_t)vm.video.VGA_CRTC[0xC]<<8) + (uint32_t)vm.video.VGA_CRTC[0xD];
				//vgapage = (( (uint32_t) VGA_CRTC[0xC]<<8) + (uint32_t) VGA_CRTC[0xD]) << 2;
				for (y=0; y<nativeHeight; y++)
					for (x=0; x<nativeWidth; x++) {
							if (!planemode) color = RAM[vm.video.videobase + ((vgapage + y*nativeWidth + x) & 0xFFFF) ];
							//if (!planemode) color = palettevga[RAM[videobase + y*nw + x]];
							else {
									vidptr = y*nativeWidth + x;
									vidptr = vidptr/4 + (x & 3) *0x10000;
									vidptr = vidptr + vgapage - (vm.video.VGA_ATTR[0x13] & 15);
									color = vm.video.VRAM[vidptr];
								}
							prestretch[y][x] = color;
						}
		}

	if (vm.video.vidgfxmode==0) {
			if (vm.video.cursorvisible) {
					curheight = 2;
					if (vm.video.cols==80) blockw = 8;
					else blockw = 16;
					x1 = vm.video.cursx * blockw;
					y1 = vm.video.cursy * 8 + 8 - curheight;
					for (y=y1*2; y<=y1*2+curheight-1; y++)
						for (x=x1; x<=x1+blockw-1; x++) {
								color = RAM[vm.video.videobase+ vm.video.cursy*vm.video.cols*2+ vm.video.cursx*2+1]&15;
								prestretch[y&1023][x&1023] = color;
							}
				}
		}

	}

	if (vm.config.noSmooth) 
	{
		if (nativeWidth == fb->getWidth() && nativeHeight == fb->getHeight())
			simpleBlit();
		if ( ((nativeWidth << 1) == fb->getWidth()) && ((nativeHeight << 1) == fb->getHeight()) ) 
			doubleBlit ();
		else 
			roughBlit ();
	}
	else stretchBlit ();
}

