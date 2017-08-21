#include <stdio.h>
#include <mem.h>
#include <stdlib.h>
#include <string.h>
#define LOOP 1

void test_case1();
void test_case2();

int main()
{

	//test_case1();
	test_case2();
	return 0;
}



void test_case1()
{
	uint64_t st,et;
	int i;
	int sum=0;
	sum=0;
	char *tmp=(char*)malloc(sizeof(char)*L3_CACHE_SIZE);
	char *s=(char*)malloc(sizeof(char)*L3_CACHE_SIZE);
	memset(s,'a',sizeof(char)*L3_CACHE_SIZE);
	for(i=0;i<LOOP;i++)
	{
		clflush(s,sizeof(char)*L3_CACHE_SIZE);


		st=get_cycle();
		*tmp=*s;
		et=get_cycle();
		sum+=et-st;
	}
	sum/=LOOP;
	printf("avg1=%d\n",sum);


	sum=0;
	for(i=0;i<LOOP;i++)
	{
		st=get_cycle();
		*tmp=*s;
		et=get_cycle();
		sum+=et-st;
	}
	sum/=LOOP;
	printf("avg2=%d\n",sum);
}


void test_case2()
{
	uint64_t st,et;
	int i;
	char *s=(char*)malloc(sizeof(char)*64*2);
	char *tmp=(char*)malloc(sizeof(char)*64);
	memset(s,'a',sizeof(char)*64*2);

	st=get_cycle();
	*tmp=*s;
	et=get_cycle();
	printf("befor:%d ",(int)(et-st));

	st=get_cycle();
	*tmp=*(s+64);
	et=get_cycle();
	printf("%d\n",(int)(et-st));

	clflush(s,64);
	st=get_cycle();
	*tmp=*s;
	et=get_cycle();
	printf("after:%d ",(int)(et-st));

	st=get_cycle();
	*tmp=*(s+64);
	et=get_cycle();
	printf("%d\n",(int)(et-st));
}
