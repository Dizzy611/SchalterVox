#ifndef METADATA_H
#define METADATA_H
#include <string>
#include <switch.h>
#include <vector>

using namespace std;



struct metadata_t {
	string filename;
	string filetype;
	string artist;
	string album;
	string title;
	string fancyftype;
	u16 bitrate=0;
	int samplerate=48000;
	u32 length=0;
	u32 length_pcm=0;
	u32 currtime=0;
	u32 currpcm=0;
	u8 channels=2;
	u8 bitdepth=16;
	vector<string> other;
};

#endif