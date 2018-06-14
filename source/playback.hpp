#ifndef PLAYBACK_H
#define PLAYBACK_H

void start_playback();
void abort_playback();
void playback_thread_main(void *);
int fillPlayBuffer(int16_t *inBuffer, int length);
void stop_playback(bool playout=true);
void set_samplerate(int sr);
void resampleBuffer(short* out, int numSamples);

#define AUDIO_SAMPLERATE 48000
#define AUDIO_BUFFER_DIVIDER 32
#define AUDIO_BUFFER_SAMPLES (AUDIO_SAMPLERATE / AUDIO_BUFFER_DIVIDER)
#define ATB_SIZE (AUDIO_BUFFER_SAMPLES * 6)

#endif