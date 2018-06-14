#include <switch.h>
#include <sstream>
#include <string>
#include "audiofile.hpp"
#include "util.hpp"
#include "vorbisdec.hpp"
#include "playback.hpp"

audioFile::audioFile(const string& filename) {
	this->Metadata.filename = filename;
	this->Metadata.filetype = getFileExt(filename);
	this->Metadata.title = "Unknown";
	this->Metadata.artist = "Unknown";
	this->Metadata.album = "Unknown";
	this->Metadata.bitrate = 0;
	this->Metadata.samplerate = 48000;
	this->Metadata.channels = 2;
	this->Metadata.bitdepth = 16;
	this->Metadata.length = 0;
	this->Metadata.currtime = 0;
	this->Metadata.fancyftype = "Unknown";
}

audioFile::~audioFile() {
	delete this->Decoder;
}

void audioFile::loadFile() {
	string filetype = this->Metadata.filetype;
	if (filetype == "ogg") {
		this->Metadata.fancyftype = "OGG Vorbis";
		this->Decoder = new vorbisdecoder(this->Metadata.filename);
		vorbisdecoder *vd = (vorbisdecoder*)this->Decoder;
		if (!vd->decoderValid) {
			printf("ERROR: %s \n", vd->decoderError.c_str());
			abort_playback();
			return;
		}
		this->Metadata.channels = vd->info->channels;
		this->Metadata.samplerate = vd->info->rate;
		set_samplerate(vd->info->rate);
		this->Metadata.bitrate = vd->info->bitrate_nominal;
		this->Metadata.length = vd->length_time();
		for (int i=0; i<(vd->comment->comments); i++) {
			vorbis_tags vt = vorbis_comment_split(vd->comment->user_comments[i]);
			if (vt.header == "ARTIST") {
				this->Metadata.artist = vt.body;
			} else if (vt.header == "ALBUM") {
				this->Metadata.album = vt.body;
			} else if (vt.header == "TITLE") {
				this->Metadata.title = vt.body;
			} else {
				this->Metadata.other.push_back(vd->comment->user_comments[i]);
			}
		}
	} else if (filetype == "mp3") {
		// TODO
	} else if (filetype == "flac") {
		// TODO
	} else if (filetype == "wav") {
		// TODO
	} else if ((filetype == "mod") || (filetype == "it") || (filetype == "s3m") || (filetype == "xm")) {
		// TODO
	}
}

float audioFile::secondsFromSamples(int samples) {
	int samplerate = this->Metadata.samplerate;
	float seconds = (samples*1.0)/(samplerate*1.0);
	return seconds;
}


int audioFile::samplesFromSeconds(float seconds) {
	return (int)seconds*this->Metadata.samplerate;
}

decoder::decoder() {
	this->decoderValid = false;
	this->decoderError = "";
	condvarInit(&this->decodeStatusCV, &this->decodeStatusLock);
	mutexLock(&this->decodeLock);
	mutexUnlock(&this->decodeLock);
	this->Metadata.filename = "Unknown";
	this->Metadata.filetype = "Unknown";
	this->Metadata.title = "Unknown";
	this->Metadata.artist = "Unknown";
	this->Metadata.album = "Unknown";
	this->Metadata.bitrate = 0;
	this->Metadata.samplerate = 48000;
	this->Metadata.channels = 2;
	this->Metadata.bitdepth = 16;
	this->Metadata.length = 0;
	this->Metadata.currtime = 0;
	this->Metadata.fancyftype = "Unknown";
}


decoder::~decoder() {
}

// Here there be stubs. These are all 'defaults' for virtual functions intended to be overridden by specific decoders.

void decoder::start() {} // Should start the decode thread, and begin filling the playback buffer with fillPlayBuffer

void decoder::stop() {} // Should stop the decode thread.

bool decoder::checkRunning() { return false; } // Should return whether or not the decode thread is active.

long decoder::tell() { return 0; } // Should return current position in PCM samples.

long decoder::tell_time() { // Should return current position in seconds.
	return 0;
}

long decoder::length() { // Should return total length in PCM samples.
	return 0;
}

long decoder::length_time() { // Should return total length in seconds.
	return 0;
}

int decoder::seek(long position) { // Should seek to a position given in PCM samples.
	return 0;
}

int decoder::seek_time(double time) { // Should seek to a position given in seconds.
	return 0;
}

int decoder::get_bitrate() { // Should return the average bitrate, constant bitrate, or VBR setting (values below 10 will be assumed to be a VBR setting)
	return 0;
}