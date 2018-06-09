#include <cstring>
#include <switch.h>
#include <malloc.h>
#include "playback.hpp"
#include <cstdio>

void start_playback() {
	atb = (u32 *)malloc(ATB_SIZE * sizeof(u32));
	mutexInit(&aLock);
	
	audoutInitialize();
	audoutStartAudioOut();
	
	playing = true;
	threadCreate(&playback_thread, playback_thread_main, NULL, 0x10000, 0x2B, 2);
	threadStart(&playback_thread);
}

void stop_playback() {
	playing = false;
	threadWaitForExit(&playback_thread);
	audoutStopAudioOut();
	audoutExit();
}

void playback_thread_main(void *) {
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
	
	while (playing) {
		u32 cnt;
		AudioOutBuffer *released;
		audoutWaitPlayFinish(&released, &cnt, U64_MAX);
		
		mutexLock(&aLock);
		u32 size;
		if (atbUsed < AUDIO_BUFFER_SAMPLES) {
			size = atbUsed * sizeof(u32);
		} else {
			size = AUDIO_BUFFER_SAMPLES * sizeof(u32);
		}
		memcpy(released->buffer, atb, size);
		if (size == 0) {
			released->data_size = AUDIO_BUFFER_SAMPLES * sizeof(u32);
		} else {
			released->data_size = size;
		}
		atbUsed -= size / sizeof(u32);
		memmove(atb, atb + (size / sizeof(u32)), atbUsed * sizeof(u32));
		
		mutexUnlock(&aLock);
		
		audoutAppendAudioOutBuffer(released);
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