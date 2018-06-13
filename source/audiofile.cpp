#include <switch.h>
#include <sstream>
#include <string>
#include "audiofile.hpp"
#include "util.hpp"
#include "vorbisdec.hpp"
#include "input.hpp"

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
}

void audioFile::playFile() {
	bool selfquit = false;
	start_playback();
	input_handler *ih = new input_handler();
	ih->start();
	string filetype = this->Metadata.filetype;
	if (filetype == "ogg") {		
		bool active = false;
		vorbisdecoder *vd;
		vd = new vorbisdecoder(this->Metadata.filename);
		if (!vd->decoderValid) {
			printf("ERROR: %s \n", vd->decoderError.c_str());
			abort_playback();
			return;
		}
		this->Metadata.channels = vd->info->channels;
		this->Metadata.samplerate = vd->info->rate;
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
		printf("DEBUG: FILENAME %s FILETYPE %s ARTIST %s ALBUM %s TITLE %s SAMPLERATE %i BITDEPTH %i CHANNELS %i\n",
		       this->Metadata.filename.c_str(), this->Metadata.filetype.c_str(), this->Metadata.artist.c_str(), this->Metadata.album.c_str(),
			   this->Metadata.title.c_str(), this->Metadata.samplerate, this->Metadata.bitdepth, this->Metadata.channels);
		printf("Beginning Ogg Vorbis playback. Press + to stop playback. Press + again to quit.\n");
		vd->start();
		active = true;
		while (active) {
			u32 signals = ih->get_signals();
			if (signals & SIG_STOPQUIT) {
				printf("DEBUG: Key pressed. Asked to quit.\n");
				active=false;
				selfquit=false;
			}
			if (!vd->checkRunning()) {
				printf("DEBUG: Vorbis thread terminated.\n");
				active=false;
				selfquit=true;
			}
			if (!appletMainLoop()) {
				printf("DEBUG: Asked to quit by Horizon.\n");
				active=false;
				selfquit=false;
			}
			gfxFlushBuffers();
			gfxSwapBuffers();
			gfxWaitForVsync();
		}
		if (!selfquit) {
			vd->stop();
		}
		printf("Vorbis playback stopped.\n");
		delete vd;
	} else if (filetype == "mp3") {
		// TODO
	} else if (filetype == "flac") {
		// TODO
	} else if (filetype == "wav") {
		// TODO
	} else if ((filetype == "mod") || (filetype == "it") || (filetype == "s3m") || (filetype == "xm")) {
		// TODO
	}
	ih->stop();
	delete ih;
	stop_playback(selfquit);
}

void audioFile::validateFile() {
	
}

void audioFile::updateMetadata() {
	
}


decoder::decoder() {
	this->decoderValid = false;
	this->decoderError = "";
	condvarInit(&this->decodeStatusCV, &this->decodeStatusLock);
	mutexLock(&this->decodeLock);
	this->resamplerState = src_new(SRC_LINEAR, 2, &this->resamplerError);
	this->resamplingBufferIn = (float*)malloc(16384*sizeof(float));
	this->resamplingBufferOut = (float*)malloc(16384*sizeof(float));
	mutexUnlock(&this->decodeLock);
}

decoder::~decoder() {
	mutexLock(&this->decodeLock);
	free(this->resamplingBufferIn);
	free(this->resamplingBufferOut);
	mutexUnlock(&this->decodeLock);
}

