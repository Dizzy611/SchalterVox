#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include "playback.hpp"
#include "vorbisdec.hpp"

#define SAMPLERATE 48000
#define CHANNELCOUNT 2
#define FRAMEMS 1
#define FRAMERATE (1000 / FRAMEMS)
#define SAMPLECOUNT (SAMPLERATE / FRAMERATE)
#define BYTESPERSAMPLE 2
#define BUFFERSIZE (SAMPLECOUNT * CHANNELCOUNT * BYTESPERSAMPLE)

using namespace std;

vorbis_tags vorbis_comment_split(const string &s) {
	vorbis_tags retval;
	stringstream ss(s);
	string item;
	string body_construct = "";
	int i = 0;
	while (getline(ss, item, '=')) {
		if (i == 0) {
			retval.header = item;
		} else {
			body_construct = body_construct + item + "=";
		}
		i++;
	}
	body_construct.pop_back();
	retval.body = body_construct;
	return retval;
}

vorbisdecoder::vorbisdecoder(const string& fileName) {
	mutexLock(&this->decodeLock);
	this->fn = fileName;
	int lvRet = ov_fopen(fileName.c_str(), &this->vorbisFile);
	if (lvRet < 0) {
		this->decoderValid = false;
		this->decoderError = "Error " + to_string(lvRet) + " during OV_OPEN";
		return;
	}
	this->info = ov_info(&this->vorbisFile, -1);
	this->comment = ov_comment(&this->vorbisFile,-1);
	this->parse_metadata();
	this->decodeBufferSize = ((this->info->rate/FRAMERATE) * BYTESPERSAMPLE * this->info->channels);
	this->decodeBuffer = (s16 *) malloc(this->decodeBufferSize);
	this->decodeRunning = false;
	this->decoderValid = true;
	mutexUnlock(&this->decodeLock);
}
		
vorbisdecoder::~vorbisdecoder() {
	mutexLock(&this->decodeLock);
	this->decodeRunning = false;
	free(this->decodeBuffer);
	ov_clear(&this->vorbisFile);
	mutexUnlock(&this->decodeLock);
}

void vorbisdecoder::start() {
	this->decodeRunning = true;
	threadCreate(&this->decodingThread, vorbisdecoder_trampoline, this, 0x10000, 0x2D, 3);
	threadStart(&this->decodingThread);
}

void vorbisdecoder::stop() {
	mutexLock(&this->decodeStatusLock);
	condvarWait(&this->decodeStatusCV);
	this->decodeRunning = false;
	mutexUnlock(&this->decodeStatusLock);
	threadWaitForExit(&this->decodingThread);
}

void vorbisdecoder::parse_metadata() {
	metadata_t m = this->metadata;
	m.filename=this->fn;
	m.filetype="ogg";
	m.fancyftype="Ogg Vorbis";
	m.bitdepth = 16;
	m.channels = this->info->channels;
	m.samplerate = this->info->rate;
	set_samplerate(m.samplerate);
	m.bitrate = this->info->bitrate_nominal/1024;
	for (int i=0; i<(this->comment->comments); i++) {
		vorbis_tags vt = vorbis_comment_split(this->comment->user_comments[i]);
		if (vt.header == "ARTIST") {
			m.artist = vt.body;
		} else if (vt.header == "ALBUM") {
			m.album = vt.body;
		} else if (vt.header == "TITLE") {
			m.title = vt.body;
		} else {
			m.other.push_back(this->comment->user_comments[i]);
		}
	}
	m.length = ov_time_total(&this->vorbisFile, -1);
	m.length_pcm = ov_pcm_total(&this->vorbisFile,-1);
	this->set_metadata(m, true);
}

void vorbisdecoder::update_metadata() {
	metadata_t m = this->metadata;
	m.currtime = ov_time_tell(&this->vorbisFile);
	m.currpcm = ov_pcm_tell(&this->vorbisFile);
	this->set_metadata(m, false);
}

int vorbisdecoder::seek(long position) {
	return ov_pcm_seek_lap(&this->vorbisFile, position);
}

int vorbisdecoder::seek_time(double time) {
	return ov_time_seek_lap(&this->vorbisFile, time);
}


bool vorbisdecoder::check_running() {
	mutexLock(&this->decodeStatusLock);
	condvarWaitTimeout(&this->decodeStatusCV,100000);
	bool tmp = this->decodeRunning;
	mutexUnlock(&this->decodeStatusLock);
	return tmp;
}

void vorbisdecoder_trampoline(void *parameter) {
	vorbisdecoder* obj = (vorbisdecoder*)parameter;
	obj->main_thread(NULL);
}

void vorbisdecoder::main_thread(void *) {
	mutexLock(&this->decodeStatusLock);
	this->decodeRunning=true;
	bool running=this->decodeRunning;
	
	while (running) {
		condvarWakeAll(&this->decodeStatusCV);
		mutexUnlock(&this->decodeStatusLock);

		mutexLock(&this->decodeLock);
		long retval = ov_read(&this->vorbisFile,(char *)this->decodeBuffer,this->decodeBufferSize,0,2,1,&this->section);

		if (retval == 0) {
			mutexLock(&this->decodeStatusLock);
			this->decodeRunning = false;
			condvarWakeAll(&this->decodeStatusCV);
			mutexUnlock(&this->decodeStatusLock);
		} else if ((retval == OV_HOLE) || (retval == OV_EBADLINK) || (retval == OV_EINVAL)) {
			mutexLock(&this->decodeStatusLock);
			this->decodeRunning = false;
			condvarWakeAll(&this->decodeStatusCV);
			mutexUnlock(&this->decodeStatusLock);
			this->decoderError = "VORBIS: Decode error " + to_string(retval);
		} else {
			int x = fillPlayBuffer(this->decodeBuffer, retval/2);
			if (x == -1) { // buffer overflow. Wait before trying again.
				while (x == -1) {
					svcSleepThread((FRAMEMS * 1000000)/4);
					x = fillPlayBuffer(this->decodeBuffer, retval/2);
				}
			}
		}
		mutexUnlock(&this->decodeLock);
		
		svcSleepThread(1000);
		
		this->update_metadata();
		mutexLock(&this->decodeStatusLock);
		running = this->decodeRunning;
	}
	condvarWakeAll(&this->decodeStatusCV);
	mutexUnlock(&this->decodeStatusLock);
}