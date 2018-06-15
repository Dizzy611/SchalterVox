// This code heavily derived from vba-next-switch sound.cpp.
#include <cstring>
#include <switch.h>
#include <malloc.h>
#include "playback.hpp"
#include <cstdio>
#include <cmath>

static int atbUsed = 0;
static u32 *atb;
static Mutex aLock;
static bool playing=false;
static bool flushing=false;
static Thread playback_thread;
static Mutex aStatusLock;
static CondVar aStatusCV;
static int current_samplerate=AUDIO_SAMPLERATE;

void start_playback() {
	atb = (u32 *)malloc(ATB_SIZE * sizeof(u32));
	mutexInit(&aLock);
	mutexInit(&aStatusLock);
	condvarInit(&aStatusCV, &aStatusLock);
	audoutInitialize();
	audoutStartAudioOut();
	
	playing = true;
	threadCreate(&playback_thread, playback_thread_main, NULL, 0x10000, 0x2B, 2);
	threadStart(&playback_thread);
}

void set_samplerate(int sr) {
	mutexLock(&aLock);
	current_samplerate = sr;
	mutexUnlock(&aLock);
}

void stop_playback(bool playout) {
	// TODO: Fix playout for resampled audio. Currently causes freeze.
	//       Suspected causes include buffer overflow (shouldn't be 
	//       possible given the size of atb) or strange/undefined behavior 
	//       at low atbUsed values (due to mismatch between buffer size, 
	//		 which is based on device samplerate, and amount of buffer 
	//       consumed, in a pass, based on input samplerate.
	//		 Unsure how to fix atm, and it's better to just stub out 
	//       the functionality (given the small buffer sizes playout 
	//       isn't noticeable anyway) than have people testing the
	//       release end up corrupting their filesystems from a hard freeze
	//       with open handles, as has happened to be repeatedly now.
	if (playout && (current_samplerate==AUDIO_SAMPLERATE)) {
		bool flsh = false;
		while (!flsh) { // Wait for the buffer to empty before terminating.
			mutexLock(&aStatusLock);
			condvarWait(&aStatusCV);
			flsh = flushing;
			mutexUnlock(&aStatusLock);
		}
		u64 sleepnano = 4000000000/AUDIO_BUFFER_DIVIDER; // Sleep for 4x the system buffer size, to drain all buffers.
		svcSleepThread(sleepnano);
		printf("DEBUG: Slept.\n");
	}
	
	printf("DEBUG: Waiting to signal audio to quit...\n");
	mutexLock(&aStatusLock);
	condvarWait(&aStatusCV);
	printf("DEBUG: Signalling audio to quit...\n");
	playing = false;
	mutexUnlock(&aStatusLock);
	printf("DEBUG: Waiting for thread to exit...\n");
	threadWaitForExit(&playback_thread);
	printf("DEBUG: Thread exited, waiting for audio to stop...\n");
	audoutStopAudioOut();
	audoutExit();
	printf("DEBUG: Audio stopped. leaving stop_playback()\n");
}

void abort_playback() { // Used for errors.
	mutexLock(&aStatusLock);
	condvarWait(&aStatusCV);
	playing = false;
	mutexUnlock(&aStatusLock);
	threadWaitForExit(&playback_thread);
	audoutStopAudioOut();
	audoutExit();
}

void playback_thread_main(void *) {
	bool waiting = true;
	while (waiting) {
		svcSleepThread(1000);
		if (atbUsed > (ATB_SIZE/2)) {
			waiting = false;
		}
		mutexLock(&aStatusLock);
		bool p = playing;
		condvarWakeAll(&aStatusCV);
		mutexUnlock(&aStatusLock);
		if (!p) break;
	}
	AudioOutBuffer sources[4];	
	
	u32 rdata_size = (AUDIO_BUFFER_SAMPLES * sizeof(u32) + 0xfff) & ~0xfff;
	u32 *rdata[4];
	for (int i = 0; i < 4; i++) {
		rdata[i] = (u32 *)memalign(0x1000, rdata_size);
		memset(rdata[i], 0, rdata_size);
		sources[i].next = 0;
		sources[i].buffer = rdata[i];
		sources[i].buffer_size = rdata_size;
		sources[i].data_size = AUDIO_BUFFER_SAMPLES * sizeof(u32);
		sources[i].data_offset = 0;
		
		audoutAppendAudioOutBuffer(&sources[i]);
	}
	mutexLock(&aStatusLock);
	bool p = playing;
	flushing = false;
	while (p) {
		condvarWakeAll(&aStatusCV);
		mutexUnlock(&aStatusLock);
		
		u32 cnt;
		AudioOutBuffer *released;
		audoutWaitPlayFinish(&released, &cnt, U64_MAX);

		mutexLock(&aLock);		
		if (atbUsed > 0) {
			mutexLock(&aStatusLock);
			flushing = false;
			condvarWakeAll(&aStatusCV);
			mutexUnlock(&aStatusLock);
			u32 size;
			
			if (atbUsed < AUDIO_BUFFER_SAMPLES) {
				size = atbUsed * sizeof(u32);
			} else {
				size = AUDIO_BUFFER_SAMPLES * sizeof(u32);
			}
			
			if (AUDIO_SAMPLERATE == current_samplerate) { // If we're matched samplerate, just blast right into the switch.
				memcpy(released->buffer, atb, size);
				if (size == 0) {
					released->data_size = AUDIO_BUFFER_SAMPLES * sizeof(u32);
				} else {
					released->data_size = size;
				}
				atbUsed -= size / sizeof(u32);
				memmove(atb, atb + (size / sizeof(u32)), atbUsed * sizeof(u32));
			} else {
				resampleBuffer((short*)released->buffer, (size / sizeof(u32)));
				int outsize = (size * current_samplerate) / (sizeof(u32) * AUDIO_SAMPLERATE); 
				atbUsed -= outsize;
				if (atbUsed<0) atbUsed=0;
				memmove(atb, atb + outsize, atbUsed * sizeof(u32));
			}
			audoutAppendAudioOutBuffer(released);
		} else {
			mutexLock(&aStatusLock);
			flushing = true;
			condvarWakeAll(&aStatusCV);
			mutexUnlock(&aStatusLock);
			u32 size;
			size = AUDIO_BUFFER_SAMPLES * sizeof(u32);
			memset(released->buffer, 0, size);
			released->data_size = size;
			audoutAppendAudioOutBuffer(released);
			svcSleepThread(3500); // Give main thread time to signal shutdown
		}
		mutexUnlock(&aLock);
		
		mutexLock(&aStatusLock);
		p = playing;
	}
	
	free(rdata[0]);
	free(rdata[1]);
}

int fillPlayBuffer(int16_t *inBuffer, int length) {
	mutexLock(&aLock);
	if (atbUsed + length >= ATB_SIZE) {
		mutexUnlock(&aLock);
		return -1;
	}
	memcpy(atb + atbUsed, inBuffer, length * sizeof(s16));
	atbUsed += length / 2;
	mutexUnlock(&aLock);
	return 0;
}


void resampleBuffer(short* out, int numSamples) {
	short *s_atb = reinterpret_cast<short *>(atb);
	unsigned long long m_mixPtr=0;
	int resampleDelta=((current_samplerate*0x40000ull)/AUDIO_SAMPLERATE);
	
	for(int i=0; i<numSamples; i++) {
		
		int idx = m_mixPtr >> 18;
		int s1 = s_atb[(idx)*2];
		int s2 = s_atb[(idx+1)*2];
		int interp = m_mixPtr & 0x3ffff;
		long long int sRamp = s2-s1;
		sRamp *= interp;
		sRamp >>= 18;
		sRamp += s1;
		if(sRamp > 32767) sRamp = 32767;
		if(sRamp < -32768) sRamp = -32768;
		out[(i*2)] = (short)sRamp;
		
		s1 = s_atb[(idx*2)+1];
		s2 = s_atb[((idx+1)*2)+1];
		sRamp = s2-s1;
		sRamp *= interp;
		sRamp >>= 18;
		sRamp += s1;
		if(sRamp > 32767) sRamp = 32767;
		if(sRamp < -32768) sRamp = -32768;
		out[((i*2)+1)] = (short)sRamp;
		
		m_mixPtr += resampleDelta;
	}
}
