#include <switch.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <malloc.h>
#include <cmath>
#include "audiofile.hpp"
#include "playback.hpp"

#define SAMPLERATE 48000
#define CHANNELCOUNT 2
#define FRAMEMS 1
#define FRAMERATE (1000 / FRAMEMS)
#define SAMPLECOUNT (SAMPLERATE / FRAMERATE)
#define BYTESPERSAMPLE 2
#define BUFFERSIZE (SAMPLECOUNT * CHANNELCOUNT * BYTESPERSAMPLE)

using namespace std;

	
stubdecoder::stubdecoder(const string& fileName) {
	mutexLock(&this->decodeLock);
	// some sort of fopen should be done here, and information important to audioFile should be populated at the least.
	int rate=48000; // this should be grabbed from the file
	inr channels=2; // as should this
	this->decodeBufferSize = (rate/FRAMERATE) * BYTESPERSAMPLE * channels);
	this->decodeBuffer = (s16 *) malloc(this->decodeBufferSize);
	this->decodeRunning = false;
	this->decoderValid = true; // set this to false if any step fails to validate
	mutexUnlock(&this->decodeLock);
}
		
stubdecoder::~stubdecoder() {
	mutexLock(&this->decodeLock);
	this->decodeRunning = false;
	free(this->decodeBuffer);
    // be sure to close your file handles here
	mutexUnlock(&this->decodeLock);
}

void stubdecoder::start() { // this function should stay pretty much as is
	this->decodeRunning = true;
	threadCreate(&this->decodingThread, stubdecoder_trampoline, this, 0x10000, 0x2D, 3);
	threadStart(&this->decodingThread);
}

void stubdecoder::stop() { // this function should stay pretty much as is
	mutexLock(&this->decodeStatusLock);
	condvarWait(&this->decodeStatusCV);
	this->decodeRunning = false;
	mutexUnlock(&this->decodeStatusLock);
	threadWaitForExit(&this->decodingThread);
}

long stubdecoder::tell() {
	// this should return the current position in terms of PCM samples.
	return 0;
}

long stubdecoder::tell_time() {
	// this should return the current position in terms of seconds.
	return 0;
}

long stubdecoder::length() {
	// this should return the total length of the file in terms of PCM samples.
	return 0; 
}

long stubdecoder::length_time() {
	// this should return the total length of the file in terms of seconds.
	return 0;
}

int stubdecoder::seek(long position) {
	// this should seek to a certain position in terms of PCM samples.
	return 1;
}

int stubdecoder::seek_time(double time) {
	// this should seek to a certain position in terms of seconds.
	return 1;
}

int stubdecoder::get_bitrate() {
	// this should give a bitrate, or a VBR quality. Numbers 10 and below are 
	// assumed by AudioFile to be some sort of VBR quality and well be treated 
	// accordingly.
	return 0;
}

bool stubdecoder::checkRunning() { // this function should stay pretty much as is
	mutexLock(&this->decodeStatusLock);
	condvarWaitTimeout(&this->decodeStatusCV,100000);
	bool tmp = this->decodeRunning;
	this->Metadata.currtime = floor(this->tell_time());
	mutexUnlock(&this->decodeStatusLock);
	return tmp;
}

void stubdecoder_trampoline(void *parameter) { // this function should stay pretty much as is
	stubdecoder* obj = (stubdecoder*)parameter;
	obj->main_thread(NULL);
}

void stubdecoder::main_thread(void *) { // this function can be heavily modified if necessary
	mutexLock(&this->decodeStatusLock);
	this->decodeRunning=true;
	bool running=this->decodeRunning;
	
	while (running) {
		condvarWakeAll(&this->decodeStatusCV);
		mutexUnlock(&this->decodeStatusLock);

		mutexLock(&this->decodeLock);
		long retval = 0; // this should read a maximum of this->decodeBufferSize
                         // bytes into this->decodeBuffer, and return the amount 
                         // of bytes actually read, or a negative number on an error.
		if (retval == 0) { // a 0 should mean EOF.
			mutexLock(&this->decodeStatusLock);
			this->decodeRunning = false;
			condvarWakeAll(&this->decodeStatusCV);
			mutexUnlock(&this->decodeStatusLock);
		} else if (retval < 0) { // this is where you should check for failures in decode
			mutexLock(&this->decodeStatusLock);
			this->decodeRunning = false;
			condvarWakeAll(&this->decodeStatusCV);
			mutexUnlock(&this->decodeStatusLock);
			this->decoderError = "Decode error " + to_string(retval);
		} else {
			int x = fillPlayBuffer(this->decodeBuffer, retval/2);
			if (x == -1) { // buffer overflow. Wait before trying again.
				while (x == -1) {
					svcSleepThread((FRAMEMS * 1000000)/4);
					x = fillPlayBuffer(this->decodeBuffer, retval/2);
				}
			}
		}
		mutexUnlock(&this->decodeLock);
		
		svcSleepThread(1000); //1000ns sleep just to reduce load on CPU. can be removed if necessary.
		
		mutexLock(&this->decodeStatusLock);
		running = this->decodeRunning;
	}
	condvarWakeAll(&this->decodeStatusCV);
	mutexUnlock(&this->decodeStatusLock);
}