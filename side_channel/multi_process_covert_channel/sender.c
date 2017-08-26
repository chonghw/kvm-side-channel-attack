#include <stdio.h>
#include <mem.h>
#include <stdlib.h>
#include <string.h>

#define TIMESLOT 1000

int main(int argc, char* argv[])
{
	volatile uint64_t st;
	int num;
	char* ch;
	int i=0;

	if(argc==2)
	{
		num=atoi(argv[1]);
	}
	else
	{
		num=L3_CACHE_SIZE/64;
	}
	printf("num=%d\nargc=%d\n",num,argc);
	ch=(char*)malloc(sizeof(char)*64*num);

	while(1)
	{
		i++;
		i%=10;
		memset(ch,i,sizeof(char)*64*num);
		st=get_cycle();
		while(get_cycle()-st<TIMESLOT)
		{
		}

	}
	return 0;
}
