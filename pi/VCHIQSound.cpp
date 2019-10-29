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
#include "../src/faux86/Audio.h"
#include "VCHIQSound.h"

using namespace Faux86;

VCHIQSound::VCHIQSound (Audio& inAudio, CVCHIQDevice *pVCHIQDevice, unsigned nSampleRate)
	: CVCHIQSoundBaseDevice(pVCHIQDevice, nSampleRate, ChunkSize)
	, audio(inAudio)
{
}

unsigned VCHIQSound::GetChunk (s16 *pBuffer, unsigned nChunkSize)
{
	uint8_t generatedAudio[ChunkSize / 2];
	int numSamples = (int) nChunkSize / 2;
	audio.fillAudioBuffer(generatedAudio, numSamples);
	
	for(int n = 0; n < numSamples; n++)
	{
		unsigned sample = generatedAudio[n];
		
		sample -= 128;			// unsigned (8 bit) -> signed (16 bit)
		sample *= 256;
		
		pBuffer[n * 2] = sample;
		pBuffer[n * 2 + 1] = sample;
	}
	
	return nChunkSize;
}
