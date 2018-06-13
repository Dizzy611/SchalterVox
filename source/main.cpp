#include <iostream>
#include <string>
#include <cstring>
#include <switch.h>
#include <errno.h>
#include "util.hpp"
#include "audiofile.hpp"

using namespace std;

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

void audioLoop() {

}

int main() {	
	gfxInitDefault();
	consoleInit(NULL);
	printf("SchalterVox Pre-Alpha Loaded!\n");
	printf("Looking for oggs...\n");
	vector<string> files;
	if (findFilesByExt("./oggs", "ogg", files) == 0) {
		for (auto & i : files) {
			printf("FILE FOUND: %s\n",i.c_str());
		}
	} else {
		printf("While reading directory: Error number %i\n", errno);
	}
	if (files.empty()) {
		printf("No OGG Vorbis files found. Please place vorbis files in the oggs/ directory.\nPress + to quit.\n");
	} else {
		printf("Loading first ogg file and attempting to play...\n");
		audioFile *af = new audioFile(files.front());
		af->playFile();
		delete af;
		printf("Exited playback mode. Press + to quit.\n");
	}
	waitExit();
	return 0;
}




	