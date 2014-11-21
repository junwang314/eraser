#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

#include "util.h"

static pthread_t e_cleaner;
static void* (*real_free)(void *) = NULL;
static void* (*real_malloc)(size_t) = NULL;
static void* (*real_memcpy)(void *, const void *, size_t) = NULL;
static int e_exit = 0;

static FILE* fmalloc = NULL;
static FILE* fmemcpy = NULL;

void e_terminator()
{
	e_ON = 0;

	fprintf(stdout, "Eraser about to exit...\n");
	e_exit = 1;
	pthread_join(e_cleaner, NULL);
    if (fmalloc) {
        fclose(fmalloc);
    }
    if (fmemcpy) {
        fclose(fmemcpy);
    }

	e_ON = 1;
}

void* cleaner()
{
	e_ON = 0;
	void *p;
	e_thread_queue *q;
	fprintf(stdout, "This is cleaner!\n");
	while(!e_thread_queue_head) {
		sleep(0);
	}
	while (!e_exit) {
		//fprintf(stdout, "cleaner loop...\n");
		for (q = e_thread_queue_head; q!=NULL; q=q->next) {
			if ((p = e_queue_deque(q))) {
				memset(p, 0x30, malloc_usable_size(p));
    			real_free(p);
			}
			else {
				usleep(100);
			}
		}
	}
	return NULL;
}
