#include "base.h"

s_memory MEMORY = {0};
// size_t BASE_CACHE_INDEX = 0;
// size_t N_CACHE_SET_INDICES = 1;
uint64_t THRESHOLD = 0;

int init_memory(const key_t key) {
	// < 0 :	Error
	// == 0 :	Success
	size_t i;

	DEBUG_PRINT("* Initializing memories...\n");
	MEMORY.SIZE = L3_CACHE_SIZE * 2;
	MEMORY.LINE_SIZE = L3_CACHE_LINE_SIZE;
	MEMORY.N_CACHE_SETS = MEMORY.SIZE / MEMORY.LINE_SIZE;
	if((MEMORY.SHM_ID = shmget(key, MEMORY.SIZE, SHM_HUGETLB | IPC_CREAT | SHM_R | SHM_W)) < 0)
		return -1;
	if((MEMORY.ADDR = (uint64_t *)shmat(MEMORY.SHM_ID, 0, 0)) == (uint64_t *)(-1)) {
		free_memory();
		return -1;
	}

	#ifdef DEBUG
		print_cache_info();
		print_memory_info();
	#endif

	return 0;
}

int init_memory_for_child(const key_t key) {
	// < 0 :	Error
	// == 0 :	Success
	if((MEMORY.SHM_ID = shmget(key, MEMORY.SIZE, 0)) < 0)
		return -1;
	if((MEMORY.ADDR = (uint64_t *)shmat(MEMORY.SHM_ID, 0, SHM_HUGETLB | IPC_CREAT | SHM_R | SHM_W)) == (uint64_t *)(-1))
		return -1;

	return 0;
}

s_set *init_set(const size_t max) {
	// NULL :	Error
	// !NULL	: Success
	s_set *set;

	if(max <= 0)								return NULL;
	if(!(set = (s_set *)malloc(sizeof(s_set))))	return NULL;
	if(!(set->elements = (uint64_t *)malloc(sizeof(uint64_t) * max))) {
		free(set);								return NULL;}
	set->MAX = max;
	set->n_elements = 0;

	return set;
}

s_set **init_sets(const size_t n, const size_t max) {
	s_set **sets;
	size_t i;

	if(!(sets = (s_set **)malloc(sizeof(s_set *) * n)))
		return NULL;

	for(i = 0; i < n; i++)
		sets[i] = init_set(max);

	return sets;
}

void free_memory(void) {
	shmctl(MEMORY.SHM_ID, IPC_RMID, NULL);
}

void free_set(s_set *set) {
	free(set->elements);
	free(set);
}

void free_sets(s_set **sets, const size_t n) {
	size_t i;

	if(!sets)
		return;

	for(i = 0; i < n; i++)
		free_set(sets[i]);
	free(sets);
}

void print_cache_info(void) {
	size_t i, j;

	DEBUG_PRINT("* Cache information\n");
	DEBUG_PRINT("\t. Size: %lu Bytes\n", (size_t)L3_CACHE_SIZE);
	DEBUG_PRINT("\t. Map:\n\t\t\t");
	for(i = 0; i < L3_CACHE_ASSOC; i++) {
		if(L3_CACHE_ASSOC > 4 && i > 1 && i < L3_CACHE_ASSOC - 1) {
			if(i == 2)
				DEBUG_PRINT(" ......  ");
			continue;
		}
		DEBUG_PRINT(" way-%-2lu  ", i);
	}
	DEBUG_PRINT("\n\t\t\t");
	for(i = 0; i < L3_CACHE_ASSOC; i++) {
		if(L3_CACHE_ASSOC > 4 && i > 1 && i < L3_CACHE_ASSOC - 1) {
			if(i == 2)
				DEBUG_PRINT("         ");
			continue;
		}
		DEBUG_PRINT("+------+ ");
	}
	DEBUG_PRINT("\n");
	for(i = 0; i < L3_CACHE_N_SETS / L3_CACHE_N_SLICES; i++) {
		if(L3_CACHE_N_SETS > 4 && i > 1 && i < (L3_CACHE_N_SETS / L3_CACHE_N_SLICES) - 1) {
			if(i == 2) {
				DEBUG_PRINT("\t\t......\n\t\t\t");
				for(j = 0; j < L3_CACHE_ASSOC; j++) {
					if(L3_CACHE_ASSOC > 4 && j > 1 && j < L3_CACHE_ASSOC - 1) {
						if(j == 2)
							DEBUG_PRINT("         ");
						continue;
					}
					DEBUG_PRINT("+------+ ");
				}
				DEBUG_PRINT("\n");
			}
			continue;
		}
		DEBUG_PRINT("\t\t%6lu\t", i);
		for(j = 0; j < L3_CACHE_ASSOC; j++) {
			if(L3_CACHE_ASSOC > 4 && j > 1 && j < L3_CACHE_ASSOC - 1) {
				if(j == 2)
					DEBUG_PRINT("         ");
				continue;
			}
			DEBUG_PRINT("| %4lu | ", (size_t)L3_CACHE_LINE_SIZE);
		}
		if(i == 0)
			DEBUG_PRINT("= %lu", (size_t)(L3_CACHE_LINE_SIZE * L3_CACHE_ASSOC));
		DEBUG_PRINT("\n\t\t\t");
		for(j = 0; j < L3_CACHE_ASSOC; j++) {
			if(L3_CACHE_ASSOC > 4 && j > 1 && j < L3_CACHE_ASSOC - 1) {
				if(j == 2)
					DEBUG_PRINT("         ");
				continue;
			}
			DEBUG_PRINT("+------+ ");
		}
		DEBUG_PRINT("\n");
	}
	DEBUG_PRINT("\t\t\t = %lu\n", (size_t)(L3_CACHE_LINE_SIZE * (L3_CACHE_N_SETS / L3_CACHE_N_SLICES)));
	DEBUG_PRINT("\t\tx %lu Slices\n", (size_t)L3_CACHE_N_SLICES);
}

void print_memory_info(void) {
	size_t i;

	DEBUG_PRINT("* Memory information\n");
	DEBUG_PRINT("\t. Size: %lu Bytes\n", MEMORY.SIZE);
	DEBUG_PRINT("\t. Map:\n\t\t\t+------+\n");
	for(i = 0; i < MEMORY.N_CACHE_SETS; i++) {
		if(MEMORY.N_CACHE_SETS > 4 && i > 1 && i < (MEMORY.N_CACHE_SETS - 1)) {
			if(i == 2)
				DEBUG_PRINT("\t\t......\n\t\t\t+------+\n");
			continue;
		}
		DEBUG_PRINT("\t\t%7lu\t", i);
		DEBUG_PRINT("| %4lu | ", MEMORY.LINE_SIZE);
		DEBUG_PRINT("(%p)", MEMORY.ADDR + MEMORY.LINE_SIZE * i);
		DEBUG_PRINT("\n\t\t\t+------+\n");
	}
	DEBUG_PRINT("\t\t\t = %lu\n", MEMORY.SIZE);
}

void print_set_info(const s_set *set) {
	size_t i;

	if(!set)
		return;

	DEBUG_PRINT("* Set information\n");
	DEBUG_PRINT("\t. Number of elements:\t%lu Elements\n", set->n_elements);
	DEBUG_PRINT("\t. Elements");
	for(i = 0; i < set->n_elements; i++) {
		if(i % 4 == 0)
			DEBUG_PRINT("\n\t");
		DEBUG_PRINT("\t0x%lx", set->elements[i]);
	}
	DEBUG_PRINT("\n");
}

int is_element_in_set(const s_set *set, const uint64_t element) {
	// < 0 :	Error or not exist
	// >= 0 :	Index of the element
	size_t i;

	if(!set)
		return -1;

	for(i = 0; i < set->n_elements; i++)
		if(set->elements[i] == element)
			return i;

	return -1;
}

int insert_element(s_set *set, const uint64_t element) {
	// < 0 :	Error
	// == 0 :	Success
	if(!set)									return -1;
	if(set->n_elements >= set->MAX)				return -1;
	if(is_element_in_set(set, element) >= 0)	return -1;

	set->elements[set->n_elements] = element;
	++set->n_elements;

	return 0;
}

int delete_element(s_set *set, const uint64_t element) {
	// < 0 :	Error
	// == 0 :	Success
	size_t idx, i;

	if(!set)										return -1;
	if(set->n_elements <= 0)						return -1;
	if((idx = is_element_in_set(set, element)) < 0)	return -1;

	for(i = idx; i < (set->n_elements - 1); i++)
		set->elements[i] = set->elements[i + 1];
	--set->n_elements;

	return 0;
}

int randomize_elements(s_set *set) {
	// < 0 :	Error
	// == 0 :	Success
	#define N_RANDOMIZE	(100)
	uint64_t memory_line;
	size_t r1, r2, tmp;
	size_t i;

	if(!set)
		return -1;

	srand(time(NULL));
	for(i = 0; i < N_RANDOMIZE; i++) {
		r1 = rand() % set->n_elements;
		r2 = rand() % set->n_elements;
		if(r1 == r2)
			continue;
		else if(r1 > r2) {
			tmp = r1;
			r1 = r2;
			r2 = tmp;
		}

		memory_line = set->elements[r1];
		set->elements[r1] = set->elements[r2];
		set->elements[r2] = memory_line;
	}

	return 0;
}

s_set *get_union_set(const s_set *set_A, const s_set *set_B) {
	// NULL :	Error
	// !NULL :	Difference set
	s_set *set;
	size_t i;

	if(!set_A || !set_B)			return NULL;
	if(!(set = copy_set(set_A)))	return NULL;

	for(i = 0; i < set_B->n_elements; i++)
		insert_element(set, set_B->elements[i]);

	return set;
}

s_set *get_difference_set(const s_set *set_A, const s_set *set_B) {
	// NULL :	Error
	// !NULL :	Difference set
	s_set *set;
	size_t i;

	if(!set_A || !set_B)			return NULL;
	if(!(set = copy_set(set_A)))	return NULL;

	for(i = 0; i < set_B->n_elements; i++)
		delete_element(set, set_B->elements[i]);

	return set;
}

s_set *get_intersection_set(const s_set *set_A, const s_set *set_B) {
	// NULL :	Error
	// !NULL :	Intersection set
	s_set *set;
	size_t min, max;
	size_t i;

	min = set_A->n_elements < set_B->n_elements ? set_A->n_elements : set_B->n_elements;
	max = set_A->n_elements > set_B->n_elements ? set_A->n_elements : set_B->n_elements;
	if(!set_A || !set_B)		return NULL;
	if(!(set = init_set(min)))	return NULL;

	for(i = 0; i < set_A->n_elements; i++)
		if(is_element_in_set(set_B, set_A->elements[i]) >= 0)
			insert_element(set, set_A->elements[i]);

	return set;
}

s_set *copy_set(const s_set *set_src) {
	// NULL :	Error
	// !NULL :	Copied set
	s_set *set_dst;
	size_t i;

	if(!set_src)							return NULL;
	if(!(set_dst = init_set(set_src->MAX)))	return NULL;

	for(i = 0; i < set_src->n_elements; i++)
		if(insert_element(set_dst, set_src->elements[i]) < 0) {
			free_set(set_dst);
			return NULL;
		}

	return set_dst;
}

uint64_t get_cycle(void) {
	uint64_t a, d;
	
	asm volatile ("cpuid; rdtsc" : "=a" (a), "=d" (d) : : "ebx", "ecx");

	return (a | ((uint64_t)d << 32));
}

s_set **get_potentially_conflict_sets(size_t *n_sets) {
	// NULL :	Error
	// !NULL :	Success
	#define N_BITS_SET_INDEX	(11)	// bit index 6-16
	#define N_BITS_LOWER		(6)		// bit index 0-5
	size_t N_BITS_UPPER = 0;			// bit index 17-?
	s_set **sets;
	size_t offset;
	size_t i, j, k;

	if(!n_sets)
		return NULL;

	for(i = MEMORY.SIZE >> (N_BITS_SET_INDEX + N_BITS_LOWER); i > 0; i = i >> 1)
		++N_BITS_UPPER;
	*n_sets = 1 << N_BITS_SET_INDEX;
	if(!(sets = init_sets(*n_sets, 1 << N_BITS_UPPER)))
		return NULL;

	DEBUG_PRINT("* Finding potentially conflict sets...\n");
	DEBUG_PRINT("\t. Number of potentially conflict sets: %lu Sets\n", *n_sets);
	for(i = 0; i < *n_sets; i++) {
		for(j = 0; j < (1 << N_BITS_UPPER); j++) {
			for(k = 0; k < (1 << N_BITS_LOWER); k += MEMORY.LINE_SIZE) {
				offset = (j << (N_BITS_SET_INDEX + N_BITS_LOWER)) + (i << N_BITS_LOWER) + k;
				if(offset >= MEMORY.SIZE)
					continue;
				insert_element(sets[i], (uint64_t)MEMORY.ADDR + offset);
			}
		}
	}

	return sets;
}

s_set *get_conflict_set(const s_set *lines) {
	// NULL :	Error
	// !NULL :	Success
	s_set *conflict_set, *lines_randomized;
	s_table *table;
	uint64_t candidate;
	size_t i, n, cnt;

	if(!lines)									return NULL;
	if(!(lines_randomized = copy_set(lines))) 	return NULL;
	if(!(table = (s_table *)malloc(sizeof(s_table) * lines_randomized->n_elements))) {
		free_set(lines_randomized);				return NULL;}
	if(!(conflict_set = init_set(lines->n_elements))) {
		free(table);
		free_set(lines_randomized);				return NULL;}

	DEBUG_PRINT("* Finding a conflict set...\n");
	n = lines_randomized->n_elements;
	cnt = 0;
	while(1) {
		++cnt;
		if(cnt > N_TRIES) {
			free(table);
			free_set(lines_randomized);
			free_set(conflict_set);
			return NULL;
		}

		randomize_elements(lines_randomized);
		conflict_set->n_elements = 0;
		for(i = 0; i < n; i++) {
			candidate = lines_randomized->elements[i];
			table[i].element = candidate;
			table[i].cycle = probe_memory_line_with_set(conflict_set, candidate);
			insert_element(conflict_set, candidate);
		}

		qsort(table, n, sizeof(s_table), compare_cycles_of_table);
		THRESHOLD = table[(n - 1) / 4].cycle;
		for(i = 0; i < n; i++)
			if(table[i].cycle > THRESHOLD && table[i].cycle < THRESHOLD + THRESHOLD_RANGE)
				THRESHOLD = table[i].cycle;

		conflict_set->n_elements = 0;
		DEBUG_PRINT("\t. Probing information\n");
		for(i = 0; i < n; i++) {
			DEBUG_PRINT("\t\t[%lu]\t%p\t%lu", i, (uint64_t *)(table[i].element), table[i].cycle);
			if(table[i].cycle <= THRESHOLD) {
				DEBUG_PRINT("\t(Inserted into a conflict set)");
				insert_element(conflict_set, table[i].element);
			}
			DEBUG_PRINT("\n");
		}
		DEBUG_PRINT("\t. Number of elements: %lu\n", conflict_set->n_elements);
		if(conflict_set->n_elements == (L3_CACHE_ASSOC * L3_CACHE_N_SLICES))
			if(table[conflict_set->n_elements].cycle > THRESHOLD + (2 * THRESHOLD_RANGE)) {
				DEBUG_PRINT("\t\t(Found a conflict set with %lu tries)\n", cnt);
				break;
			}
		DEBUG_PRINT("\t\t(Failed to find a conflict set)\n");
	}
	free(table);
	free_set(lines_randomized);

	THRESHOLD += THRESHOLD_RANGE;
	DEBUG_PRINT("\t. Threshold: %lu\n", THRESHOLD);

	return conflict_set;
}

s_set **get_eviction_sets(const s_set *lines, const s_set *conflict_set, size_t *n_sets) {
	// NULL :	Error
	// !NULL :	Success
	s_set **eviction_sets, *difference_lines, *conflict_set_, *tmp;
	size_t n_eviction_sets;
	uint64_t candidate, l;
	uint64_t cycle;
	size_t i, j, k, n ,n2, cnt;
	s_table *table;
	uint64_t tHRESHOLD;

	n_eviction_sets = L3_CACHE_N_SLICES;
	if(!lines || !conflict_set || !n_sets)								return NULL;
	if(!(difference_lines = get_difference_set(lines, conflict_set)))	return NULL;
	if(!(conflict_set_ = copy_set(conflict_set))) {
		free_set(difference_lines);										return NULL;}
	if(!(eviction_sets = init_sets(n_eviction_sets, conflict_set->n_elements))) {
		free_set(difference_lines);
		free_set(conflict_set_);										return NULL;}
	if(!(table = (s_table *)malloc(sizeof(s_table) * conflict_set_->n_elements))) {
		free_sets(eviction_sets, n_eviction_sets);
		free_set(difference_lines);
		free_set(conflict_set_);										return NULL;}

	DEBUG_PRINT("* Finding eviction sets...\n");
	n = difference_lines->n_elements;
	i = k = 0;
	cnt = 0;
	while(1) {
		if((++i) >= n) {
			randomize_elements(difference_lines);
			i = 0;
		}

		candidate = difference_lines->elements[i];
		cycle = probe_memory_line_with_set(conflict_set_, candidate);
		DEBUG_PRINT("\t\t%p\t%lu", (uint64_t *)candidate, cycle);
		if(cycle > THRESHOLD) {
			DEBUG_PRINT("\t(Conflicted)\n");

			get_eviction_set_again:
			if((++cnt) > N_TRIES) {
				free_set(difference_lines);
				free(table);
				free_set(conflict_set_);
				free_sets(eviction_sets, n_eviction_sets);
				return NULL;
			}
			eviction_sets[k]->n_elements = 0;
			n2 = conflict_set_->n_elements;
			tmp = copy_set(conflict_set_);
			for(j = 0; j < n2; j++) {
				l = conflict_set_->elements[j];
				delete_element(tmp, l);
				table[j].element = l;
				table[j].cycle = probe_memory_line_with_set(tmp, candidate);
				insert_element(tmp, l);
			}
			free_set(tmp);

			qsort(table, n2, sizeof(s_table), compare_cycles_of_table);
			tHRESHOLD = table[0].cycle;
			for(j = 1; j < n2; j++)
				if(table[j].cycle > tHRESHOLD && table[j].cycle < tHRESHOLD + THRESHOLD_RANGE)
					tHRESHOLD = table[j].cycle;
			for(j = 0; j < n2; j++) {
				DEBUG_PRINT("\t\t\t[%lu]\t%p\t%lu", j, (uint64_t *)(table[j].element), table[j].cycle);
				if(table[j].cycle <= tHRESHOLD) {
					DEBUG_PRINT("\t(Inserted into eviction set #%lu)", k);
					insert_element(eviction_sets[k], table[j].element);
				}
				DEBUG_PRINT("\n");
			}
			DEBUG_PRINT("\t\t\t. Number of elements: %lu\n", eviction_sets[k]->n_elements);

			tmp = get_difference_set(conflict_set_, eviction_sets[k]);
			free_set(conflict_set_);
			if(eviction_sets[k]->n_elements != L3_CACHE_ASSOC) {
				DEBUG_PRINT("\t\t\t(Failed to find eviction set #%lu)\n", k);
				conflict_set_ = get_union_set(tmp, eviction_sets[k]);
				goto get_eviction_set_again;
			}
			if((++k) == n_eviction_sets)
				break;
			conflict_set_ = tmp;
			cnt = 0;
		}
		else
			DEBUG_PRINT("\n");
	}
	free_set(difference_lines);
	free(table);

	*n_sets = n_eviction_sets;

	return eviction_sets;
}

uint64_t probe_memory_line_with_set(const s_set *set, const uint64_t candidate) {
	uint64_t cycle[N_PROBES];
	uint64_t data;
	uint64_t *elements;
	size_t i, j, n;

	if(!set)
		return 0;

	n = set->n_elements;
	elements = set->elements;
	for(i = 0; i < N_PROBES; i++) {
		*(uint64_t *)candidate = 0x1;
		data = *(uint64_t *)candidate;
		*(uint64_t *)candidate = 0x1;
		data = *(uint64_t *)candidate;

		for(j = 0; j < n; j++) {
			*(uint64_t *)elements[j] = 0x1;
			data = *(uint64_t *)elements[j];
			*(uint64_t *)elements[j] = 0x1;
			data = *(uint64_t *)elements[j];
		}

		cycle[i] = get_cycle();

		*(uint64_t *)candidate = 0x1;

		cycle[i] = get_cycle() - cycle[i];
	}
	qsort(cycle, N_PROBES, sizeof(uint64_t), compare_cycles);

	return cycle[N_PROBES / 2];
}

uint64_t probe_memory_line_in_set(const s_set *set, const uint64_t candidate) {
	uint64_t cycle[N_PROBES];
	uint64_t data;
	uint64_t *elements;
	size_t i, j, n, idx;

	if(!set)
		return 0;

	idx = is_element_in_set(set, candidate);
	n = set->n_elements;
	elements = set->elements;
	for(i = 0; i < N_PROBES; i++) {
		*(uint64_t *)candidate = 0x1;
		data = *(uint64_t *)candidate;
		*(uint64_t *)candidate = 0x1;
		data = *(uint64_t *)candidate;

		for(j = 0; j < n; j++)
			if(j != idx) {
				*(uint64_t *)elements[j] = 0x1;
				data = *(uint64_t *)elements[j];
				*(uint64_t *)elements[j] = 0x1;
				data = *(uint64_t *)elements[j];
			}

		cycle[i] = get_cycle();

		*(uint64_t *)candidate = 0x1;

		cycle[i] = get_cycle() - cycle[i];
	}
	qsort(cycle, N_PROBES, sizeof(uint64_t), compare_cycles);

	return cycle[N_PROBES / 2];
}

int compare_cycles(const void *first, const void *second) {
	if(*(uint64_t *)first > *(uint64_t *)second)		return 1;
	else if(*(uint64_t *)first < *(uint64_t *)second)	return -1;
														return 0;
}

int compare_cycles_of_table(const void *first, const void *second) {
	s_table table_first = *(s_table *)first;
	s_table table_second = *(s_table *)second;
	if(table_first.cycle > table_second.cycle)		return 1;
	else if(table_first.cycle < table_second.cycle)	return -1;
													return 0;
}