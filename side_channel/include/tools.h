#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <mem.h>
#include <math.h>

void binary_print(size_t addr,int size);
void clflush(volatile void *p,int size);
uint64_t get_cycle();
