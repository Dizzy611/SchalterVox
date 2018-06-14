#include <switch.h>
#include "util.hpp"
#include "input.hpp"

input_handler::input_handler() {	
	mutexInit(&this->status_lock);
	condvarInit(&this->status_cv, &this->status_lock);
	mutexInit(&this->input_lock);
	condvarInit(&this->input_cv, &this->input_lock);
	this->running = false;
	this->signals = 0;
}

input_handler::~input_handler() {

}

void input_handler::start() {
	threadCreate(&this->input_thread, input_trampoline, this, 0x10000, 0x2E, 1);
	threadStart(&this->input_thread);
}

void input_handler::stop() {
	mutexLock(&this->status_lock);
	condvarWait(&this->status_cv);
	this->running = false;
	mutexUnlock(&this->status_lock);
	threadWaitForExit(&this->input_thread);
}

void input_trampoline(void *parameter) {
	input_handler* obj = (input_handler*)parameter;
	obj->main_thread(NULL);
}

void input_handler::main_thread(void *) {
	mutexLock(&this->status_lock);
	this->running = true;
	bool i = this->running;
	while (i) {
		condvarWakeAll(&this->status_cv);
		mutexUnlock(&this->status_lock);
		
		hidScanInput();
		mutexLock(&this->input_lock);
		this->process_signals(hidKeysDown(CONTROLLER_P1_AUTO));
		condvarWakeAll(&this->input_cv);
		mutexUnlock(&this->input_lock);
		
		mutexLock(&this->status_lock);
		i = this->running;
	}
}

void input_handler::process_signals(u64 key_down) {
	if (key_down & KEY_PLUS) {
		this->signals = this->signals | SIG_STOPQUIT;
	}
	if ((key_down & KEY_DRIGHT) || (key_down & KEY_ZR) || (key_down & KEY_R) || (key_down & KEY_SR)) {
		this->signals = this->signals | SIG_NEXT;
	}// Add more signals here as needed
}

u32 input_handler::get_signals() {
	mutexLock(&this->input_lock);
	condvarWait(&this->input_cv);
	u32 retval = this->signals;
	this->signals = 0;
	mutexUnlock(&this->input_lock);
	return retval;
}