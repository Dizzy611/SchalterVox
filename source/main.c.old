// The following is the original C version of the code. This has similar problems to the C++ version in that it tends to stutter wildly.
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <switch.h>
#include <vorbis/codec.h>
#include <dirent.h>
#include <errno.h>

// Sane minimums/maximums for an 80x25 screen, allowing for the text to be slightly centered.
#define TXMIN 10 
#define TXMAX 70
#define TYMIN 5
#define TYMAX 20

u32 tx, ty; // Current text position. I know, I know, globals are evil, but this is the best way I can find to do this without making the code ugly.

const char *getFileExt(const char *fn) {
	const char *ext = strrchr(fn, '.');
	if(!ext || ext == fn) return "";
	return ext + 1;
}

int wrapOut(char *string, int newline) {
	// TODO: Word wrapping.
	return 0;
}

int textOut(char *string, int newline) {
	u32 temp = tx + strlen(string);
	if (temp > TXMAX) {
		return wrapOut(string, newline);
	} else {
		printf("\x1b[%d;%dH%s", ty, tx, string);
		if (newline == 1) {
			tx = TXMIN;
			ty++;
			if (ty > TYMAX) {
				// TODO: Scroll down.
				ty = TYMIN;
			}
		} else {
			tx += strlen(string);
		}
	}
	return 0;
}

int findFiles(char *path, char *ext, char **dirarray, int size) {
	struct dirent *dp;
	DIR *dfd = opendir(path);
	for (int i=0; i<=size-1; i++) { // Fill array with null strings.
		dirarray[i] = malloc(1);
		dirarray[i][0] = 0;
	}
	if(dfd != NULL) {
		int i = 0;
		while ((dp = readdir(dfd)) != NULL) {
			char *fname = malloc(strlen(dp->d_name) + strlen(path) + 2);
			sprintf(fname, "%s/%s", path, dp->d_name);
			char *ext = getFileExt(fname);
			if (strcmp(ext, "ogg") == 0) {
				if (i<=size-1) {
					dirarray[i] = malloc(strlen(fname) + 1);
					strcpy(dirarray[i], fname);
					i++;
				} else {
					return 0;
				}
			}
			free(fname);
		}
		closedir(dfd);
	} else {
		textOut("Unable to open directory.", 1);
		return errno;
	}
	return 0;
}

int printDir(char *path) {
	struct dirent *dp;
	DIR *dfd = opendir(path);
	if(dfd != NULL) {
		while ((dp = readdir(dfd)) != NULL) {
			textOut(dp->d_name, 1);
		}
		closedir(dfd);
	} else {
		textOut("Unable to open directory.", 1);
		return errno;
	}
	return 0;
}

int main(int argc, char** argv) {
	//u32* fb;           // Framebuffer (currently unused)
	//u32 w, h, p;       // Framebuffer width/height/position
	
	// Starting point 
	tx = TXMIN;
	ty = TYMIN;
	
	gfxInitDefault();
	consoleInit(NULL);
	printf("%d", errno);
	textOut("SchalterPlayer Loaded!", 1);
	textOut("Looking for oggs...", 1);
	char *oggs[256];
	int x = findFiles("./oggs", ".ogg", oggs, 256);
	for (int i = 0; i<=255; i++) {
		if (oggs[i][0] != 0) {
			textOut(oggs[i], 1);
		}
	}
	if (x != 0) {
		char errorstring[100];
		sprintf(errorstring, "Error Number %d", x);
		textOut(errorstring, 1);
	}
	textOut("Checking for working libogg...", 1);
	textOut("FAILED: libogg not yet implemented. Press + to quit.", 1);

	while(appletMainLoop()) {
		hidScanInput();
		u64 keys = hidKeysDown(CONTROLLER_P1_AUTO);
		
		if (keys & KEY_PLUS) break;
		//fb = (u32*) gfxGetFramebuffer((u32*)&w, (u32*)&h); // We aren't yet using the framebuffer.
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		gfxWaitForVsync();
	}
	gfxExit();
	return 0;
}


	
