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


class decoder {
	public:
		decoder();
		~decoder();
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
		string filename;
		string filetype;
		string title;
		string author;
		string album;
		audioFile(const string& filename);
		void playFile();
		
	private:
		void validateFile();
		void updateMetadata();
		int vorbisDec();
		int flacDec();
		int mp3Dec();
		int wavDec();
		int modplugDec();
		int nothingDec();
};

#endif