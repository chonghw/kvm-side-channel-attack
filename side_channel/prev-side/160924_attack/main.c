#include "attack.h"
#include <signal.h>

int MODE = 0;

void sig_handler(int sig);
void ready(const int argc, char *argv[]);
int start(void);
void stop(void);

void sig_handler(int sig) {
	stop();
	exit(0);
}

void ready(const int argc, char *argv[]) {
	int opt;

	srand(time(NULL));
	signal(SIGINT, sig_handler);

	while((opt = getopt(argc, argv, "b:c:so")) != -1) {
		switch(opt){
		case 'b':	// Set BASE_CACHE_INDEX
			base_cache_set_index = atoi(optarg);
			break;
		case 'c':	// Set N_CACHE_SETS
			n_cache_sets = atoi(optarg);
			break;
		case 's':	// Set MODE as Side-channel listener
			MODE = 1;
			break;
		case 'o':	// Set MODE as Covert-channel sender
			MODE = 2;
			break;
		}
	}
	if(n_cache_sets < 1)
		n_cache_sets = 1;
	else if(n_cache_sets > N_CACHE_SET_INDICES_MAX)
		n_cache_sets = N_CACHE_SET_INDICES_MAX;
	if(base_cache_set_index + n_cache_sets > N_CACHE_SET_INDICES_MAX)
		base_cache_set_index = N_CACHE_SET_INDICES_MAX - n_cache_sets;
}

int start(void) {
	// < 0 :	Error
	// == 0 :	Success

	switch(MODE) {
		case 0:	// Covert-channel sender
			if(init_listener() < 0) {
				DEBUG_PRINT("\tinit_listener()\n");
				return -1;
			}
			if(start_listen() < 0) {
				DEBUG_PRINT("\tstart_listen()\n");
				return -1;
			}
			break;
		// case 1:	// Side-channel listener
		// 	init_listener();
		// 	start_listen();
		// 	break;
		case 2:	// Covert-channel sender
			if(init_sender() < 0) {
				DEBUG_PRINT("\tinit_sender()\n");
				return -1;
			}
			if(start_send() < 0) {
				DEBUG_PRINT("\tstart_send()\n");
				return -1;
			}
			break;
		default:
			return -1;
	}

	return 0;
}

void stop(void) {
	switch(MODE) {
		case 0:
			stop_listener();
			break;
		// case 1:
		// 	stop_listener();
		// 	break;
		case 2:
			stop_sender();
			break;
		default:
			break;
	}
}

int main(const int argc, char *argv[]) {
	// Ready
	ready(argc, argv);

	// Start
	if(start() < 0) {
		DEBUG_PRINT("initialize()\n");
		stop();
		return -1;
	}

	// Stop
	stop();
	return 0;
}