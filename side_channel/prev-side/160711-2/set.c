#include "set.h"

int set_addr(s_element *element, const void *addr) {
	if(!element)
		return -1;

	element->addr = (void *)addr;
}

void *get_addr(const s_element *element) {
	if(!element)
		return NULL;
	return element->addr;
}

s_element *create_element(void) {
	s_element *element;
	
	if(!(element = (s_element *)malloc(sizeof(s_element))))
		return NULL;
	element->addr = NULL;
	element->prev = NULL;
	element->next = NULL;

	return element;
}

int insert_element(s_set *set, s_element *element) {
	s_element *element_prev;
	size_t i;

	if(!set || !element)
		return -1;

	if(set->n_elements == 0)
		set->elements = element;
	else {
		element_prev = set->elements;
		for(i = 0; i < set->n_elements - 1; i++)
			element_prev = element_prev->next;
		element_prev->next = element;
		element->prev = element_prev;
	}
	++set->n_elements;

	return 0;
}

int insert_element_by_addr(s_set *set, const void *addr) {
	s_element *element;

	if(!set)
		return -1;

	element = get_element_by_addr(set, addr);
	if(!element)
		element = create_element();
	else
		return -1;
	set_addr(element, addr);
	insert_element(set, element);

	return 0;
}

int delete_element(s_set *set, s_element *element) {
	s_element *i;

	if(!set || !element)
		return -1;

	for(i = set->elements; i != NULL; i = i->next) {
		if(i == element)
			break;
	}
	if(!i)
		return -1;

	if(element->prev)	element->prev->next = element->next;
	else				set->elements = element->next;
	if(element->next)	element->next->prev = element->prev;
	free(element);
	--set->n_elements;

	return 0;
}

int delete_element_by_addr(s_set *set, const void *addr) {
	s_element *element;

	if(!set)
		return -1;

	element = get_element_by_addr(set, addr);
	if(!element)
		return -1;
	delete_element(set, element);

	return 0;
}

s_element *get_element_by_addr(const s_set *set, const void *addr) {
	s_element *i;

	if(!set)
		return NULL;

	for(i = set->elements; i != NULL; i = i->next)
		if(i->addr == addr)
			break;

	return i;
}

s_element *get_element_by_idx(const s_set *set, const size_t idx) {
	s_element *i;
	size_t cnt;

	if(!set)
		return NULL;

	cnt = 0;
	for(i = set->elements; i != NULL; i = i->next) {
		if(cnt == idx)
			break;
		++cnt;
	}

	return i;
}

s_set *create_set(void) {
	s_set *set;
	
	if(!(set = (s_set *)malloc(sizeof(s_set))))
		return NULL;
	set->n_elements = 0;
	set->elements = NULL;

	return set;
}

int init_set(s_set *set) {
	s_element *element;
	size_t i;

	if(!set)
		return -1;

	for(i = 0; i < set->n_elements; i++)
		delete_element(set, get_element_by_idx(set, i));
	set->elements = NULL;
	set->n_elements = 0;

	return 0;
}

int free_set(s_set **set) {
	if(!set || !(*set))
		return -1;

	init_set(*set);
	free(*set);
	set = NULL;

	return 0;
}

s_set *copy_set(const s_set *set_src) {
	s_set *set_dst;
	s_element *i;

	if(!set_src)
		return NULL;
	set_dst = create_set();

	for(i = set_src->elements; i != NULL; i = i->next) {
		s_element *element = create_element();
		set_addr(element, get_addr(i));
		insert_element(set_dst, element);
	}

	return set_dst;
}

void print_set(const s_set *set, const char *str) {
	s_element *i;

	if(!set)
		return;

	if(str)	printf("%s\n", str);
	else	printf("* Set information\n");
	printf("\t. Number of elements:\t%lu\n", set->n_elements);
	// for(i = set->elements; i != NULL; i = i->next)
	// 	printf("\t%p\n", i->addr);
}