#ifndef DECODER_H
#define DECODER_H
#include <switch.h>
#include <string>
#include "metadata.hpp"

#define SAMPLERATE 48000
#define CHANNELCOUNT 2
#define FRAMEMS 1
#define FRAMERATE (1000 / FRAMEMS)
#define SAMPLECOUNT (SAMPLERATE / FRAMERATE)
#define BYTESPERSAMPLE 2
#define BUFFERSIZE (SAMPLECOUNT * CHANNELCOUNT * BYTESPERSAMPLE)

using namespace std;

class decoder {
	public:
		decoder();
		virtual ~decoder();
		bool decoderValid;
		string decoderError;
		virtual void start();
		virtual void stop();
		virtual bool check_running();
		virtual int seek(long position);
		virtual int seek_time(double time);
		metadata_t get_metadata(bool firstrun);
		void set_metadata(metadata_t in, bool firstrun);
		
	protected:
		virtual void parse_metadata();
		virtual void update_metadata();
		metadata_t metadata;
		Thread decodingThread;
		Mutex decodeLock = 0;
		CondVar decodeStatusCV;
		Mutex decodeStatusLock = 0;
		CondVar metadataCV;
		Mutex metadataLock = 0;
		s16 *decodeBuffer;
		u32 decodeBufferSize;
		bool decodeRunning;		
};

// Add new decoders in here so audiofile can access them.
#ifdef IN_AUDIOFILE
#include "vorbisdec.hpp"
//#include "mp3dec.hpp"
#endif

#endif