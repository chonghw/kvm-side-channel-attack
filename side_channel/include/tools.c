#include "tools.h"


void binary_print(size_t addr,int size)
{
	int i;
	for(i=1;i<=size;i++)
	{
		printf("%d",(int)(addr>>(size-1)));
		if(i==47 || i==58)
		{
			printf("|");
		}
		if(i%4==0)
		{
			printf(" ");
		}
		addr=addr<<1;
	}
}

uint64_t get_cycle()
{
	uint64_t a, d;
	asm volatile ("cpuid; rdtsc" : "=a" (a), "=d" (d) : : "ebx", "ecx");
	return (a | ((uint64_t)d << 32));
}



void clflush(volatile void *p,int size)
{
	int i;
	for(i=0;i<size;i+=CACHELINE_SIZE)
	{
		asm volatile("clflush (%0)" :: "r"((char*)p+i));
	}
}
