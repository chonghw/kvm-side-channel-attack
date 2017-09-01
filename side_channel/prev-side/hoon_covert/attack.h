#ifndef __COVERT_CHANNEL_H__
#define __COVERT_CHANNEL_H__

#include "base.h"

#define KEY	(27317)

////////////////////////////////////////////////
// Constants for covert/side-channel listener //
#ifdef VM
	#define T_DETECT	(8000)
#else
	#define T_DETECT	(700)
#endif
#define T_TIMESLOT	(15000)
#define N_TIMESLOTS	(5)
////////////////////////////////////////////////

////////////////////////////////////////////////
//    Constants for covert-channel sender     //
// #define T_MARK	(40000)
// #define T_PAUSE	(10000)
////////////////////////////////////////////////

extern char OUTPUT[256];
extern size_t BASE_CACHE_SET_INDEX, N_CACHE_SETS;

uint64_t probe(uint64_t *start, const size_t n);
uint64_t get_cycle_in_vm(void);

uint64_t *create_direction(const s_set *set);
uint64_t *change_direction(uint64_t *start, const size_t n);

int init_listener(void);
int start_listen(void);
void stop_listener(void);

int init_sender(void);
int start_send(void);
void stop_sender(void);

#endif