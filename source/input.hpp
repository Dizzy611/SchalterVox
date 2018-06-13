#ifndef INPUT_H
#define INPUT_H
#include <switch.h>
#include <queue>

using namespace std;

#define SIG_STOPQUIT 1
#define SIG_PLAY 2
#define SIG_NEXT 4
#define SIG_PREV 8
#define SIG_SEEK 16
#define SIG_ERROR 32
#define SIG_WAIT 64

class input_handler {
	public:
		input_handler();
		~input_handler();
		u32 get_signals();
		void start();
		void stop();
	private:
		Thread input_thread;
		Mutex status_lock;
		CondVar status_cv;
		Mutex input_lock;
		CondVar input_cv;
		u32 signals;
		bool running;
		void process_signals(u64 key_down);
		void main_thread(void *);
};

