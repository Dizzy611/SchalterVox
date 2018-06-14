#include <iostream>
#include <string>
#include <cstring>
#include <switch.h>
#include <errno.h>
#include "util.hpp"
#include "audiofile.hpp"
#include "input.hpp"

using namespace std;

const vector<string> valid_filetypes {"ogg"};

void waitExit() {
	while(appletMainLoop()) {
		hidScanInput();
		u64 keys = hidKeysDown(CONTROLLER_P1_AUTO);
		
		if (keys & KEY_PLUS) break;

		gfxFlushBuffers();
		gfxSwapBuffers();
		gfxWaitForVsync();
	}
	
	gfxExit();
}

bool audioLoop(const string& musicfile) {

	bool selfquit = false;
	bool active = true;
	bool next = false;
	input_handler *ih = new input_handler();
	ih->start();
	
	start_playback();

	audioFile *af = new audioFile(musicfile);
	af->loadFile();
	af->Decoder->start();
	
	if ((af->Metadata.artist != "Unknown") && (af->Metadata.title != "Unknown")) {
		printf("Now Playing: %s - %s\n", af->Metadata.artist.c_str(), af->Metadata.title.c_str());
	}
	if (af->Metadata.album != "Unknown") {
		printf("Album: %s\n", af->Metadata.album.c_str());
	}
	
	// Dunno why the bitrate is off by a factor of 5 for Vorbis. It seems to be from testing though.
	printf("%ikbit %iHz %i-bit %i channel %s\n", af->Metadata.bitrate*5/1024, af->Metadata.samplerate, af->Metadata.bitdepth, af->Metadata.channels, af->Metadata.fancyftype.c_str());
	printf("Press + to stop, R/ZR/SR/DPAD Right to skip forward a track.\n");
	while (active) {
		printf("%i/%i seconds\r", af->Decoder->Metadata.currtime, af->Metadata.length);
		u32 signals = ih->get_signals();
		if (signals & SIG_STOPQUIT) {
			active=false;
			selfquit=false;
			next=false;
		}
		if (!af->Decoder->checkRunning()) {
			active=false;	
			selfquit=true;
			next=true;
		}
		if (!appletMainLoop()) {
			active=false;
			selfquit=false;
			next=false;
		}
		if (signals & SIG_NEXT) {
			active=false;
			selfquit=false;
			next=true;
		}
		gfxFlushBuffers();
		gfxSwapBuffers();
		gfxWaitForVsync();
	}
	printf("\nStopped.\n");
	if (!selfquit) {
		af->Decoder->stop();
	}
	
	stop_playback(selfquit);
	ih->stop();
	delete ih;
	delete af;
	return next;
}

int main() {	
	gfxInitDefault();
	consoleInit(NULL);
	printf("SchalterVox Pre-Alpha Loaded!\n");
	printf("Looking for oggs...\n");
	vector<string> files;
	for (auto & i : valid_filetypes) {
		if (findFilesByExt("./media", i, files) == 0) {
			for (auto & i : files) {
				printf("FILE FOUND: %s\n",i.c_str());
			}
		} else {
			printf("While reading directory: Error number %i\n", errno);
		}
	}
	if (files.empty()) {
		printf("No acceptable media files found. Please place files in the media/ directory.\nPress + to quit.\n");
	} else {
		for (auto & i : files) {
			printf("Loading a media file and attempting to play...\n");
			if (!audioLoop(i)) break;
		}
		printf("Exited Audio mode. Press + to quit.\n");
	}
	waitExit();
	return 0;
}




	