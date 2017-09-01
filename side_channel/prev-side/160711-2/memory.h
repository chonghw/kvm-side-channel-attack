#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct cache {
	size_t size;
	size_t line_size;
	size_t n_ways;
	size_t n_sets;
} s_cache_info;

typedef struct memory {
	void *ptr;
	size_t size;
	size_t line_size;
	size_t n_lines;
} s_memory_info;

int init_cache_info(s_cache_info *cache);
void print_cache_info(const s_cache_info cache);

int init_memory_info(s_memory_info *memory, const s_cache_info cache);
int free_memory_info(s_memory_info *memory);
void print_memory_info(const s_memory_info memory);

#endif