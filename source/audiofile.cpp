// passed to decoder.hpp to make it pull in all the decoders.
#define IN_AUDIOFILE
#include "decoder.hpp"
#include "audiofile.hpp"
#include "metadata.hpp"
#include "util.hpp"
#include "playback.hpp"

audioFile::audioFile(const string& filename) {
	this->metadata.filename = filename;
	this->metadata.filetype = getFileExt(filename);
	this->metadata.title = "Unknown";
	this->metadata.artist = "Unknown";
	this->metadata.album = "Unknown";
	this->metadata.bitrate = 0;
	this->metadata.samplerate = 48000;
	this->metadata.channels = 2;
	this->metadata.bitdepth = 16;
	this->metadata.length = 0;
	this->metadata.currtime = 0;
	this->metadata.fancyftype = "Unknown";
}

audioFile::~audioFile() {
	delete this->Decoder;
}

void audioFile::load_file() {
	string filetype = this->metadata.filetype;
	this->Decoder=NULL;
	if (filetype == "ogg") {
		this->Decoder = new vorbisdecoder(this->metadata.filename);
	} else if (filetype == "mp3") {
		// this->Decoder = new mp3decoder(this->metadata.filename);
	} else if (filetype == "flac") {
		// this->Decoder = new flacdecoder(this->metadata.filename);
	} else if (filetype == "wav") {
		// this->Decoder = new wavdecoder(this->metadata.filename);
	} else if ((filetype == "mod") || (filetype == "it") || (filetype == "s3m") || (filetype == "xm")) {
		// this->Decoder = new moddecoder(this->metadata.filename);
	} 
	
	
	if (this->Decoder==NULL || !this->Decoder->decoderValid) {
		if (this->Decoder!=NULL) {
			printf("ERROR CREATING DECODER: %s", this->Decoder->decoderError.c_str());
		} else {
			printf("ERROR CREATING DECODER: Unrecognized or unsupported filetype.\n");
		}
		abort_playback();
		return;
	}
}

void audioFile::update_metadata(bool firstrun) {
	if (firstrun) {
		this->metadata = this->Decoder->get_metadata(firstrun);
	} else {
		metadata_t m = this->Decoder->get_metadata(firstrun);
		this->metadata.bitrate = m.bitrate;
		this->metadata.samplerate = m.samplerate;
		this->metadata.length = m.length;
		this->metadata.length_pcm = m.length_pcm;
		this->metadata.currtime = m.currtime;
		this->metadata.currpcm = m.currpcm;
	}
}

