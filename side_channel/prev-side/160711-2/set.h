#ifndef __SET_H__
#define __SET_H__

#include <stdio.h>
#include <stdlib.h>

typedef struct element {
	void *addr;
	struct element *prev, *next;
} s_element;

typedef struct set {
	size_t n_elements;
	s_element *elements;
} s_set;

int set_addr(s_element *element, const void *addr);
void *get_addr(const s_element *element);

s_element *create_element(void);
int insert_element(s_set *set, s_element *element);
int insert_element_by_addr(s_set *set, const void *addr);
int delete_element(s_set *set, s_element *element);
int delete_element_by_addr(s_set *set, const void *addr);
s_element *get_element_by_addr(const s_set *set, const void *addr);
s_element *get_element_by_idx(const s_set *set, const size_t idx);

s_set *create_set(void);
int init_set(s_set *set);
int free_set(s_set **set);
s_set *copy_set(const s_set *set_src);
void print_set(const s_set *set, const char *str);

#endif