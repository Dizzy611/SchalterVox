#include <switch.h>
#include "audiofile.hpp"
#include "util.hpp"
#include "vorbisdec.hpp"

audioFile::audioFile(const string& filename) {
	this->filename = filename;
	this->filetype = getFileExt(filename);
	this->title = "Unknown";
	this->author = "hisUnknown";
	this->album = "Unknown";
}

void audioFile::playFile() {
	start_playback();
	if (this->filetype == "ogg") {
		bool active = false;
		bool selfquit = false;
		vorbisdecoder *vd;
		vd = new vorbisdecoder(this->filename);
		if (!vd->decoderValid) {
			printf("ERROR: %s \n", vd->decoderError.c_str());
			stop_playback(false);
			return;
		}
		printf("Loaded vorbis file %s\n", this->filename.c_str());
		printf("%i channels @%ldHz\n", vd->info->channels, vd->info->rate);
		for (int i=0; i<(vd->comment->comments); i++) {
			printf(vd->comment->user_comments[i]);
			printf("\n");
		}
		printf("Beginning Ogg Vorbis playback. Press + to stop playback. Press + again to quit.\n");
		vd->start();
		active = true;
		while (active) {
			hidScanInput();
			u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
			if (kDown & KEY_PLUS) {
				printf("DEBUG: Key pressed. Asked to quit.\n");
				active=false;
				selfquit=false;
			}
			if (!vd->checkRunning()) {
				printf("DEBUG: Vorbis thread terminated.\n");
				active=false;
				selfquit=true;
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
	stop_playback();
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

