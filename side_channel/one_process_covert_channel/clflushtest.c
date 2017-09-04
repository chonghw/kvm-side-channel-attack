#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mem.h>
#include <tools.h>

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define LOOP 1
#define THRESHOLD 10000
#define KEY 1

typedef struct cache_set
{
	int index;
	int top;
	int length;
	char *cache_lines[WAY*SLICE*2];
	cache_set *next;
	cache_set *prev;
}Cache_set;

typedef struct conflict_set
{
	int top;
	uint64_t access_time[WAY*SLICE*2];
	char *cache_lines[WAY*SLICE*2];
}Conflict_set;

//for debug
void print_candidate_info(Cache_set* candidate_head);
void print_candidate_data(Cache_set* candidate_head);

//cache_set struct linked list
void cache_line_to_cadidate(Cache_set *head, uint64_t cache_line_addr);
Cache_set* make_cache_set();

//for candidate 
Cache_set* make_candidate_set();
int make_index(uint64_t addr);

//for conflict
Cache_set* make_conflict_set(Cache_set* candidate_head);
void qsort_conflict_set_asc(Conflict_set* conflict_set,int left,int right);
int access_time_asc(const void *a, const void *b);
uint64_t make_conflict_threshold();
int measure_candidate_time(uint64_t threashold, char* candidate, Conflict_set *conflict_set);

int measure_candidate_time(uint64_t threashold, char* candidate, Conflict_set *conflict_set)
{
	uint64_t a,d;
	volatile char *tmp_data=(char*)malloc(sizeof(char));
	uint64_t i;
	volatile uint64_t cycle_start,cycle_end;

	*tmp_data=*candidate;
	for(i=0;i<conflict_set->top;i++)
	{
		*tmp_data=*conflict_set->cache_lines[i];
	}


	__asm__("lfence;");
	asm volatile ("rdtsc;" : "=a" (a), "=d" (d) : : "ebx", "ecx");
	cycle_start = (a | ((uint64_t)d << 32));

	*tmp_data=*candidate;

	__asm__("lfence;");
	asm volatile ("rdtsc;" : "=a" (a), "=d" (d) : : "ebx", "ecx");
	cycle_end = (a | ((uint64_t)d << 32));

	printf("cycle:%lu\n",cycle_end-cycle_start);

	if(cycle_end-cycle_start>threashold)
	{
		return 1;
	}
	else if(cycle_end-cycle_start<threashold)
	{
		return -1;
	}
	else
	{
		return 0;
	}

}

int main()
{
	Cache_set* candidate_head;
	Cache_set *conflict_head;

	printf("=========make candidates=========\n");
	candidate_head=make_candidate_set();
	printf("=========end make candidates=========\n");
	getc(stdin);
	/*
	print_candidate_info(candidate_head);
	getc(stdin);
	print_candidate_data(candidate_head);
	*/
	printf("=========make conflict set=========\n");
	conflict_head=make_conflict_set(candidate_head);

	return 0;
}


int access_time_asc(const void *a, const void *b)
{
	if( *(uint64_t *)a > *(uint64_t *)b )
	{
		return 1;
	}
	else if(*(uint64_t *)a < *(uint64_t *)b )
	{	
		return -1;
	}
	else
	{
		return 0;
	}
}

uint64_t make_conflict_threshold()
{
	volatile char test;
	volatile char data;
	uint64_t normal_time,delay_time,threshold;
	uint64_t a,d;
	int i;
	volatile uint64_t cycle_start,cycle_end;
	uint64_t normal_max=0,delay_max=0;

	for(i=0;i<100;i++)
	{
		test=1;
		clflush(&test,CACHELINE_SIZE);

		__asm__("lfence;");
		asm volatile ("rdtsc;" : "=a" (a), "=d" (d) : : "ebx", "ecx");
		cycle_start = (a | ((uint64_t)d << 32));

		data=test;

		__asm__("lfence;");
		asm volatile ("rdtsc;" : "=a" (a), "=d" (d) : : "ebx", "ecx");
		cycle_end = (a | ((uint64_t)d << 32));
		printf("delay time:%4lu    ", cycle_end - cycle_start);

		delay_max+=(cycle_end-cycle_start);


		test=1;
		__asm__("lfence;");
		asm volatile ("rdtsc;" : "=a" (a), "=d" (d) : : "ebx", "ecx");
		cycle_start = (a | ((uint64_t)d << 32));

		data=test;

		__asm__("lfence;");
		asm volatile ("rdtsc;" : "=a" (a), "=d" (d) : : "ebx", "ecx");
		cycle_end = (a | ((uint64_t)d << 32));
		printf("normal time:%4lu\n",cycle_end - cycle_start);

		normal_max+=(cycle_end-cycle_start);


	}
	printf("delay max: %4lu    normal max: %4lu    threshold: %4lu\n",delay_max/100,normal_max/100,((delay_max/100)-(normal_max/100))/2);
	getc(stdin);
	return ((delay_max/100)-(normal_max/100))/2;
}



Cache_set* make_conflict_set(Cache_set* candidate_head)
{
	int threshold;
	uint64_t access_time;
	uint64_t i,j;
	Conflict_set tmp_conflict_set,result_conflict_set;

	Cache_set *tmp_candidate_head=candidate_head;
	Cache_set *conflict_set=make_cache_set();

	threshold=make_conflict_threshold();
	//threshold=100;

	for(tmp_candidate_head;;tmp_candidate_head=tmp_candidate_head->next)
	{
		if(tmp_candidate_head->index!=-1)
		{
			while(1)
			{
				tmp_conflict_set.top=0;
				for(i=0;i<tmp_candidate_head->top;i++)
				{
					printf("%3lu    candidate num:%3d    ",i,tmp_candidate_head->top);
					if(measure_candidate_time(threshold,tmp_candidate_head->cache_lines[i],&tmp_conflict_set)==-1)
					{
						tmp_conflict_set.cache_lines[ tmp_conflict_set.top ] =tmp_candidate_head->cache_lines[i];
						tmp_conflict_set.top++;
					}
				}
				printf("tmp_conflict_set top:%d\n",tmp_conflict_set.top);
				getc(stdin);
				
				if(tmp_conflict_set.top==WAY*SLICE)
				{
					printf("found conflict set!\n");
					break;
				}
			}
		}

		if(tmp_candidate_head->next==NULL)
		{
			break;
		}
	}
}

int make_index(uint64_t addr)
{
	int left_shift=64-(INDEX_SET_BIT+OFFSET_BIT);
	int right_shift=64-INDEX_SET_BIT;
	return (addr<<left_shift)>>right_shift;
}

Cache_set* make_cache_set()
{
	Cache_set *head=(Cache_set*)malloc(sizeof(Cache_set));
	head->top=0;
	head->index=-1;
	head->next=NULL;
	head->prev=NULL;

	return head;
}

void cache_line_to_cadidate(Cache_set *head, uint64_t cache_line_addr)
{
	Cache_set *tmp_head=head;
	Cache_set *new_cache_set;
	int index=make_index(cache_line_addr);
	int cache_lines_top_max=WAY*SLICE*2;

	//binary_print(cache_line_addr,64); printf("\n");
	for(tmp_head;;tmp_head=tmp_head->next)
	{
		if(tmp_head->index==index)
		{
			//printf("index hit:%d    ",index);
			if(tmp_head->top<cache_lines_top_max)
			{
				tmp_head->cache_lines[tmp_head->top]=(char*)cache_line_addr;
				tmp_head->top++;
				//printf("enough top: %d    ",tmp_head->top);
				//binary_print(cache_line_addr,64); printf("\n");
				return;
			}
		}
		if(tmp_head->next==NULL)
		{
			break;
		}
	}
	//printf("new index!!!    %d\n",index);
	new_cache_set=make_cache_set();
	new_cache_set->cache_lines[new_cache_set->top]=(char*)cache_line_addr;
	new_cache_set->top++;
	new_cache_set->index=index;

	tmp_head->next=new_cache_set;
	new_cache_set->prev=tmp_head;
	//getc(stdin);
}

Cache_set* make_candidate_set()
{
	size_t i,j;
	int index_set=pow(2,INDEX_SET_BIT);  //  0~((1^11)-1)
	int cache_set_amnt=WAY*SLICE*2;      // 0~479
	int tag_bit_shift=INDEX_SET_BIT+OFFSET_BIT;  // 11+6=17
	int shm_id;
	void *shm_addr;
	size_t tmp_addr;
	Cache_set *candidate_head;
	Cache_set *tmp_head;

	candidate_head=make_cache_set();
	shm_id=shmget((key_t)KEY,L3_CACHE_SIZE*2,IPC_CREAT|0666);
	shm_addr=shmat(shm_id,(void*)0,0);
	


	for(i=0;i<index_set;i++)  //make index bit 	
	{
		for(j=0;j<cache_set_amnt;j++) //make tag bit
		{
			//printf("(%lu,%lu)_",i,j);
			//printf("push addr_%6lu_",( (j<<tag_bit_shift) | (i<<OFFSET_BIT) ));
			//binary_print(((j<<tag_bit_shift) | (i<<OFFSET_BIT)),64); printf("\n");
			//
			cache_line_to_cadidate(candidate_head,(uint64_t)shm_addr+((j<<tag_bit_shift) | (i<<OFFSET_BIT)));  //make address
			*((char*)shm_addr+((j<<tag_bit_shift) | (i<<OFFSET_BIT)))=0x1;  //push data
			//
			//printf("end\n");
		}
		//printf("\n");
	}

	/*
	printf("base_address:");
	binary_print((uint64_t)shm_addr,64); printf("\n");

	printf("===============list info===============\n");
	print_candidate_info(candidate_head);
	getc(stdin);

	print_candidate_data(candidate_head);
	*/


	return candidate_head;
}
void print_candidate_data(Cache_set* candidate_head)
{
	size_t i;
	Cache_set* tmp_head;
	tmp_head=candidate_head;
	for(tmp_head;;tmp_head=tmp_head->next)
	{
		for(i=0;i<tmp_head->top;i++)
		{
			printf("index:%4d    top:%3lu    address:%lu(",tmp_head->index, i, (uint64_t)tmp_head->cache_lines[i]);
			binary_print((uint64_t)tmp_head->cache_lines[i],64);
			printf(")    value:%d\n",*tmp_head->cache_lines[i]);
		}
		if(tmp_head->next==NULL)
		{
			break;
		}
	}

}
void print_candidate_info(Cache_set* candidate_head)
{
	Cache_set* tmp_head;
	tmp_head=candidate_head;
	for(tmp_head;;tmp_head=tmp_head->next)
	{
		printf("index:%4d    top:%3d    index_binary(",tmp_head->index,tmp_head->top);
		binary_print((uint64_t)(tmp_head->index<<6),64);
		printf(")\n");
		if(tmp_head->next==NULL)
		{
			break;
		}
	}
}




void qsort_conflict_set_asc(Conflict_set* conflict_set,int left,int right)
{
	int pivot=left;
	int left_end=pivot+1;
	uint64_t tmp_time;
	char *tmp_cache_line;
	int i;

	if(right-left<=1)
	{
		return;
	}

	for(i=left_end;i<right;i++)
	{
		if(conflict_set->access_time[i]<conflict_set->access_time[pivot])
		{

			tmp_time=conflict_set->access_time[i];
			conflict_set->access_time[i]=conflict_set->access_time[left_end];
			conflict_set->access_time[left_end]=tmp_time;

			tmp_cache_line=conflict_set->cache_lines[i];
			conflict_set->cache_lines[i]=conflict_set->cache_lines[left_end];
			conflict_set->cache_lines[left_end]=tmp_cache_line;

			left_end++;


		}
	}
	tmp_time=conflict_set->access_time[pivot];
	conflict_set->access_time[pivot]=conflict_set->access_time[left_end-1];
	conflict_set->access_time[left_end-1]=tmp_time;

	tmp_cache_line=conflict_set->cache_lines[pivot];
	conflict_set->cache_lines[pivot]=conflict_set->cache_lines[left_end-1];
	conflict_set->cache_lines[left_end-1]=tmp_cache_line;

	qsort_conflict_set_asc(conflict_set,left,left_end);
	qsort_conflict_set_asc(conflict_set,left_end,right);
}
