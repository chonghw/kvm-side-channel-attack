#include <stdio.h>
#include <mem.h>
#include <stdlib.h>
#include <string.h>
#define LOOP 1

void test_case1();
void test_case2();

void test_case3();
int main()
{

	//test_case1();
	//test_case2();
	test_case3();
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



void test_case3()
{

	uint64_t st,et;
	int i,j;
	int over_count;
	uint64_t avg_read_time;
	uint64_t threshold;
	char *s=(char*)malloc(sizeof(char)*L3_CACHE_SIZE);
	char *tmp=(char*)malloc(sizeof(char)*CACHELINE_SIZE);
	memset(s,0,sizeof(char)*L3_CACHE_SIZE);


	avg_read_time=0;
	for(i=99;i>=0;i--)
	{
		st=get_cycle();
		*tmp=*(s+i);
		et=get_cycle();
		avg_read_time+=(et-st);
		printf("i:%d==%lu\n",i,et-st);
	}

	avg_read_time/=100;

	
	threshold=0;
	for(i=99;i>=0;i--)
	{
		clflush((s+i),64);
		st=get_cycle();
		*tmp=*(s+i);
		et=get_cycle();
		threshold+=(et-st);
		printf("i:%d==%lu\n",i,et-st);
	}

	threshold/=10;

	printf("avg:%lu thres:%lu\n",avg_read_time,threshold);
	getc(stdin);

	for(i=0;i<L3_CACHE_SIZE;i+=CACHELINE_SIZE)
	{
		over_count=0;
		for(j=i;j<L3_CACHE_SIZE;j+=CACHELINE_SIZE)
		{
			clflush(s+i,CACHELINE_SIZE);

			st=get_cycle();
			*tmp=*(s+j);
			et=get_cycle();
			if((et-st)>threshold)
			{
				//printf("i=%d : j=%d delay:%lu\n",i,j,et-st);
				over_count++;
			}
		}
		printf("%d:%d\n",i,over_count);
	}

	free(s);
	free(tmp);
}
