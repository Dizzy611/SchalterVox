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
#define FRAMERATE (1000 / 25)
#define SAMPLECOUNT (SAMPLERATE / FRAMERATE)
#define BYTESPERSAMPLE 2
#define BUFFERSIZE (SAMPLECOUNT * CHANNELCOUNT * BYTESPERSAMPLE)

using namespace std;
	
vorbisdecoder::vorbisdecoder(const string& fileName) {
	mutexLock(&this->decodeLock);
	int lvRet = ov_fopen(fileName.c_str(), &this->vorbisFile);
	if (lvRet < 0) {
		throw "Unable to open Vorbis file. (Failed during OV_FOPEN)";
	}
	this->info = ov_info(&this->vorbisFile, -1);
	this->comment = ov_comment(&this->vorbisFile,-1);
	this->resamplerData.src_ratio = (1.0 * SAMPLERATE)/this->info->rate;
	this->resamplerData.output_frames = BUFFERSIZE/CHANNELCOUNT;
	this->decodeBufferSize = ((this->info->rate/FRAMERATE) * BYTESPERSAMPLE * this->info->channels);
	this->resamplerData.input_frames = this->decodeBufferSize/this->info->channels;
	this->resamplerData.end_of_input = 0;
	this->decodeBuffer = (s16 *) malloc(this->decodeBufferSize);
	this->resampledBuffer = (s16 *) malloc(BUFFERSIZE);
	this->decodeRunning = false;
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
	printf("Vorbis decode thread started...\n");
	
	while (running) {
		condvarWakeAll(&this->decodeStatusCV);
		mutexUnlock(&this->decodeStatusLock);

		mutexLock(&this->decodeLock);
		long retval = ov_read(&this->vorbisFile,(char *)this->decodeBuffer,this->decodeBufferSize,0,2,1,&this->section);	
		if (retval == 0) {
			//this->resamplerData.end_of_input = 1;
			mutexLock(&this->decodeStatusLock);
			condvarWait(&this->decodeStatusCV);
			this->decodeRunning = false;
			mutexUnlock(&this->decodeStatusLock);
		} else {
			
			//src_short_to_float_array(this->decodeBuffer, this->resamplingBufferIn, this->decodeBufferSize);
			//this->resamplerData.data_in = this->resamplingBufferIn;
			//this->resamplerData.data_out = this->resamplingBufferOut;
			//src_process(this->resamplerState, &this->resamplerData);			
			//src_float_to_short_array(this->resamplingBufferOut, this->resampledBuffer, BUFFERSIZE);
			int x = fillPlayBuffer(this->decodeBuffer, BUFFERSIZE);
			if (x == -1) { // buffer overflow. Wait before trying again.
				while (x == -1) {
					svcSleepThread(16000000); // 16ms, one frame.
					x = fillPlayBuffer(this->decodeBuffer, BUFFERSIZE);
				}
			}
			
		}
		mutexUnlock(&this->decodeLock);
		
		
		mutexLock(&this->decodeStatusLock);
		running = this->decodeRunning;
	}
	condvarWakeAll(&this->decodeStatusCV);
	mutexUnlock(&this->decodeStatusLock);
}