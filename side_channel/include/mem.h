#include <sys/time.h>
#include <stdint.h>

#define L3_CACHE_SIZE 1024*1024*30
#define CACHELINE_SIZE 64 

void clflush(volatile void *p,int size);

uint64_t get_cycle();
