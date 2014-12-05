#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <signal.h>

#include "util.h"

static pthread_t e_cleaner;
#if defined(ERASER) || defined(INLINE)
static void* (*real_free)(void *) = NULL;
#endif
#ifdef PADDING
static void* (*real_malloc)(size_t) = NULL;
#endif
#ifdef LEAK
static void* (*real_memcpy)(void *, const void *, size_t) = NULL;
#endif
static int e_exit = 0;

static FILE* fmalloc = NULL;
static FILE* fmemcpy = NULL;
static FILE* fleak[4];

void e_terminator()
{
	e_ON = 0;

#ifdef DEBUG
	fprintf(stdout, "Eraser about to exit...\n");
#endif
	e_exit = 1;
	if (e_cleaner) {
		pthread_join(e_cleaner, NULL);
	}
    if (fmalloc) {
        fclose(fmalloc);
    }
    if (fmemcpy) {
        fclose(fmemcpy);
    }
	int i;
	for (i=0; i<sizeof(fleak)/sizeof(fleak[0]); i++) {
		if (fleak[i]) {
			fclose(fleak[i]);
		}
	}

	e_ON = 1;
}

#ifdef ERASER
void* cleaner()
{
	e_ON = 0;
	sigset_t set;
	sigfillset(&set);
	pthread_sigmask(SIG_SETMASK, &set, NULL);

	void *p;
	e_thread_queue *q;
#ifdef DEBUG
	fprintf(stdout, "This is cleaner!\n");
#endif
	while(!e_thread_queue_head) {
		sleep(0);
	}
	while (!e_exit) {
		//fprintf(stdout, "cleaner loop...\n");
		for (q = e_thread_queue_head; q!=NULL; q=q->next) {
			if ((p = e_queue_deque(q))) {
				memset(p, 0x06, malloc_usable_size(p));
    			real_free(p);
			}
			else {
				usleep(100);
			}
		}
	}
	return NULL;
}
#endif
