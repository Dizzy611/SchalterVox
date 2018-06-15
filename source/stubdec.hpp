// This file and the accompanying source are stubs intended
// to show how to program a decoder to work with SchalterVox.
#ifndef STUB_H
#define STUB_H
#include <string>
#include <sstream>
#include "decoder.hpp" 

// aside from adding variables as needed, this header should remain relatively the same

void stubdecoder_trampoline(void *parameter);

class stubdecoder : public decoder {
	public:
		stubdecoder(const string& fileName);
		~stubdecoder();
		void start();
		void stop();
		int seek(long position);
		int seek_time(double time);
		bool check_running();
		void main_thread(void *); // This should really be private, but it needs to be public due to the thread trampoline.
		void parse_metadata();
		void update_metadata();
	private:
		// stick your file handle in here, and any other state variables for the decoder.
};
#endif
