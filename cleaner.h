#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

#include "util.h"

static pthread_t e_cleaner;
static void* (*real_free)(void *)=NULL;

void* cleaner()
{
	e_ON = 0;
	e_head = 0;
	e_tail = 0;
	fprintf(stderr, "This is cleaner!\n");
	void *p;
	while (1) {
		p = e_deque();
    	memset(p, 0, malloc_usable_size(p));
    	real_free(p);
	}
	return NULL;
}
