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

string name_channels(int c) {
	switch (c) {
		case 1:
			return "Mono";
			break;
		case 2:
			return "Stereo";
			break;
		default:
			return c + " Channel";
	}
}

string convert_time(int time) {
	int hour = time/3600;
	int second = time%3600;
	int minute = second/60;
	second %= 60;
	char retval[16];
	if (hour > 0) {
		sprintf(retval,"%.2d:%.2d:%.2d",hour,minute,second);
	} else {
		sprintf(retval,"%.2d:%.2d",minute,second);
	}
	return string(retval);
}

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

int audioLoop(const string& musicfile) {
	printf("SchalterVox is loading...\n");
	bool selfquit = false;
	bool active = true;
	bool next = 0;
	
	input_handler *ih = new input_handler();
	ih->start();
	
	start_playback();

	audioFile *af = new audioFile(musicfile);
	af->load_file();
	af->update_metadata(true);
	af->Decoder->start();
	
	consoleClear();
	string outstr = "";
		
	outstr += "SchalterVox Alpha\nNow Playing: "; //row0 end
	
	string tmp;
	if (af->metadata.artist != "Unknown") {
		tmp = af->metadata.artist + " - " + af->metadata.title;
	} else { 
		tmp = af->metadata.title;
	}
	if (tmp.length()>80) {
		tmp.resize(76);
		tmp = tmp + "...";
	}
	outstr += tmp + "\n"; //row1 end
	if (af->metadata.album != "Unknown") {
		outstr += "Album: ";
		outstr += af->metadata.album;		
	}
	outstr += "\n"; //row2 end

	outstr += to_string(af->metadata.bitrate) + "kbps ";
	outstr += to_string(af->metadata.samplerate) + "Hz ";
	outstr += to_string(af->metadata.bitdepth) + "bit ";
	outstr += name_channels(af->metadata.channels); 
	outstr += " " + af->metadata.fancyftype + "\n"; //row3 end
	printf(outstr.c_str());

	while (active) {
		af->update_metadata(false);
		outstr = "";
		printf("\x1b[5;0H"); // move to the 5th row, first column		
		outstr += "Time: ";
		outstr += convert_time(af->metadata.currtime);
		outstr += "/";
		outstr += convert_time(af->metadata.length);
		outstr += "\nPress + to stop, R/ZR/SR/DPAD Right to skip forward a track.\n";
		printf(outstr.c_str());
		
		u32 signals = ih->get_signals();
		if (signals & SIG_STOPQUIT) {
			active=false;
			selfquit=false;
			next=0;
			printf("\nStopping...\n");
		}
		if (!af->Decoder->check_running()) {
			active=false;	
			selfquit=true;
			next=1;
			printf("\nNext track...\n");
		}
		if (!appletMainLoop()) {
			active=false;
			selfquit=false;
			next=-1;
			printf("\nQuitting...\n");
		}
		if (signals & SIG_NEXT) {
			active=false;
			selfquit=false;
			next=1;
			printf("\nSkipping...\n");
		}
		gfxFlushBuffers();
		gfxSwapBuffers();
		gfxWaitForVsync();
	}
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
	printf("SchalterVox Alpha Loaded!\n");
	printf("Looking for media...\n");
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
			int x = audioLoop(i);
			if (x == -1) return 0;
			if (x == 0) break;
		}
		printf("Exited Audio mode. Press + to quit.\n");
	}
	waitExit();
	return 0;
}
