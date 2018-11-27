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
#include "MemUtils.h"

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
	refreshTextMode();
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
	if (renderSurface && renderSurface != hostSurface)
	{
		RenderSurface::destroy(renderSurface);
	}
}

void Renderer::init()
{
	fb = &vm.config.hostSystemInterface->getFrameBuffer();

	fb->init(OUTPUT_FRAMEBUFFER_WIDTH, OUTPUT_FRAMEBUFFER_HEIGHT);
	//fb->init(320, 200);
	//fb->init(1920, 1080);
	//fb->init(1024, 1024);

	hostSurface = fb->getSurface();

#ifdef DOUBLE_BUFFER
	renderSurface = RenderSurface::create(1024, 1024);
#else
	renderSurface = hostSurface;
#endif

	createScaleMap();

	refreshTextMode();

	//sprintf (windowtitle, "%s", BUILD_STRING);
	//setwindowtitle ("");

	InitMutex(screenMutex);


	vm.taskManager.addTask(new RenderTask(*this));
}

void Renderer::refreshTextMode()
{
	for (unsigned n = 0; n < MaxColumns * MaxRows; n++)
	{
		textModeDirtyFlag[n] = 1;
	}
}

void Renderer::createScaleMap()
{
	log(LogVerbose, "Creating scale map");
	if (!scalemap)
	{
		scalemap = new uint32_t[(hostSurface->width + 1) * hostSurface->height];
	}

	uint32_t srcx, srcy, dstx, dsty, scalemapptr;
	double xscale, yscale;

	xscale = (double) nativeWidth / (double) hostSurface->width;
	yscale = (double) nativeHeight / (double) hostSurface->height;
	scalemapptr = 0;
	for (dsty=0; dsty<(uint32_t)hostSurface->height; dsty++) 
	{
		srcy = (uint32_t) ( (double) dsty * yscale);
		scalemap[scalemapptr++] = srcy;
		for (dstx=0; dstx<(uint32_t)hostSurface->width; dstx++) 
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

	// Blink cursor
	cursorcurtick = (uint32_t)vm.timing.getMS();
	if ((cursorcurtick - cursorprevtick) >= 250)
	{
		vm.video.updatedscreen = 1;
		vm.video.cursorvisible = ~vm.video.cursorvisible & 1;
		cursorprevtick = cursorcurtick;
		
		vm.renderer.markTextDirty(vm.renderer.cursorX, vm.renderer.cursorY);
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

void Renderer::setCursorPosition(uint32_t x, uint32_t y)
{
	markTextDirty(cursorX, cursorY);
	cursorX = x;
	cursorY = y;
	markTextDirty(cursorX, cursorY);
}

void Renderer::stretchBlit () 
{
	// TODO
	/*
	uint32_t srcx, srcy, dstx, dsty, lastx, lasty, r, g, b;
	uint32_t consecutivex, consecutivey = 0, limitx, limity, scalemapptr;
	uint32_t ofs;
	uint8_t *pixelrgb;

	limitx = (uint32_t)((double) nativeWidth / (double) hostSurface->width);
	limity = (uint32_t)((double) nativeHeight / (double) hostSurface->height);

	if (!fb->lock())
		return;

	lasty = 0;
	scalemapptr = 0;
	for (dsty=0; dsty<(uint32_t)hostSurface->height; dsty++) {
			srcy = scalemap[scalemapptr++];
			ofs = dsty*hostSurface->pitch;
			consecutivex = 0;
			lastx = 0;
			if (srcy == lasty) consecutivey++;
			else consecutivey = 0;
			for (dstx=0; dstx<(uint32_t)hostSurface->width; dstx++) {
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
					hostSurface->pixels [ofs++] = mapRGB ((uint8_t) r, (uint8_t) g, (uint8_t) b);
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
	//ProfileBlock block(vm.timing, "Renderer::simpleBlit");

	{
		//ProfileBlock block(vm.timing, "Renderer::simpleBlit inner");

		for (uint32_t y = 0; y < hostSurface->height; y++)
		{
			MemUtils::memcpy(hostSurface->pixels + (y * hostSurface->pitch), renderSurface->pixels + (y * renderSurface->pitch), hostSurface->width);
		}
	}
}

void Renderer::roughBlit () 
{
	uint32_t srcx, srcy, dstx, dsty, scalemapptr;
	uint8_t* pixels = hostSurface->pixels;
	uint32_t pitch = hostSurface->pitch;
	uint32_t width = hostSurface->width;
	uint32_t height = hostSurface->height;

	{
		scalemapptr = 0;
		for (dsty = 0; dsty < height; dsty++)
		{
			srcy = scalemap[scalemapptr++];
			uint8_t* dstPtr = pixels + dsty * pitch;

			for (dstx = 0; dstx < width; dstx++)
			{
				srcx = scalemap[scalemapptr++];
				*dstPtr++ = renderSurface->get(srcx, srcy);
			}
		}
	}
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
	uint8_t* pixels = hostSurface->pixels;
	uint32_t pitch = hostSurface->pitch;
	uint32_t width = hostSurface->width;
	uint32_t height = hostSurface->height;

	for (dsty=0; dsty<height; dsty += 2) 
	{
		srcy = (uint32_t) (dsty >> 1);
		ofs = dsty * pitch;
		for (dstx=0; dstx < width; dstx += 2) 
		{
			srcx = (uint32_t) (dstx >> 1);
			curcolor = renderSurface->get(srcx, srcy);
			pixels [ofs+pitch] = curcolor;
			pixels [ofs++] = curcolor;
			pixels [ofs+pitch] = curcolor;
			pixels [ofs++] = curcolor;
		}
	}
}

void Renderer::onMemoryWrite(uint32_t address, uint8_t value)
{
	if (!vm.video.vidgfxmode)
	{
		uint32_t base = vm.video.vgapage + vm.video.videobase;

		if (address >= base && address < base + vm.video.cols * vm.video.rows * 2)
		{
			textModeDirtyFlag[(address - base) / 2] = 1;
		}
	}
	else
	{
		/*switch (vm.video.vidmode)
		{
			case 0x13:	// 320x200 256 colour
			{
				bool isPlaneMode = (vm.video.VGA_SC[4] & 6) != 0;

				if (!isPlaneMode)
				{
					uint32_t relAddress = address - vm.video.videobase - vm.video.vgapage;
					if (relAddress < 320 * 200)
					{
						uint32_t y = relAddress / 320;
						uint32_t x = relAddress % 320;
						renderSurface->set(x, y, value);
					}
				}
				else
				{
					// TODO
				}
			}
			break;
		}*/
	}

}

void Renderer::renderTextMode()
{
	uint32_t glyphWidth = 640 / vm.video.cols;
	uint32_t glyphHeight = 400 / vm.video.rows;
	uint32_t outX = 0, outY = 0;
	uint8_t* fontData = vm.video.fontcga;
	uint8_t* RAM = vm.memory.RAM;

	for (uint32_t row = 0; row < vm.video.rows; row++)
	{
		for (uint32_t col = 0; col < vm.video.cols; col++)
		{
			bool isDirty = textModeDirtyFlag[row * vm.video.cols + col] != 0;

			if (isDirty)
			{
				textModeDirtyFlag[row * vm.video.cols + col] = 0;

				uint32_t vidptr = vm.video.vgapage + vm.video.videobase + row * vm.video.cols * 2 + col * 2;
				uint8_t curchar = RAM[vidptr];

				for (uint32_t j = 0; j < glyphHeight; j++)
				{
					for (uint32_t i = 0; i < glyphWidth; i++)
					{
						uint32_t glyphRow = j * 16 / glyphHeight;
						uint32_t glyphCol = i * 8 / glyphWidth;
						uint8_t glyphData = fontData[curchar * 128 + glyphRow * 8 + glyphCol];
						uint8_t color;

						if (vm.video.vidcolor)
						{
							if (!glyphData)
								color = RAM[vidptr + 1] / 16; //high intensity background
							else
								color = RAM[vidptr + 1] & 15;
						}
						else
						{
							if ((RAM[vidptr + 1] & 0x70))
							{
								if (!glyphData)
									color = 7;
								else
									color = 0;
							}
							else
							{
								if (!glyphData)
									color = 0;
								else
									color = 7;
							}
						}
						renderSurface->set(outX, outY, color);
						outX++;
					}
					outX -= glyphWidth;
					outY++;
				}
				outY -= glyphHeight;
			}

			outX += glyphWidth;
		}
		outX = 0;
		outY += glyphHeight;
	}

	// Draw cursor
	if (vm.video.cursorvisible && cursorX < vm.video.cols && cursorY < vm.video.rows) 
	{
		uint32_t curheight = 2;
		uint32_t x1 = cursorX * glyphWidth;
		uint32_t y1 = cursorY * 8 + 8 - curheight;
		for (uint32_t y = y1 * 2; y <= y1 * 2 + curheight - 1; y++)
		{
			for (uint32_t x = x1; x <= x1 + glyphWidth - 1; x++)
			{
				uint8_t color = RAM[vm.video.videobase + cursorY * vm.video.cols * 2 + cursorX * 2 + 1] & 15;
				renderSurface->set(x, y, color);
			}
		}
	}
}

void Renderer::markTextDirty(uint32_t x, uint32_t y)
{
	if (x < MaxColumns && y < MaxRows)
	{
		textModeDirtyFlag[y * vm.video.cols + x] = 1;
	}
}

void Renderer::draw () 
{
	//ProfileBlock block(vm.timing, "Renderer::draw");

	if (screenModeChanged)
	{
		fb->resize(nativeWidth, nativeHeight);
		createScaleMap();
		screenModeChanged = false;
	}

	{
		//ProfileBlock innerblock(vm.timing, "Renderer::draw inner");



		uint8_t* RAM = vm.memory.RAM;
		uint8_t* portram = vm.ports.portram;
		uint32_t planemode, chary, charx, vidptr, curpixel, usepal, intensity, x1;
		uint8_t color;
		uint32_t x, y;
		switch (vm.video.vidmode) {
		case 0:
		case 1:
		case 2: //text modes
		case 3:
		case 7:
		case 0x82:
			assert(nativeWidth == 640 && nativeHeight == 400);
			//nativeWidth = 640;
			//nativeHeight = 400;
			renderTextMode();
			break;
		case 4:
		case 5:
			assert(nativeWidth == 320 && nativeHeight == 200);
			//nativeWidth = 320;
			//nativeHeight = 200;
			usepal = (portram[0x3D9] >> 5) & 1;
			intensity = ((portram[0x3D9] >> 4) & 1) << 3;
			for (y = 0; y < 200; y++) {
				for (x = 0; x < 320; x++) {
					charx = x;
					chary = y;
					vidptr = vm.video.videobase + ((chary >> 1) * 80) + ((chary & 1) * 8192) + (charx >> 2);
					curpixel = RAM[vidptr];
					switch (charx & 3) {
					case 3:
						curpixel = curpixel & 3;
						break;
					case 2:
						curpixel = (curpixel >> 2) & 3;
						break;
					case 1:
						curpixel = (curpixel >> 4) & 3;
						break;
					case 0:
						curpixel = (curpixel >> 6) & 3;
						break;
					}
					if (vm.video.vidmode == 4) {
						curpixel = curpixel * 2 + usepal + intensity;
						if (curpixel == (usepal + intensity))  curpixel = vm.video.cgabg;
						color = curpixel;
						renderSurface->set(x, y, color);
					}
					else {
						curpixel = curpixel * 63;
						color = curpixel;
						renderSurface->set(x, y, color);
					}
				}
			}
			break;
		case 6:
			assert(nativeWidth == 640 && nativeHeight == 400);
			//nativeWidth = 640;
			//nativeHeight = 200;
			for (y = 0; y < 400; y += 2) {
				for (x = 0; x < 640; x++) {
					charx = x;
					chary = y >> 1;
					vidptr = vm.video.videobase + ((chary >> 1) * 80) + ((chary & 1) * 8192) + (charx >> 3);
					curpixel = (RAM[vidptr] >> (7 - (charx & 7))) & 1;
					color = curpixel * 15;
					renderSurface->set(x, y, color);
					renderSurface->set(x, y + 1, color);
				}
			}
			break;
		case 127:
			assert(nativeWidth == 720 && nativeHeight == 348);
			// nativeWidth = 720;
			// nativeHeight = 348;
			for (y = 0; y < 348; y++) {
				for (x = 0; x < 720; x++) {
					charx = x;
					chary = y >> 1;
					vidptr = vm.video.videobase + ((y & 3) << 13) + (y >> 2) * 90 + (x >> 3);
					curpixel = (RAM[vidptr] >> (7 - (charx & 7))) & 1;
					if (curpixel)
						color = 0xf;
					else
						color = 0x00;
					renderSurface->set(x, y, color);
				}
			}
			break;
		case 0x8: //160x200 16-color (PCjr)
			assert(nativeWidth == 640 && nativeHeight == 400);
			// nativeWidth = 640; //fix this
			// nativeHeight = 400; //part later
			for (y = 0; y < 400; y++)
				for (x = 0; x < 640; x++) {
					vidptr = 0xB8000 + (y >> 2) * 80 + (x >> 3) + ((y >> 1) & 1) * 8192;
					if (((x >> 1) & 1) == 0) color = RAM[vidptr] >> 4;
					else color = RAM[vidptr] & 15;
					renderSurface->set(x, y, color);
				}
			break;
		case 0x9: //320x200 16-color (Tandy/PCjr)
			assert(nativeWidth == 640 && nativeHeight == 400);
			// nativeWidth = 640; //fix this
			// nativeHeight = 400; //part later
			for (y = 0; y < 400; y++)
				for (x = 0; x < 640; x++) {
					vidptr = 0xB8000 + (y >> 3) * 160 + (x >> 2) + ((y >> 1) & 3) * 8192;
					if (((x >> 1) & 1) == 0) color = RAM[vidptr] >> 4;
					else color = RAM[vidptr] & 15;
					renderSurface->set(x, y, color);
				}
			break;
		case 0xD:
			assert(nativeWidth == 320 && nativeHeight == 200);
			// nativeWidth = 320;
			// nativeHeight = 200;
			for (y = 0; y < 200; y++)
				for (x = 0; x < 320; x++) {
					vidptr = y * 40 + (x >> 3);
					x1 = 7 - (x & 7);
					color = (vm.video.VRAM[vidptr] >> x1) & 1;
					color += (((vm.video.VRAM[0x10000 + vidptr] >> x1) & 1) << 1);
					color += (((vm.video.VRAM[0x20000 + vidptr] >> x1) & 1) << 2);
					color += (((vm.video.VRAM[0x30000 + vidptr] >> x1) & 1) << 3);
					renderSurface->set(x, y, color);
				}
			break;
		case 0xE:
			break;
		case 0x10:
			assert(nativeWidth == 640 && nativeHeight == 350);
			// nativeWidth = 640;
			// nativeHeight = 350;
			for (y = 0; y < 350; y++)
				for (x = 0; x < 640; x++) {
					vidptr = y * 80 + (x >> 3);
					x1 = 7 - (x & 7);
					color = (vm.video.VRAM[vidptr] >> x1) & 1;
					color |= (((vm.video.VRAM[0x10000 + vidptr] >> x1) & 1) << 1);
					color |= (((vm.video.VRAM[0x20000 + vidptr] >> x1) & 1) << 2);
					color |= (((vm.video.VRAM[0x30000 + vidptr] >> x1) & 1) << 3);
					renderSurface->set(x, y, color);
				}
			break;
		case 0x12:
			assert(nativeWidth == 640 && nativeHeight == 480);
			// nativeWidth = 640;
			// nativeHeight = 480;
			for (y = 0; y < nativeHeight; y++)
				for (x = 0; x < nativeWidth; x++) {
					vidptr = y * 80 + (x / 8);
					color = (vm.video.VRAM[vidptr] >> (~x & 7)) & 1;
					color |= ((vm.video.VRAM[vidptr + 0x10000] >> (~x & 7)) & 1) << 1;
					color |= ((vm.video.VRAM[vidptr + 0x20000] >> (~x & 7)) & 1) << 2;
					color |= ((vm.video.VRAM[vidptr + 0x30000] >> (~x & 7)) & 1) << 3;
					renderSurface->set(x, y, color);
				}
			break;
		case 0x13:
			assert(nativeWidth == 320 && nativeHeight == 200);
			if (vm.video.vtotal == 11) { //ugly hack to show Flashback at the proper resolution
				//nativeWidth = 256;
				//nativeHeight = 224;
			}
			else {
				//nativeWidth = 320;
				//nativeHeight = 200;
			}
			if (vm.video.VGA_SC[4] & 6) 
				planemode = 1;
			else 
				planemode = 0;

			if (!planemode)
			{
				if (renderSurface->pitch == nativeWidth)
				{
					MemUtils::memcpy(renderSurface->pixels, &RAM[vm.video.videobase + ((vm.video.vgapage) & 0xFFFF)], nativeWidth * nativeHeight);
				}
				else
				{
					for (y = 0; y < nativeHeight; y++)
					{
						MemUtils::memcpy(&renderSurface->pixels[y * renderSurface->pitch], &RAM[vm.video.videobase + ((vm.video.vgapage + y*nativeWidth) & 0xFFFF)], nativeWidth);
					}
				}
			}
			else
			{
				for (y = 0; y < nativeHeight; y++)
				{
					for (x = 0; x < nativeWidth; x++)
					{
						vidptr = y*nativeWidth + x;
						vidptr = vidptr / 4 + (x & 3) * 0x10000;
						vidptr = vidptr + vm.video.vgapage - (vm.video.VGA_ATTR[0x13] & 15);
						color = vm.video.VRAM[vidptr];
						renderSurface->set(x, y, color);
					}
				}
			}
		}
	}

	if (renderSurface != hostSurface)
	{
		if (vm.config.noSmooth)
		{
			if (nativeWidth == hostSurface->width && nativeHeight == hostSurface->height)
				simpleBlit();
			else if (((nativeWidth << 1) == hostSurface->width) && ((nativeHeight << 1) == hostSurface->height))
				doubleBlit();
			else
				roughBlit();
		}
		else stretchBlit();
	}

	fb->present();
}

RenderSurface* RenderSurface::create(uint32_t inWidth, uint32_t inHeight)
{
	RenderSurface* newSurface = new RenderSurface();
	newSurface->width = newSurface->pitch = inWidth;
	newSurface->height = inHeight;
	newSurface->pixels = new uint8_t[inWidth * inHeight];
	return newSurface;
}

void RenderSurface::destroy(RenderSurface* surface)
{
	delete[] surface->pixels;
	delete surface;
}

