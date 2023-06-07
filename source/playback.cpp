// This code heavily derived from vba-next-switch sound.cpp.
#include <cstring>
#include <switch.h>
#include <malloc.h>
#include "playback.hpp"

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
	atb = (u32 *)malloc(ATB_SIZE * sizeof(u32)); // Allocate audio buffer
	mutexInit(&aLock); // Lock audio mutex
	mutexInit(&aStatusLock); // Lock tag mutex
	condvarInit(&aStatusCV, &aStatusLock); // Tie audio status mutex state to audio status conditional variable
	audoutInitialize(); // Initialize output state
	audoutStartAudioOut(); // Start output
	
	playing = true;
	threadCreate(&playback_thread, playback_thread_main, NULL, 0x10000, 0x2B, 2); // Create playback thread
	threadStart(&playback_thread); // Start playback thread
}

void set_samplerate(int sr) { // Lock audio mutex, set samplerate, unlock mutex.
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
        // ED:   Is this bug even still active? I thought I'd fixed resampling.
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
	}
	// Lock the audio status mutex, wait until it's ready to be unlocked, set playing to false,
        //  wait for the playback thread to exit, then stop the audio output
	mutexLock(&aStatusLock);
	condvarWait(&aStatusCV);
	playing = false;
	mutexUnlock(&aStatusLock);
	threadWaitForExit(&playback_thread);
	audoutStopAudioOut();
	audoutExit();
}

void abort_playback() { // Used for errors. Essentially does stop_playback without the flushing. Should maybe combine the two functions.
	mutexLock(&aStatusLock);
	condvarWait(&aStatusCV);
	playing = false;
	mutexUnlock(&aStatusLock);
	threadWaitForExit(&playback_thread);
	audoutStopAudioOut();
	audoutExit();
}

void playback_thread_main(void *) { // Main playback thread
	bool waiting = true;
	while (waiting) { // Wait until at least half the buffer is unused before adding more to it.
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
	
	u32 rdata_size = (AUDIO_BUFFER_SAMPLES * sizeof(u32) + 0xfff) & ~0xfff; // Set buffer size for raw data. Not sure why the + 0xfff & ~0xfff? Extra space, but aligned to 0x1000 I guess.
	u32 *rdata[4]; // Also not sure why 4 buffers. Presumably this is just how the switch prefers its audio?
	for (int i = 0; i < 4; i++) {
		rdata[i] = (u32 *)memalign(0x1000, rdata_size); // Allocate 4kb of aligned memory for each buffer
		memset(rdata[i], 0, rdata_size); // Clear this buffer
		sources[i].next = 0; 
		sources[i].buffer = rdata[i]; // Also clear and set the size of the audio output buffers.
		sources[i].buffer_size = rdata_size; 
		sources[i].data_size = AUDIO_BUFFER_SAMPLES * sizeof(u32);
		sources[i].data_offset = 0;
		
		audoutAppendAudioOutBuffer(&sources[i]); // Add all 4 buffers to the audio output
	}
	mutexLock(&aStatusLock);
	bool p = playing;
	flushing = false;
	while (p) {
		condvarWakeAll(&aStatusCV); // Wait until the Audio Status thread is ready to be unlocked, then unlock it
		mutexUnlock(&aStatusLock);
		
		u32 cnt; // Count of released buffers
		AudioOutBuffer *released;
		audoutWaitPlayFinish(&released, &cnt, U64_MAX); // Wait for playback to finish of current buffers, get a count of how many are available.

		mutexLock(&aLock); // Lock the audio mutex
		if (atbUsed > 0) {
			mutexLock(&aStatusLock); // Lock the audio status mutex, say that we're no longer flushing buffers, unlock it.
			flushing = false;
			condvarWakeAll(&aStatusCV);
			mutexUnlock(&aStatusLock);
			u32 size;
			
			if (atbUsed < AUDIO_BUFFER_SAMPLES) { // If the internal buffer is less than the max number of buffer samples per the samplerate, only send what's there.
				size = atbUsed * sizeof(u32);
			} else {
				size = AUDIO_BUFFER_SAMPLES * sizeof(u32); // If it's not, send all samples for the (48000Hz on the switch) samplerate.
			}
			
			if (AUDIO_SAMPLERATE == current_samplerate) { // If we're matched samplerate, just blast right into the switch.
				memcpy(released->buffer, atb, size); // Copy from the internal buffer into the released Switch buffers.
				if (size == 0) { // Not sure why this is here? Should be covered by the above if/else, size should never be 0.
					released->data_size = AUDIO_BUFFER_SAMPLES * sizeof(u32);
				} else {
					released->data_size = size;
				} // Regardless, set the data size of the released buffer to the amount of samples we just pushed.
				atbUsed -= size / sizeof(u32); // Reduce the used amount of the internal ring buffer.
				memmove(atb, atb + (size / sizeof(u32)), atbUsed * sizeof(u32)); // Slide the ring buffer
			} else {
				resampleBuffer((short*)released->buffer, (size / sizeof(u32))); // Resample audio to Switch samplerate, push into released buffers.
				int outsize = (size * current_samplerate) / (sizeof(u32) * AUDIO_SAMPLERATE); // Get the size of the interpolated output
				atbUsed -= outsize;
				if (atbUsed<0) atbUsed=0;
				memmove(atb, atb + outsize, atbUsed * sizeof(u32)); // Slide the ring buffer
			}
			audoutAppendAudioOutBuffer(released); // Append the released buffers with their new contents to the Switch audio buffers
		} else { // If there's nothing in the internal ring buffer, flush the Switch buffers then shut down the playback thread.
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
	
	// Why am I freeing only two of these? Didn't I use 4?
	free(rdata[0]);
	free(rdata[1]);
}

int fillPlayBuffer(int16_t *inBuffer, int length) { // Fill the ring buffer with new audio.
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

// This code partially by LogiCow of the MZX Discord
void resampleBuffer(short* out, int numSamples) { // The scary function! Linearly resample/interpolate to Switch samplerate.
	// QUESTION: What happens to this function when the samplerate is *higher* than output?
	short *s_atb = reinterpret_cast<short *>(atb); // Recast the internal ring buffer into 16-bit shorts (per-channel) instead of 32-bit integers (both channels)
	unsigned long long m_mixPtr=0; // Create a 64-bit pointer into the mixing buffer
	int resampleDelta=((current_samplerate*0x40000ull)/AUDIO_SAMPLERATE); // Figure out the multiplier to turn the input samplerate to the desired samplerate.
									      // Uncertain of the 0x40000ull value other than it exists to enable the division using a 64-bit integer,
                                                                              // and is used to position within the mixing buffer.
	for(int i=0; i<numSamples; i++) { // Iterate over the input samples.
		
		int idx = m_mixPtr >> 18; // Get an index into the source buffer by taking the mix pointer and discarding the extra bits.
		int s1 = s_atb[(idx)*2]; // Get the 'current' left channel sample
		int s2 = s_atb[(idx+1)*2]; // Get the 'next' left channel sample
		int interp = m_mixPtr & 0x3ffff; // Get the actual interpolation multiplier by clearing the high bits of the mix pointer value.
		long long int sRamp = s2-s1; // Get the difference between the two samples.
		sRamp *= interp; // Interpolate the difference
		sRamp >>= 18; // Bitshift to clamp.
		sRamp += s1; // Add back the original sample.
		if(sRamp > 32767) sRamp = 32767; // Further clamping, ensure values are not larger/smaller that signed 16-bit max/min
		if(sRamp < -32768) sRamp = -32768;
		out[(i*2)] = (short)sRamp; // Add the interpolated sample to the output buffer
		
		// Repeat for the right channel. Should this be broken out into its own function?
		s1 = s_atb[(idx*2)+1];
		s2 = s_atb[((idx+1)*2)+1];
		sRamp = s2-s1;
		sRamp *= interp;
		sRamp >>= 18;
		sRamp += s1;
		if(sRamp > 32767) sRamp = 32767;
		if(sRamp < -32768) sRamp = -32768;
		out[((i*2)+1)] = (short)sRamp;
		
		// Move the mix pointer forwards.
		m_mixPtr += resampleDelta;
	}
}
