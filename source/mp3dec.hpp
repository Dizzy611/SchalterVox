#ifndef MP3DEC_HPP
#define MP3DEC_HPP
#include <string>
#include <sstream>
#include <map>

extern "C" {
	#include "dr_libs/dr_mp3.h"
}

// aside from adding variables as needed, this header should remain relatively the same

void float_to_s16(float* in, s16* out, int length);

long calculate_length(int bitrate, long filesize);

void mp3decoder_trampoline(void *parameter);

struct mp3metadata {
	int in_rate;
	int out_rate;
	int bit_rate;
	int in_channels;
	int out_channels;
	string artist;
	string album;
	string title;
	int current_time;
	int length;
	int bit_depth;
};

struct id3 {
	int version; // 1 for v1, 2 for v2, 3 for both present, 0 for no tags present.
	char v1_title[30];
	char v1_artist[30];
	char v1_album[30];
	int v1_year;
	char v1_tracknum;
	char v1_genre;
	char v1_comment[30];
	map<string,string> v2_frames;
};


typedef enum { artist, album, title } Basic_Tag;

id3 parse_id3_tags(const string& fileName);

string get_id3_tag(id3 tags, Basic_Tag request);

class mp3decoder : public decoder {
	public:
		mp3decoder(const string& fileName);
		~mp3decoder();
		void start();
		void stop();
		long tell();
		long tell_time();
 		long length();
		long length_time();
		int seek(long position);
		int seek_time(double time);
		int get_bitrate();
		mp3metadata get_metadata();
		bool checkRunning();
		void main_thread(void *); // This should really be private, but it needs to be public due to the thread trampoline.
	private:
		drmp3 mp3;
		float *floatBuffer; // mp3 decoder produces floating points, we need a buffer to store these when we turn them into s16 pcm samples.
		float floatBufferSize;
		int numFrames;
		void convert_buffer();
		mp3metadata metadata;
		id3 rawid3;
		int fileSize; // necessary for determining length.
};
#endif