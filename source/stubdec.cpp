// This file and the accompanying header are stubs intended
// to show how to program a decoder to work with SchalterVox.
#include <switch.h>
#include <string>
#include <cstring>
#include <malloc.h>
#include <cmath>
#include "audiofile.hpp"
#include "playback.hpp"
#include "stubdec.hpp"

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
	// the file should be opened and metadata structures populated here
	this->parse_metadata();
	
	// substitute the samplerate of your file, the bytes per sample
	// and the channels for 48000, 2, and 2 here.
	this->decodeBufferSize = ((48000/FRAMERATE) * 2 * 2);
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

int stubdecoder::seek(long position) {
	// this should seek to a certain position in terms of PCM samples.
	return 1;
}

int stubdecoder::seek_time(double time) {
	// this should seek to a certain position in terms of seconds.
	return 1;
}

bool stubdecoder::check_running() { // this function should stay pretty much as is
	mutexLock(&this->decodeStatusLock);
	condvarWaitTimeout(&this->decodeStatusCV,100000);
	bool tmp = this->decodeRunning;
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

		this->update_metadata(); // This should update e.g. the current time and any 
		                         // other variables that have changed. Do as little 
						         // as possible to avoid crashes.		
		mutexLock(&this->decodeStatusLock);
		running = this->decodeRunning;
	}
	condvarWakeAll(&this->decodeStatusCV);
	mutexUnlock(&this->decodeStatusLock);
}

void stubdecoder::parse_metadata() {
	metadata_t m = this->metadata;
	// Populate the metadata_t struct. See metadata.hpp
	this->set_metadata(m, true);
}

void stubdecoder::update_metadata() {
	metadata_t m = this->metadata;
	// Update parts of the metadata_t struct that have changed.
	this->set_metadata(m, false);
}
