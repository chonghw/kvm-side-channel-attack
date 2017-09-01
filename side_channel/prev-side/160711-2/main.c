#include "set.h"
#include "memory.h"
#include <stdint.h>
#include <string.h>

#define THRESHOLD	(285)
#define N_PROBES	(3)

s_cache_info cache;
s_memory_info memory;

s_set **potentially_conflict_sets = NULL;
size_t n_potentially_conflict_sets = 0;

// s_set **eviction_sets = NULL;
// size_t n_eviction_sets = 0;

s_set *eviction_set = NULL;

int probe(const s_set *set, const void *candidate) {
	s_element *l;
	size_t data;
	size_t a, d;
	size_t ts;
	size_t i;
	size_t cnt;

	if(!set)
		return 0;

	printf("Probing %p:", candidate);
	cnt = 0;
	for(i = 0; i < N_PROBES; i++) {
		// read candidate
		data = *(size_t *)candidate;
		// foreach l in set do
		for(l = set->elements; l != NULL; l = l->next)
			// read l 
			data = *(size_t *)get_addr(l);
		// measure time to read candidate
		asm volatile ("cpuid; rdtsc" : "=a" (a), "=d" (d) : : "ebx", "ecx");
		ts = (a | ((uint64_t)d << 32));
		data = *(size_t *)candidate;
		asm volatile ("cpuid; rdtsc" : "=a" (a), "=d" (d) : : "ebx", "ecx");
		ts = (a | ((uint64_t)d << 32)) - ts;
		// return time > threshold
		printf("\t%lu", ts);
		if(ts > THRESHOLD)
			++cnt;
	}
	printf("\t(%lu/%lu)", cnt, (size_t)N_PROBES);
	if(cnt > 0)
		printf("\t(Cache miss)");
	printf("\n");
	return (cnt > 0);
}

int randomize_lines(s_set *set) {
	s_element *e1, *e2;
	void *addr_tmp;
	size_t r1, r2;
	size_t tmp;
	size_t i;

	if(!set)
		return -1;

	for(i = 0; i < set->n_elements; i++) {
		r1 = rand() % set->n_elements;
		r2 = rand() % set->n_elements;
		if(r1 == r2)
			continue;
		else if(r1 > r2) {
			tmp = r1;
			r1 = r2;
			r2 = tmp;
		}

		e1 = get_element_by_idx(set, r1);
		e2 = get_element_by_idx(set, r2);
		addr_tmp = get_addr(e1);
		set_addr(e1, get_addr(e2));
		set_addr(e2, addr_tmp);
	}

	return 0;
}

int init_relative_complement(s_set *set_A, const s_set *set_B) {	// Result = A - B
	s_element *element;
	s_element *i;

	if(!set_A || !set_B)
		return -1;

	for(i = set_B->elements; i != NULL; i = i->next)
		delete_element_by_addr(set_A, get_addr(i));

	return 0;
}

int init_potentially_conflict_sets(void) {
	s_element *element;
	size_t N_BITS_UPPER = 0;		// bit index 17-?
	size_t N_BITS_SET_INDEX = 11;	// bit index 6-16
	size_t N_BITS_LOWER = 6;		// bit index 0-5
	size_t offset;
	size_t i, j, k;

	for(i = memory.size >> (N_BITS_SET_INDEX + N_BITS_LOWER); i > 0; i = i >> 1)
		++N_BITS_UPPER;

	n_potentially_conflict_sets = 1 << N_BITS_SET_INDEX;
	potentially_conflict_sets = (s_set **)malloc(sizeof(s_set *) * n_potentially_conflict_sets);
	if(!potentially_conflict_sets)
		return -1;
	memset(potentially_conflict_sets, 0, sizeof(s_set *) * n_potentially_conflict_sets);

	for(i = 0; i < n_potentially_conflict_sets; i++) {
		potentially_conflict_sets[i] = create_set();
		for(j = 0; j < (1 << N_BITS_UPPER); j++) {
			for(k = 0; k < (1 << N_BITS_LOWER); k += memory.line_size) {
				offset = (j << (N_BITS_SET_INDEX + N_BITS_LOWER)) + (i << N_BITS_LOWER) + k;
				if(offset >= memory.size)
					continue;
				insert_element_by_addr(potentially_conflict_sets[i], (void *)((size_t)memory.ptr + offset));
			}
		}
	}

	return 0;
}

int free_potentially_conflict_sets(void) {
	size_t i;

	for(i = 0; i < n_potentially_conflict_sets; i++)
		free_set(&potentially_conflict_sets[i]);
	potentially_conflict_sets = NULL;
	n_potentially_conflict_sets = 0;

	return 0;
}

int init_eviction_set(const size_t set_index) {
	s_set *lines, *conflict_set, *set_tmp, *set_tmp2;
	s_element *candidate, *l, *element;

	// randomize lines
	lines = potentially_conflict_sets[set_index];
	randomize_lines(lines);
	// conflict_set <- {}
	conflict_set = create_set();
	// foreach candidate in lines do
	for(candidate = lines->elements; candidate != NULL; candidate = candidate->next)
		// if not probe(conflict_set, candidate) then
		if(!probe(conflict_set, get_addr(candidate)))
			// insert candidate into conflict_set
			insert_element_by_addr(conflict_set, get_addr(candidate));
	print_set(conflict_set, "* Conflict set");
	// // foreach candidate in lines - conflict_set do
	// set_tmp = copy_set(lines);
	// init_relative_complement(set_tmp, conflict_set);
	// for(candidate = set_tmp->elements; candidate != NULL; candidate = candidate->next) {
	// 	// if probe(conflict_set, candidate) then
	// 	if(probe(conflict_set, get_addr(candidate))) {
	// 		// eviction_set <- {}
	// 		eviction_set = create_set();
	// 		// foreach l in conflict_set do
	// 		for(l = conflict_set->elements; l != NULL; l = l->next) {
	// 			// if not probe(conflict_set - {l}, candidate) then
	// 			set_tmp2 = copy_set(conflict_set);
	// 			delete_element_by_addr(set_tmp2, get_addr(l));
	// 			if(!probe(set_tmp2, get_addr(candidate)))
	// 				// insert l into eviction_set
	// 				insert_element_by_addr(eviction_set, get_addr(l));
	// 			free_set(&set_tmp2);
	// 		}
	// 		// output eviction_set
	// 		if(eviction_set->n_elements) {
	// 			print_set(eviction_set, "* Eviction set");
	// 			getchar();
	// 		}
	// 		// conflict_set <- conflict_set - eviction_set
	// 		init_relative_complement(conflict_set, eviction_set);
	// 		free_set(&eviction_set);
	// 	}
	// }
	// free_set(&set_tmp);
	free_set(&conflict_set);
}

int init_eviction_sets(void) {
	size_t i;

	// n_eviction_sets = n_potentially_conflict_sets;
	// eviction_sets = (s_set **)malloc(sizeof(s_set *) * n_eviction_sets);
	// if(!eviction_sets)
	// 	return -1;
	// memset(eviction_sets, 0, sizeof(s_set *) * n_eviction_sets);

	for(i = 0; i < n_potentially_conflict_sets; i++)
		init_eviction_set(i);

	return 0;
}

int free_eviction_sets(void) {
	// size_t i;

	// if(eviction_sets == NULL)
	// 	return 0;

	// for(i = 0; i < n_eviction_sets; i++)
	// 	free_set(&eviction_sets[i]);
	// eviction_sets = NULL;
	// n_eviction_sets = 0;

	return 0;
}

int main(const int argc, const char *argv[]) {
	srand(time(NULL));

	/* Steps for preparation */
	init_cache_info(&cache);
	print_cache_info(cache);
	init_memory_info(&memory, cache);
	print_memory_info(memory);
	init_potentially_conflict_sets();
	// init_eviction_sets();
	init_eviction_set(0xFF);

	/* Steps for attack */
	// while(1) {
	// 	PRIME();
	// 	IDLE();
	// 	probe();
	// }

	/* Steps for finalization */
	free_memory_info(&memory);
	free_potentially_conflict_sets();
	// free_eviction_sets();
	free_set(&eviction_set);

	return 0;
}