#include "attack.h"
#include <signal.h>

int MODE = 0;
char DATA[2 << 15] = {0};
size_t N_DATA = 0;

void sig_handler(int sig);
int ready(const int argc, char *argv[]);
int start(void);
void stop(void);

void sig_handler(int sig) {
	stop();
	exit(0);
}

int ready(const int argc, char *argv[]) {
	int opt;
	size_t i;

	srand(time(NULL));
	signal(SIGINT, sig_handler);

	while((opt = getopt(argc, argv, "b:c:d:so")) != -1) {
		switch(opt){
		case 'b':	// Set BASE_CACHE_INDEX
			BASE_CACHE_SET_INDEX = atoi(optarg);
			break;
		case 'c':	// Set N_CACHE_SETS
			N_CACHE_SETS = atoi(optarg);
			break;
		case 'd':	// Set DATA
			N_DATA = strlen(optarg);
			if(N_DATA > sizeof(DATA))
				N_DATA = sizeof(DATA);
			strncpy(DATA, optarg, N_DATA);
			break;
		case 's':	// Set MODE as Side-channel listener
			MODE = 1;
			break;
		case 'o':	// Set MODE as Covert-channel sender
			MODE = 2;
			break;
		}
	}
	if(MODE == 0 || MODE == 2)
		N_CACHE_SETS = 3;
	if(MODE == 2) {
		for(i = 0; i < N_DATA; i++)
			if(DATA[i] != '0' && DATA[i] != '1')
				break;
		if(i != N_DATA) {
			printf("\tError: Data format\n");
			return -1;
		}
	}

	if(N_CACHE_SETS < 1)
		N_CACHE_SETS = 1;
	else if(N_CACHE_SETS > L3_CACHE_N_SETS)
		N_CACHE_SETS = L3_CACHE_N_SETS;
	if(BASE_CACHE_SET_INDEX + N_CACHE_SETS > L3_CACHE_N_SETS)
		BASE_CACHE_SET_INDEX = L3_CACHE_N_SETS - N_CACHE_SETS;

	return 0;
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
			if(start_send(DATA, N_DATA) < 0) {
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
	if(ready(argc, argv) < 0) {
		DEBUG_PRINT("ready()\n");
		return -1;
	}

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