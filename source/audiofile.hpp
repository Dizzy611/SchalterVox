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
	string fancyftype;
	u16 bitrate;
	int samplerate;
	u32 length;
	u32 currtime;
	u8 channels;
	u8 bitdepth;
	vector<string> other;
};

class decoder {
	public:
		decoder();
		virtual ~decoder();
		bool decoderValid;
		string decoderError;
		virtual void start();
		virtual void stop();
		virtual bool checkRunning();
		virtual long tell();
		virtual long tell_time();
 		virtual long length();
		virtual long length_time();
		virtual int seek(long position);
		virtual int seek_time(double time);
		virtual int get_bitrate();
		metadata Metadata;
	protected:
		Thread decodingThread;
		Mutex decodeLock = 0;
		CondVar decodeStatusCV;
		Mutex decodeStatusLock = 0;
		s16 *decodeBuffer;
		u32 decodeBufferSize;
		bool decodeRunning;		
};

class audioFile {
	public:
		decoder *Decoder;
		audioFile(const string& filename);
		~audioFile();
		void loadFile();
		float secondsFromSamples(int samples);
		int samplesFromSeconds(float seconds);
		metadata Metadata;
};

#endif