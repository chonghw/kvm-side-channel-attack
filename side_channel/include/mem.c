#include "mem.h"


void clflush(volatile void *p,int size)
{
	int i;
	for(i=0;i<size;i+=CACHELINE_SIZE)
	{
		asm volatile("clflush (%0)" :: "r"((char*)p+i));
	}
}

uint64_t get_cycle()
{
	uint64_t a, d;
	asm volatile ("cpuid; rdtsc" : "=a" (a), "=d" (d) : : "ebx", "ecx");
	return (a | ((uint64_t)d << 32));
}
