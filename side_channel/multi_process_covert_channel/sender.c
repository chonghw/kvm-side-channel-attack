#include <stdio.h>
#include <mem.h>
#include <stdlib.h>
#include <string.h>

#define TIMESLOT 1000

int main( void)
{
	volatile uint64_t st;
	char* ch=(char*)malloc(sizeof(char)*L3_CACHE_SIZE);
	int i=0;
	while(1)
	{
		i++;
		i%=10;
		memset(ch,i,sizeof(char)*L3_CACHE_SIZE);
		st=get_cycle();
		while(get_cycle()-st<TIMESLOT)
		{
		}

	}
	return 0;
}
