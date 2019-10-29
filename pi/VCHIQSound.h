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

#include <circle/interrupt.h>
#include <circle/types.h>
#include <vc4/sound/vchiqsoundbasedevice.h>
#include <vc4/vchiq/vchiqdevice.h>

namespace Faux86
{
	class Audio;
	
	class VCHIQSound : public CVCHIQSoundBaseDevice
	{
	public:
		VCHIQSound (Audio& inAudio, CVCHIQDevice *pVCHIQDevice, unsigned nSampleRate);

	protected:
		static constexpr unsigned ChunkSize = 2048;
		
		virtual unsigned GetChunk (s16 *pBuffer, unsigned nChunkSize) override;

		Audio& audio;
	};
}
