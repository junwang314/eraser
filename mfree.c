#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "cleaner.h"
#include "util.h"

static int do_fprintf=1;
static pid_t e_pid;

static void mtrace_init(void)
{
	e_ON = 0;

	e_pid = getpid();
    real_free = dlsym(RTLD_NEXT, "free");
    if (NULL == real_free) {
        fprintf(stdout, "Error in `dlsym`: %s\n", dlerror());
    }
	e_queue_init();
	pthread_create(&e_cleaner, NULL, cleaner, NULL);
	if(atexit(e_terminator)) {
		fprintf(stdout, "Error: atexit(e_terminator) failed\n");
	}

	e_ON = 1;
}

int gettid(void)
{
    return syscall(SYS_gettid);
}

void free(void *p)
{
    if(real_free==NULL) {
        mtrace_init();
    }

#ifdef FORK
	e_ON = 0;
	if(e_pid != getpid()) {
		fprintf(stdout, "Process fork detected\n");
		//FILE *fp = fopen("eraser.log", "a");
		//if (!fp)
		//	fp = stdout;
		//fprintf(fp, "Process fork detected, parent %lu, child %lu\n",
		//		(unsigned long)e_pid,
		//		(unsigned long)getpid());
		//if(fp!=stdout)
		//	fclose(fp);
		e_pid = getpid();
		pthread_create(&e_cleaner, NULL, cleaner, NULL);
	}
	e_ON = 1;
#endif

	if (!p) {
		return;
	}
	if (!e_ON) {
		real_free(p);
		return;
	}
	e_ON = 0;
	e_queue_enque(p);
	//fprintf(stdout, "%d/%d: free(%p)\n", getpid(), gettid(), p);
	//printf("%d/%d: free(%p)\n", getpid(), gettid(), p);
	e_ON = 1;
	return;
}
