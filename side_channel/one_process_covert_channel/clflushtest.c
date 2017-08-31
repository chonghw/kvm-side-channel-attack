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
	size_t *cache_lines[WAY*SLICE];
	cache_set *next;
	cache_set *prev;
}Cache_set;


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

	for(tmp_head;;tmp_head=tmp_head->next)
	{
		if(tmp_head->index==index)
		{
			if(tmp_head->top<WAY*SLICE)
			{
				tmp_head->cache_lines[tmp_head->top]=cache_line_addr;
				tmp_head->top++;
			}
			else
			{
				new_cache_set=make_cache_set();
				new_cache_set->cache_lines[new_cache_set->top]=cache_line_addr;
				new_cache_set->top++;
				new_cache_set->index=make_index_set(cache_line_addr);

				tmp_head->next=new_cache_set;
				new_cache_set->prev=tmp_head;
			}
		}
		else
		{
			new_cache_set=make_cache_set();
			new_cache_set->cache_lines[new_cache_set->top]=cache_line_addr;
			new_cache_set->top++;
			new_cache_set->index=make_index_set(cache_line_addr);

			tmp_head->next=new_cache_set;
			new_cache_set->prev=tmp_head;
		}
		if(tmp_head->next==NULL)
		{
			break;
		}
	}
}


int make_index_set(size_t addr)
{
	int left_shift=64-(INDEX_SET_SIZE+OFFSET_SIZE);
	int right_shift=64-INDEX_SET_SIZE;
	return (addr<<left_shift)>>right_shift;
}


char* make_candidate_set()
{
	int i,j;
	int top=1;

	char* lines=(char*)malloc(sizeof(char)*L3_CACHE_SIZE*2);
	char** candidate=(char**)malloc(sizeof(char*)*WAY*SLICE);

	size_t addr=(size_t)&lines[0];
	int index;

	index=make_index_set(addr);

	candidate[0]=&lines[0];

	for(i=0;i<L3_CACHE_SIZE*2;i+=CACHELINE_SIZE)
	{
		addr=(size_t)&lines[i];
		if(make_index_set(addr)==index)
		{
			candidate[top]=&lines[i];
			top++;
			if(top==20*12)
			{
				break;
			}
		}
	}

	for(i=0;i<top;i++)
	{
		addr=(size_t)&(*candidate[i]);
		index=make_index_set(addr);
		printf("%d    index:%d    addr:%lu(",i,index,addr);
		binary_print(addr,64); printf(")\n");

	}

	return NULL;
}



int main()
{
	make_candidate_set();
	return 0;
}



