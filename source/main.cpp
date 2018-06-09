#include <iostream>
#include <string>
#include <cstring>
#include <switch.h>
#include <errno.h>
#include "util.hpp"
#include "audiofile.hpp"

using namespace std;

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
	
	printf("Loading first ogg file and attempting to play...\n");
	audioFile *af = new audioFile(files.front());
	af->playFile();
	delete af;
	printf("Playback loop ended. TODO: Play next file. :P\n");
	while(appletMainLoop()) {
		hidScanInput();
		u64 keys = hidKeysDown(CONTROLLER_P1_AUTO);
		
		if (keys & KEY_PLUS) break;

		gfxFlushBuffers();
		gfxSwapBuffers();
		gfxWaitForVsync();
	}
	
	gfxExit();
	return 0;
}


	