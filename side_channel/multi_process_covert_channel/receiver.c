#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#define THRESHOLD 1000
#define TIMESLOT 650
#define LOOP 10


char *cache_line;
char *llc;
FILE *file;
unsigned long avg_delay_time;
unsigned long over_threshold_counter;
volatile uint64_t sTime,eTime;
int threshold;

void read_cache_lines1(char *llc,int i,uint64_t timeslot);
void read_cache_lines2(char *llc,int i,uint64_t timeslot);
void loop1();
void loop2();

int main(int argc,char* argv[])
{
	char *file_name;
	threshold=THRESHOLD;

	if(argc==2)
	{
		file_name=argv[1];
	}
	else if(argc==3)
	{
		threshold=atoi(argv[1]);
		file_name=argv[2];
	}
	else
	{
		file_name="Delay_result.txt";
	}
	printf("%d\n%s\n",threshold,file_name);
	over_threshold_counter=0;
	avg_delay_time=0;
	file=fopen(file_name,"wt");
	llc=(char*)malloc(sizeof(char)*L3_CACHE_SIZE);
	cache_line=(char*)malloc(sizeof(char)*CACHELINE_SIZE);


	//loop1();

	loop2();


	free(llc);
	free(cache_line);
	fclose(file);
	return 0;
}

void loop1()
{
	int i;
	for(i=0;i<L3_CACHE_SIZE;i+=CACHELINE_SIZE)
	{
		clflush(llc+i,CACHELINE_SIZE);
		sTime=get_cycle();
		while(get_cycle()-sTime<TIMESLOT)
		{
		}
		read_cache_lines1(llc+i,i,get_cycle()-sTime);

	}

	fprintf(file,"total_time:%lu    loop:%d    avg_delay_time:%lu\n",avg_delay_time,(L3_CACHE_SIZE/CACHELINE_SIZE),avg_delay_time/(L3_CACHE_SIZE/CACHELINE_SIZE));
	fprintf(file,"THRESHOLD:%d    over_threshold_counter number:%lu\n",threshold,over_threshold_counter);
}

void read_cache_lines1(char *llc,int i,uint64_t timeslot)
{
	sTime=get_cycle();
	*cache_line=*(llc);
	eTime=get_cycle();
	avg_delay_time+=eTime-sTime;
	if(eTime-sTime>threshold)
	{
		over_threshold_counter++;
	}

		fprintf(file,"num:%d timeslot:%lu    ",i/CACHELINE_SIZE,timeslot);
		fprintf(file,"delay:%lu\n",eTime-sTime);
}



void loop2()
{
	int j;
	volatile int i;
	unsigned long avg=0;
	for(j=0;j<LOOP;j++)
	{
		printf("%d.",j);
		fflush(stdout);
		over_threshold_counter=0;
		avg_delay_time=0;
		for(i=0;i<L3_CACHE_SIZE;i+=CACHELINE_SIZE)
		{
			clflush(llc+i,CACHELINE_SIZE);
			sTime=get_cycle();
			while(get_cycle()-sTime<TIMESLOT)
			{
			}
			read_cache_lines2(llc+i,i,get_cycle()-sTime);

		}

		fprintf(file,"total_time:%lu    loop:%d    avg_delay_time:%lu    ",avg_delay_time,(L3_CACHE_SIZE/CACHELINE_SIZE),avg_delay_time/(L3_CACHE_SIZE/CACHELINE_SIZE));
		fprintf(file,"THRESHOLD:%d    over_threshold_counter number:%lu\n",threshold,over_threshold_counter);
		avg+=over_threshold_counter;
	}
	fprintf(file,"avg_over_threshold_counter numver:%lu\n",avg/LOOP);
}

void read_cache_lines2(char *llc,int i,uint64_t timeslot)
{
	sTime=get_cycle();
	*cache_line=*(llc);
	eTime=get_cycle();
	avg_delay_time+=eTime-sTime;
	if(eTime-sTime>threshold)
	{
		over_threshold_counter++;
	}

}

