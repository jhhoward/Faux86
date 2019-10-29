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

#include "Config.h"
#include "VM.h"
#include "Audio.h"
#include "MemUtils.h"

using namespace Faux86;

/*
// TODO
struct wav_hdr_s wav_hdr;
FILE *wav_file = NULL;
*/

void Audio::createOutputWAV(char *filename) 
{
	// TODO
	/*
	log(Log, "Creating %s for audio logging... ", filename);
	wav_file = fopen (filename, "wb");
	if (wav_file == NULL) {
			log (Log, "failed!\n");
			return;
		}
	log(Log, "OK!\n");

	wav_hdr.AudioFormat = 1; //PCM
	wav_hdr.bitsPerSample = 8;
	wav_hdr.blockAlign = 1;
	wav_hdr.ChunkSize = sizeof (wav_hdr) - 4;
	memcpy (&wav_hdr.WAVE[0], "WAVE", 4);
	memcpy (&wav_hdr.fmt[0], "fmt ", 4);
	wav_hdr.NumOfChan = 1;
	wav_hdr.bytesPerSec = sampleRate * (uint32_t) (wav_hdr.bitsPerSample >> 3) * (uint32_t) wav_hdr.NumOfChan;
	memcpy (&wav_hdr.RIFF[0], "RIFF", 4);
	wav_hdr.Subchunk1Size = 16;
	wav_hdr.SamplesPerSec = sampleRate;
	memcpy (&wav_hdr.Subchunk2ID[0], "data", 4);
	wav_hdr.Subchunk2Size = 0;
	//fwrite((void *)&wav_hdr, 1, sizeof(wav_hdr), wav_file);
	*/
}

bool Audio::isAudioBufferFilled() 
{
	if (audbufptr >= usebuffersize) return true;
	return false;
}

void Audio::tick() 
{
	int16_t sample;
	if (audbufptr >= usebuffersize) return;
	sample = vm.adlib.generateSample() >> 8;
	if (vm.config.useDisneySoundSource) sample += vm.soundSource.generateSample();
	sample += vm.blaster.generateSample();
	if (vm.pcSpeaker.enabled) sample += (vm.pcSpeaker.generateSample() >> 1);
	if (audbufptr < (int) sizeof(audbuf) ) audbuf[audbufptr++] = (uint8_t) ((uint16_t) sample+128);
}

void Audio::fillAudioBuffer(uint8_t *stream, int len)
{
	MemUtils::memcpy (stream, audbuf, len);
	MemUtils::memmove (audbuf, &audbuf[len], usebuffersize - len);

	audbufptr -= len;
	if (audbufptr < 0) 
		audbufptr = 0;
}

Audio::Audio(VM& inVM)
	: vm(inVM)
{
}

void Audio::init()
{
	sampleRate = vm.config.audio.sampleRate;
	latency = vm.config.audio.latency;
	log(Log, "Initializing audio stream... ");

	audbufptr = usebuffersize = (sampleRate / 1000) * latency;
	vm.timing.gensamplerate = sampleRate;
	doublesamplecount = (uint32_t) ( (double) sampleRate * (double) 0.01);

	MemUtils::memset (audbuf, 128, sizeof (audbuf) );
	audbufptr = usebuffersize;

	vm.config.hostSystemInterface->getAudio().init(vm);
}

Audio::~Audio() 
{
	vm.config.hostSystemInterface->getAudio().shutdown();
	// TODO
	/*
	SDL_PauseAudio (1);

	if (wav_file == NULL) return;
	wav_hdr.ChunkSize = wav_hdr.Subchunk2Size + sizeof(wav_hdr) - 8;
	fseek(wav_file, 0, SEEK_SET);
	fwrite((void *)&wav_hdr, 1, sizeof(wav_hdr), wav_file);
	fclose (wav_file);
	*/
}
