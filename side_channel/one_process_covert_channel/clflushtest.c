#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mem.h>
#include <tools.h>
#define LOOP 1
#define THRESHOLD 10000

typedef struct cache_set
{
	int index;
	int top;
	char *cache_lines[WAY*SLICE];
	cache_set *next;
	cache_set *prev;
}Cache_set;

int make_index_set(size_t addr)
{
	int left_shift=64-(INDEX_SET_SIZE+OFFSET_SIZE);
	int right_shift=64-INDEX_SET_SIZE;
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

void cache_line_to_cadidate(Cache_set *head, size_t cache_line_addr)
{
	Cache_set *tmp_head=head;
	Cache_set *new_cache_set;
	int index=make_index_set(cache_line_addr);

	//binary_print(cache_line_addr,64); printf("\n");
	for(tmp_head;;tmp_head=tmp_head->next)
	{
		if(tmp_head->index==index)
		{
			printf("index hit:%d    ",index);
			if(tmp_head->top<WAY*SLICE)
			{
				tmp_head->cache_lines[tmp_head->top]=(char*)cache_line_addr;
				tmp_head->top++;
				printf("enough top: %d    ",tmp_head->top);
				binary_print(cache_line_addr,64); printf("\n");
			}
			else
			{
				new_cache_set=make_cache_set();
				new_cache_set->cache_lines[new_cache_set->top]=(char*)cache_line_addr;
				new_cache_set->top++;
				new_cache_set->index=index;

				printf("new top: %d\n",tmp_head->top);
				binary_print(cache_line_addr,64); printf("\n");

				tmp_head->next=new_cache_set;
				new_cache_set->prev=tmp_head;
			}
			//getc(stdin);
			return;
		}
		if(tmp_head->next==NULL)
		{
			break;
		}
	}
	printf("new index!!!    %d\n",index);
	new_cache_set=make_cache_set();
	new_cache_set->cache_lines[new_cache_set->top]=(char*)cache_line_addr;
	new_cache_set->top++;
	new_cache_set->index=index;

	tmp_head->next=new_cache_set;
	new_cache_set->prev=tmp_head;
	//getc(stdin);
}


char* make_candidate_set()
{
	int i,j;
	int top=1;
	int index;
	size_t addr;

	char* lines=(char*)malloc(sizeof(char)*L3_CACHE_SIZE*2);
	//char** candidate=(char**)malloc(sizeof(char*)*WAY*SLICE);
	Cache_set * candidate_head = make_cache_set();
	Cache_set *tmp_head=candidate_head;

	/*
	addr=(size_t)&lines[0];
	index=make_index_set(addr);

	candidate[0]=&lines[0];
	*/

	for(i=0;i<L3_CACHE_SIZE*2;i+=CACHELINE_SIZE)
	{
		addr=(size_t)&lines[i];
		printf("%.2f%% new cache line!!! %d:%lu    ",(double)i/(L3_CACHE_SIZE*2),i,addr);
		cache_line_to_cadidate(candidate_head,addr);
	}
	tmp_head=candidate_head;
	i=0;
	for(tmp_head;;tmp_head=tmp_head->next)
	{
		for(j=0;j<tmp_head->top;j++)
		{
			printf("set index:%d     top:%d    cache_line_index:%d    cache_line_addr:%p    index:%d\n",i,tmp_head->top,j,tmp_head->cache_lines[j],tmp_head->index);
		}
		printf("\n");
		i++;
		if(tmp_head->next==NULL)
		{
			break;
		}
	}
	return NULL;
}



int main()
{
	make_candidate_set();
	return 0;
}



