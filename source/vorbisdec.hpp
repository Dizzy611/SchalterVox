#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

void vorbisdecoder_trampoline(void *parameter);

class vorbisdecoder : public decoder {
	public:
		vorbisdecoder(const string& fileName);
		~vorbisdecoder();
		void start();
		void stop();
		bool checkRunning();
		vorbis_info* info;
		vorbis_comment* comment;
		void main_thread(void *); // This should really be private, but it needs to be public due to the thread trampoline.
	private:
		OggVorbis_File vorbisFile;
		int section;
		
};
