#include "decoder.hpp"

decoder::decoder() {
	this->decoderValid = false;
	this->decoderError = "";
	condvarInit(&this->decodeStatusCV, &this->decodeStatusLock);
	mutexLock(&this->decodeLock);
	mutexUnlock(&this->decodeLock);
	this->metadata.filename = "Unknown";
	this->metadata.filetype = "Unknown";
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


decoder::~decoder() {
}


/* metadata_t decoder::get_metadata() {
	metadata_t m;
	mutexLock(&this->metadataLock);
	condvarWait(&this->metadataCV);
	m = this->metadata;
	mutexUnlock(&this->metadataLock);
	return m;
} */ // Shallow copies are bad mmk. Was causing crashes.


metadata_t decoder::get_metadata(bool firstrun) {
	mutexLock(&this->metadataLock);
	condvarWait(&this->metadataCV);
	metadata_t m;
	if (firstrun) {
		m.filename = this->metadata.filename.data();
		m.filetype = this->metadata.filetype.data();
		m.artist = this->metadata.artist.data();
		m.album = this->metadata.album.data();
		m.title = this->metadata.title.data();
		m.fancyftype = this->metadata.fancyftype.data();
		m.other.clear();
		for (auto & i : this->metadata.other) {
			m.other.push_back(i);
		}
		m.channels = this->metadata.channels;
		m.bitdepth = this->metadata.bitdepth;
	} else {
		m.filename = "";
		m.filetype = "";
		m.artist = "";
		m.album = "";
		m.title = "";
		m.fancyftype = "";
		m.other.clear();
		m.channels = 2;
		m.bitdepth = 16;
	}
	m.bitrate = this->metadata.bitrate;
	m.samplerate = this->metadata.samplerate;
	m.length = this->metadata.length;
	m.length_pcm = this->metadata.length_pcm;
	m.currtime = this->metadata.currtime;
	m.currpcm = this->metadata.currpcm;
	mutexUnlock(&this->metadataLock);
	return m;
}

/* void decoder::set_metadata(metadata_t in) {
	mutexLock(&this->metadataLock);
	
	this->metadata = in;
	condvarWakeAll(&this->metadataCV);
	mutexUnlock(&this->metadataLock);
}
 */ // Avoiding shallow copies here too.

void decoder::set_metadata(metadata_t in, bool firstrun) {	
	mutexLock(&this->metadataLock);
	if (firstrun) {
		this->metadata.filename = in.filename.data();
		this->metadata.filetype = in.filetype.data();
		this->metadata.artist = in.artist.data();
		this->metadata.album = in.album.data();
		this->metadata.title = in.title.data();
		this->metadata.fancyftype = in.fancyftype.data();
		this->metadata.channels = in.channels;
		this->metadata.bitdepth = in.bitdepth;
		this->metadata.other.clear();
		for (auto & i : in.other) {
			this->metadata.other.push_back(i);
		}
	}
	this->metadata.bitrate = in.bitrate;
	this->metadata.samplerate = in.samplerate;
	this->metadata.length = in.length;
	this->metadata.length_pcm = in.length_pcm;
	this->metadata.currtime = in.currtime;
	this->metadata.currpcm = in.currpcm;
	condvarWakeAll(&this->metadataCV);
	mutexUnlock(&this->metadataLock);
}

// Here there be stubs. These are all 'defaults' for virtual functions intended to be overridden by specific decoders.

void decoder::start() {} // Should start the decode thread, and begin filling the playback buffer with fillPlayBuffer

void decoder::stop() {} // Should stop the decode thread.	

bool decoder::check_running() { return false; } // Should return whether or not the decode thread is active.

int decoder::seek(long position) { // Should seek to a position given in PCM samples.
	return 0;
}

void decoder::parse_metadata() {}; // Deal with metadata, initial and subsequent
void decoder::update_metadata() {};

int decoder::seek_time(double time) { // Should seek to a position given in seconds.
	return 0;
}
