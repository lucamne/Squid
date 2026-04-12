#include "engine.h"

#include <process.h>
#include <windows.h>
#include <sys/timeb.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
/// Platform API

int platform_create_thread(void (*func)(void *), void* arg) {
	return _beginthread(func, 0, arg) == -1L;
}

void platform_end_thread(void) {
	_endthread();
}

RB_MUTEX platform_create_mutex(void) {
	HANDLE* handle = malloc(sizeof(HANDLE));
	*handle = CreateMutexW(NULL, 0, NULL);
	return (RB_MUTEX)handle;
}

int platform_wait_mutex(RB_MUTEX handle) {
	HANDLE h = *((HANDLE*)handle);
	return WaitForSingleObject(h, INFINITE) == WAIT_FAILED;
}

int platform_release_mutex(RB_MUTEX handle) {
	HANDLE h = *((HANDLE*)handle);
	return ReleaseMutex(h) == 0;
}

ULL platform_get_time_ms(void) {
	struct timeb t;
	ftime(&t);
	return 1000ull * (ULL)t.time + (ULL)t.millitm;
}

void platform_send_uci_command(const char* cmd) {
	printf("%s", cmd);
}

////////////////////////////////////////////////////////////////////////////////
/// Main Loop

#define INPUT_BUFFER_SIZE 4096
int main(int argc, char** argv) {
	engine_init();
	// scan for new line from stdin and send to engine 
	char buff[INPUT_BUFFER_SIZE];
	int pos = 0;
	while (1) {
		if (pos == INPUT_BUFFER_SIZE) {
			fprintf(stderr, "Line too long. Command not sent.\n");
			// clear remaining chars
			while (fgetc(stdin) != '\n');
			pos = 0;
		}
		buff[pos++] = (char)fgetc(stdin);
		if (buff[pos - 1] == '\n') {
			engine_send_uci_command(buff);
			pos = 0;
		}
	}

	return 0;
}
