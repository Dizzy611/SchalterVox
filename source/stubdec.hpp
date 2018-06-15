#include <string>
#include <sstream>

// aside from adding variables as needed, this header should remain relatively the same

void stubdecoder_trampoline(void *parameter);

class stubdecoder : public decoder {
	public:
		stubdecoder(const string& fileName);
		~stubdecoder();
		void start();
		void stop();
		long tell();
		long tell_time();
 		long length();
		long length_time();
		int seek(long position);
		int seek_time(double time);
		int get_bitrate();
		bool checkRunning();
		void main_thread(void *); // This should really be private, but it needs to be public due to the thread trampoline.
	private:
		// stick your file handle in here, and any other state variables for the decoder.
};
