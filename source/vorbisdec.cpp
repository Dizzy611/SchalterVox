#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <switch.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <samplerate.h>
#include <cstring>
#include <malloc.h>

#include "audiofile.hpp"
#include "playback.hpp"
#include "vorbisdec.hpp"

#define SAMPLERATE 48000
#define CHANNELCOUNT 2
#define FRAMEMS 1
#define FRAMERATE (1000 / FRAMEMS)
#define SAMPLECOUNT (SAMPLERATE / FRAMERATE)
#define BYTESPERSAMPLE 2
#define BUFFERSIZE (SAMPLECOUNT * CHANNELCOUNT * BYTESPERSAMPLE)

using namespace std;
	
vorbisdecoder::vorbisdecoder(const string& fileName) {
	mutexLock(&this->decodeLock);
	int lvRet = ov_fopen(fileName.c_str(), &this->vorbisFile);
	if (lvRet < 0) {
		this->decoderValid = false;
		this->decoderError = "Error " + to_string(lvRet) + " during OV_OPEN";
		return;
	}
	this->info = ov_info(&this->vorbisFile, -1);
	if (this->info->rate != 48000) {
		this->decoderValid = false;
		this->decoderError = "Error, file is " + to_string(this->info->rate) + "Hz, can only play at 48000Hz currently.";
		return;
	}
	this->comment = ov_comment(&this->vorbisFile,-1);
	this->resamplerData.src_ratio = (1.0 * SAMPLERATE)/this->info->rate;
	this->resamplerData.output_frames = BUFFERSIZE/CHANNELCOUNT;
	this->decodeBufferSize = ((this->info->rate/FRAMERATE) * BYTESPERSAMPLE * this->info->channels);
	this->resamplerData.input_frames = this->decodeBufferSize/this->info->channels;
	this->resamplerData.end_of_input = 0;
	this->decodeBuffer = (s16 *) malloc(this->decodeBufferSize);
	this->resampledBuffer = (s16 *) malloc(BUFFERSIZE);
	this->decodeRunning = false;
	this->decoderValid = true;
	mutexUnlock(&this->decodeLock);
}
		
vorbisdecoder::~vorbisdecoder() {
	mutexLock(&this->decodeLock);
	this->decodeRunning = false;
	free(this->decodeBuffer);
	free(this->resampledBuffer);
	ov_clear(&this->vorbisFile);
	mutexUnlock(&this->decodeLock);
}

void vorbisdecoder::start() {
	this->decodeRunning = true;
	threadCreate(&this->decodingThread, vorbisdecoder_trampoline, this, 0x10000, 0x2D, 3);
	threadStart(&this->decodingThread);
}

void vorbisdecoder::stop() {
	mutexLock(&this->decodeStatusLock);
	condvarWait(&this->decodeStatusCV);
	this->decodeRunning = false;
	mutexUnlock(&this->decodeStatusLock);
	threadWaitForExit(&this->decodingThread);
}

bool vorbisdecoder::checkRunning() {
	mutexLock(&this->decodeStatusLock);
	condvarWait(&this->decodeStatusCV);
	bool tmp = this->decodeRunning;
	mutexUnlock(&this->decodeStatusLock);
	return tmp;
}

void vorbisdecoder_trampoline(void *parameter) {
	vorbisdecoder* obj = (vorbisdecoder*)parameter;
	obj->main_thread(NULL);
}

void vorbisdecoder::main_thread(void *) {
	mutexLock(&this->decodeStatusLock);
	this->decodeRunning=true;
	bool running=this->decodeRunning;
	printf("Vorbis decode thread started, filling buffer...\n");
	
	while (running) {
		condvarWakeAll(&this->decodeStatusCV);
		mutexUnlock(&this->decodeStatusLock);

		mutexLock(&this->decodeLock);
		long retval = ov_read(&this->vorbisFile,(char *)this->decodeBuffer,this->decodeBufferSize,0,2,1,&this->section);	
		if (retval == 0) {
			printf("Vorbis reached EOF.\n");
			//this->resamplerData.end_of_input = 1;
			mutexLock(&this->decodeStatusLock);
			
			this->decodeRunning = false;
			condvarWakeAll(&this->decodeStatusCV);
			mutexUnlock(&this->decodeStatusLock);
		} else {
			
			//src_short_to_float_array(this->decodeBuffer, this->resamplingBufferIn, this->decodeBufferSize);
			//this->resamplerData.data_in = this->resamplingBufferIn;
			//this->resamplerData.data_out = this->resamplingBufferOut;
			//src_process(this->resamplerState, &this->resamplerData);			
			//src_float_to_short_array(this->resamplingBufferOut, this->resampledBuffer, BUFFERSIZE);
			int x = fillPlayBuffer(this->decodeBuffer, retval/2);
			if (x == -1) { // buffer overflow. Wait before trying again.
				while (x == -1) {
					svcSleepThread((FRAMEMS * 1000000)/4);
					x = fillPlayBuffer(this->decodeBuffer, retval/2);
				}
			}
			
		}
		mutexUnlock(&this->decodeLock);
		
		svcSleepThread(1000);
		
		mutexLock(&this->decodeStatusLock);
		running = this->decodeRunning;
	}
	condvarWakeAll(&this->decodeStatusCV);
	mutexUnlock(&this->decodeStatusLock);
}