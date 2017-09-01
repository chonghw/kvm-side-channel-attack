#include "memory.h"

key_t key = 5959;
int shmid = 0;

int init_cache_info(s_cache_info *cache) {
	if(!cache)
		return -1;

	cache->size = sysconf(_SC_LEVEL3_CACHE_SIZE);
	cache->line_size = sysconf(_SC_LEVEL3_CACHE_LINESIZE);
	cache->n_ways = sysconf(_SC_LEVEL3_CACHE_ASSOC);
	cache->n_sets = cache->size / (cache->line_size * cache->n_ways);

	return 0;
}

void print_cache_info(const s_cache_info cache) {
	size_t i, j;

	printf("* Cache information\n");
	printf("\t. Size: %lu Bytes\n", cache.size);
	printf("\t. Map:\n\t\t\t");
	for(i = 0; i < cache.n_ways; i++) {
		if(cache.n_ways > 4 && i > 1 && i < cache.n_ways - 1) {
			if(i == 2)
				printf(" ......  ");
			continue;
		}
		printf(" way-%-2lu  ", i);
	}
	printf("\n\t\t\t");
	for(i = 0; i < cache.n_ways; i++) {
		if(cache.n_ways > 4 && i > 1 && i < cache.n_ways - 1) {
			if(i == 2)
				printf("         ");
			continue;
		}
		printf("+------+ ");
	}
	printf("\n");
	for(i = 0; i < cache.n_sets; i++) {
		if(cache.n_sets > 4 && i > 1 && i < cache.n_sets - 1) {
			if(i == 2) {
				printf("\t\t......\n\t\t\t");
				for(j = 0; j < cache.n_ways; j++) {
					if(cache.n_ways > 4 && j > 1 && j < cache.n_ways - 1) {
						if(j == 2)
							printf("         ");
						continue;
					}
					printf("+------+ ");
				}
				printf("\n");
			}
			continue;
		}
		printf("\t\t%6lu\t", i);
		for(j = 0; j < cache.n_ways; j++) {
			if(cache.n_ways > 4 && j > 1 && j < cache.n_ways - 1) {
				if(j == 2)
					printf("         ");
				continue;
			}
			printf("| %4lu | ", cache.line_size);
		}
		if(i == 0)
			printf("= %lu", cache.line_size * cache.n_ways);
		printf("\n\t\t\t");
		for(j = 0; j < cache.n_ways; j++) {
			if(cache.n_ways > 4 && j > 1 && j < cache.n_ways - 1) {
				if(j == 2)
					printf("         ");
				continue;
			}
			printf("+------+ ");
		}
		printf("\n");
	}
	printf("\t\t\t = %lu\n", cache.line_size * cache.n_sets);
}

int init_memory_info(s_memory_info *memory, const s_cache_info cache) {
	if(!memory)
		return -1;

	memory->size = cache.size * 2;
	memory->line_size = cache.line_size;
	memory->n_lines = memory->size / memory->line_size;
	if((shmid = shmget(key, memory->size, SHM_HUGETLB | IPC_CREAT | SHM_R | SHM_W)) < 0)
		return -1;
	if((memory->ptr = (void *)shmat(shmid, 0, 0)) == (void *)-1) {
		shmctl(shmid, IPC_RMID, NULL);
		return -1;
	}

	return 0;
}

int free_memory_info(s_memory_info *memory) {
	if(!memory || !memory->ptr)
		return -1;

	// free(memory->ptr);
	shmctl(shmid, IPC_RMID, NULL);

	return 0;
}

void print_memory_info(const s_memory_info memory) {
	size_t i;

	printf("* Memory information\n");
	printf("\t. Size: %lu Bytes\n", memory.size);
	printf("\t. Map:\n\t\t\t+------+\n");
	for(i = 0; i < memory.n_lines; i++) {
		if(memory.n_lines > 4 && i > 1 && i < memory.n_lines - 1) {
			if(i == 2) {
				printf("\t\t......\n\t\t\t+------+\n");
			}
			continue;
		}
		printf("\t\t%7lu\t", i);
		printf("| %4lu | ", memory.line_size);
		printf("(%p)", memory.ptr + memory.line_size * i);
		printf("\n\t\t\t+------+\n");
	}
	printf("\t\t\t = %lu\n", memory.size);
}