#ifndef AUDIOFILE_H
#define AUDIOFILE_H
#include <string>
#include <switch.h>
#include <samplerate.h>

#include "metadata.hpp"
#include "util.hpp"
#include "playback.hpp"

#include "decoder.hpp"

using namespace std;

class audioFile {
	public:
		decoder *Decoder;
		audioFile(const string& filename);
		~audioFile();
		void load_file();
		void update_metadata(bool firstrun);
		metadata_t metadata; // a copy of the decoder's metadata
	
};

#endif