#include "util.hpp"
#include "audiofile.hpp"
#include "input.hpp"
#include "playback.hpp"

using namespace std;

vector<string> valid_filetypes {"ogg"};


string name_channels(int c) {
	switch (c) {
		case 1:
			return "Mono";
			break;
		case 2:
			return "Stereo";
			break;
		default:
			return to_string(c) + " Channel";
	}
}

string convert_time(int time) {
	char retval[16];
	if (time == -1) {
		sprintf(retval,"??:??");
	} else {
		int hour = time/3600;
		int second = time%3600;
		int minute = second/60;
		second %= 60;
		if (hour > 0) {
			sprintf(retval,"%.2d:%.2d:%.2d",hour,minute,second);
		} else {
			sprintf(retval,"%.2d:%.2d",minute,second);
		}
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
	#ifndef DEBUG_DISABLE_ALOOP_PRINT
	consoleClear();
	#endif
	
	#ifdef DEBUG_DISABLE_ALOOP_PRINT
	printf("SchalterVox Alpha. Debug Mode (metadata disabled).\n");
	#else
	printf("\x1b[1;0HSchalterVox Alpha");
	#endif
	
	#ifndef DEBUG_DISABLE_ALOOP_PRINT
	printf("\x1b[2;0HNow Playing: %s - %s",af->metadata.artist.c_str(),
	        af->metadata.title.c_str());
	printf("\x1b[3;0HAlbum: %s",af->metadata.album.c_str());
	printf("\x1b[4;0H%ikbps %iHz %ibit %s %s",af->metadata.bitrate, 
	        af->metadata.samplerate, af->metadata.bitdepth, 
			name_channels(af->metadata.channels).c_str(), 
			af->metadata.fancyftype.c_str());
	#endif
	
	while (active) {
		af->update_metadata(false);
		#ifndef DEBUG_DISABLE_ALOOP_PRINT
		printf("\x1b[5;0HTime: %s/%s", convert_time(af->metadata.currtime).c_str(), convert_time(af->metadata.length).c_str());
		printf("\x1b[6;0HPress + to stop, R/ZR/SR/DPAD Right to skip forward a track.\n");
		#endif
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
	if (findFilesByExt("./media", valid_filetypes, files) == 0) {
		for (auto & i : files) {
			printf("FILE FOUND: %s\n",i.c_str());
		}
	} else {
		printf("While reading directory: Error number %i\n", errno);
	}
	
	if (files.empty()) {
		printf("No acceptable media files found. Please place files in the media/ directory.\nPress + to quit.\n");
	} else {
		for (auto & i : files) {
			printf("Loading a media file (%s) and attempting to play...\n", i.c_str());
			int x = audioLoop(i);
			if (x == -1) return 0;
			if (x == 0) break;
		}
		printf("Exited Audio mode. Press + to quit.\n");
	}
	waitExit();
	return 0;
}
