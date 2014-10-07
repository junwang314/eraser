#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

#include "util.h"

static pthread_t e_cleaner;
static void* (*real_free)(void *) = NULL;
static int e_exit = 0;

void e_terminator()
{
	e_ON = 0;

	fprintf(stderr, "About to exit...\n");
	e_exit = 1;
	pthread_join(e_cleaner, NULL);

	e_ON = 1;
}

void* cleaner()
{
	e_ON = 0;
	void *p;
	fprintf(stderr, "This is cleaner!\n");
	while (!e_exit) {
		//fprintf(stderr, "cleaner loop...\n");
		if (p = e_queue_deque()) {
			memset(p, 0x30, malloc_usable_size(p));
    		real_free(p);
		}
		else {
			usleep(100);
		}
	}
	return NULL;
}
