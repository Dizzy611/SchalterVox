#ifndef AUDIOFILE_H
#define AUDIOFILE_H
#include <string>
#include <switch.h>
#include <samplerate.h>

#include "util.hpp"
#include "playback.hpp"


using namespace std;

typedef string vorbisFile;
typedef string flacFile;
typedef string mp3File;
typedef string wavFile;
typedef string modFile;

struct metadata {
	string filename;
	string filetype;
	string artist;
	string album;
	string title;
	u16 bitrate;
	int samplerate;
	u8 channels;
	u8 bitdepth;
	vector<string> other;
};

class decoder {
	public:
		decoder();
		~decoder();
		bool decoderValid;
		string decoderError;
	protected:
		Thread decodingThread;
		Mutex decodeLock = 0;
		CondVar decodeStatusCV;
		Mutex decodeStatusLock = 0;
		s16 *decodeBuffer;
		u32 decodeBufferSize;
		bool decodeRunning;
		
		float *resamplingBufferIn;
		float *resamplingBufferOut;
		SRC_DATA resamplerData;
		SRC_STATE *resamplerState;
		int resamplerError;
		s16 *resampledBuffer;
		
};

class audioFile {
	public:
		decoder Decoder;
		metadata Metadata;
		audioFile(const string& filename);
		void playFile();
	private:
		int checkDecoder();
		void validateFile();
		void updateMetadata();
};

#endif