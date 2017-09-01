#ifndef __BASE_H__
#define __BASE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

// #define DEBUG
#define Intel_i7_4790
// #define Intel_e5_2650

/////////////////////////////////////
//    CPU-dependent Information    //
#ifdef Intel_i7_4790						// for Intel Core i7-4790
	#define L3_CACHE_SIZE	(8388608)		// $ getconf LEVEL3_CACHE_SIZE
	#define L3_CACHE_LINE_SIZE	(64)		// $ getconf LEVEL3_CACHE_LINESIZE
	#define L3_CACHE_ASSOC	(16)			// $ getconf LEVEL3_CACHE_ASSOC
	#define L3_CACHE_N_SLICES	(4)			// $ grep 'cpu cores' /proc/cpuinfo | tail -1 | awk '{print $NF}'
	#define L3_CACHE_N_SETS	(L3_CACHE_SIZE / (L3_CACHE_LINE_SIZE * L3_CACHE_ASSOC * L3_CACHE_N_SLICES))
#else
	#ifdef Intel_e5_2650					// for Intel Xeon e5-2650
		#define L3_CACHE_SIZE	(20971520)	// $ getconf LEVEL3_CACHE_SIZE
		#define L3_CACHE_LINE_SIZE	(64)	// $ getconf LEVEL3_CACHE_LINESIZE
		#define L3_CACHE_ASSOC	(20)		// $ getconf LEVEL3_CACHE_ASSOC
		#define L3_CACHE_N_SLICES	(8)		// $ grep 'cpu cores' /proc/cpuinfo | tail -1 | awk '{print $NF}'
		#define L3_CACHE_N_SETS	(L3_CACHE_SIZE / (L3_CACHE_LINE_SIZE * L3_CACHE_ASSOC * L3_CACHE_N_SLICES))
	#endif
#endif
/////////////////////////////////////

/////////////////////////////////////
//    CPU-independent Information  //
#ifdef DEBUG
	#define DEBUG_PRINT(fmt, args...)	printf(fmt, ##args)
	#define DEBUG_PRINT_LINE(fmt, args...)	printf("[%s L:%d]\t" fmt, __FILE__, __LINE__, ##args)
#else
	#define DEBUG_PRINT(fmt, args...)
	#define DEBUG_PRINT_LINE(fmt, args...)
#endif
#define N_CACHE_SET_INDICES_MAX	(L3_CACHE_N_SETS)
#define THRESHOLD_RANGE	(20)
#define N_TRIES	(100)
#define N_PROBES	(100)
/////////////////////////////////////

typedef struct memory {
	size_t SIZE;
	size_t LINE_SIZE;
	size_t N_CACHE_SETS;
	int SHM_ID;
	uint64_t *ADDR;
} s_memory;

typedef struct set {
	size_t MAX, n_elements;
	uint64_t *elements;
} s_set;

typedef struct table {
	uint64_t element;
	uint64_t cycle;
} s_table;

extern s_memory MEMORY;
extern size_t BASE_CACHE_INDEX;
extern size_t N_CACHE_SET_INDICES;

/* functions for init... */
int init_memory(const key_t key);
s_set *init_set(const size_t max);
s_set **init_sets(const size_t n, const size_t max);

/* Functions for free... */
void free_memory(void);
void free_set(s_set *set);
void free_sets(s_set **sets, const size_t n);

/* Functions for print... */
void print_cache_info(void);
void print_memory_info(void);
void print_set_info(const s_set *set);

/* Functions for set operation */
int is_element_in_set(const s_set *set, const uint64_t element);
int insert_element(s_set *set, const uint64_t element);
int delete_element(s_set *set, const uint64_t element);
int randomize_elements(s_set *set);
s_set *get_union_set(const s_set *set_A, const s_set *set_B);
s_set *get_difference_set(const s_set *set_A, const s_set *set_B);
s_set *get_intersection_set(const s_set *set_A, const s_set *set_B);
s_set *copy_set(const s_set *set_src);

/* Functions for attack */
uint64_t get_cycle(void);
s_set **get_potentially_conflict_sets(size_t *n_sets);
s_set *get_conflict_set(const s_set *lines);
s_set **get_eviction_sets(const s_set *lines, const s_set *conflict_set, size_t *n_sets);
uint64_t probe_memory_line_with_set(const s_set *set, const uint64_t candidate);
uint64_t probe_memory_line_in_set(const s_set *set, const uint64_t candidate);
int compare_cycles(const void *first, const void *second);
int compare_cycles_of_table(const void *first, const void *second);

#endif