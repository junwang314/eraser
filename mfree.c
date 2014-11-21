#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include "cleaner.h"
#include "util.h"

static pid_t e_pid;

static void mtrace_init(void)
{
	e_ON = 0;

	e_pid = getpid();

    real_free = dlsym(RTLD_NEXT, "free");
    if (NULL == real_free) {
        fprintf(stdout, "Error in `dlsym`: %s\n", dlerror());
    }

    real_malloc = dlsym(RTLD_NEXT, "malloc");
    if (NULL == real_malloc) {
        fprintf(stdout, "Error in `dlsym`: %s\n", dlerror());
    }
    if (fmalloc==NULL) {
        fmalloc = fopen("malloc.log", "a");
    }

    real_memcpy = dlsym(RTLD_NEXT, "memcpy");
    if (NULL == real_memcpy) {
        fprintf(stdout, "Error in `dlsym`: %s\n", dlerror());
    }
    if (fmemcpy==NULL) {
        fmemcpy = fopen("memcpy.log", "a");
    }

	/* For SPEC benchmark, some processes for counting purpose are also
	 * created. We do not do heap erasing for them.
	 * Only the process named "run_base_ref" is what we need to protect.
	 */
#ifdef SPEC
	const char *pstr = "../run_base_ref";
	int isTarget;
	isTarget = !strncmp(program_invocation_name, pstr, strlen(pstr));
	if (!isTarget) {
		e_ON = 0;
		return;
	}
#endif

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
    if(real_free == NULL) {
        mtrace_init();
    }

#ifdef FORK
	e_ON = 0;
	if(e_pid != getpid()) {
		e_pid = getpid();
		fprintf(stdout, "Process fork detected\n");
		//FILE *fp = fopen("eraser.log", "a");
		//if (!fp)
		//	fp = stdout;
		//fprintf(fp, "Process fork detected, parent %lu, child %lu\n",
		//		(unsigned long)e_pid,
		//		(unsigned long)getpid());
		//if(fp!=stdout)
		//	fclose(fp);
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
	if (!e_thq) {
		e_thq = e_get_thread_queue();
	}
	//fprintf(stdout, "%d/%d: free(%p)\n", getpid(), gettid(), p);
	e_queue_enque(p, e_thq);
	//printf("%d/%d: free(%p)\n", getpid(), gettid(), p);
	e_ON = 1;
	return;
}

void *malloc(size_t size)
{
    if(real_malloc == NULL) {
        mtrace_init();
    }
    if (!e_ON) {
        return real_malloc(size);
    }
	e_ON = 0;
	fprintf(fmalloc, "malloc(%d)\n", size);
	e_ON = 1;
    return real_malloc(size);
}

void *memcpy(void *dest, const void *src, size_t n)
{
    if(real_memcpy == NULL) {
        mtrace_init();
    }
    if (!e_ON) {
        return real_memcpy(dest, src, n);
    }
    e_ON = 0;
	fprintf(fmemcpy, "memcpy(%p, %p, %d)\n", dest, src, n);
    e_ON = 1;
    return real_memcpy(dest, src, n);
}
