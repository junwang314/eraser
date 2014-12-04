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
#include <assert.h>

#include "cleaner.h"
#include "util.h"
#include "heap_dump.h"

static pid_t e_pid;

static void mtrace_init(void)
{
	e_ON = 0;

	e_pid = getpid();

#if defined(ERASER) || defined(INLINE)
	real_free = dlsym(RTLD_NEXT, "free");
	if (NULL == real_free) {
		fprintf(stdout, "Error in `dlsym`: %s\n", dlerror());
	}
#endif

#ifdef PADDING
	real_malloc = dlsym(RTLD_NEXT, "malloc");
	if (NULL == real_malloc) {
		fprintf(stdout, "Error in `dlsym`: %s\n", dlerror());
	}
	if (fmalloc==NULL) {
		fmalloc = fopen("malloc.log", "a");
	}
#endif

#ifdef LEAK
	real_memcpy = dlsym(RTLD_NEXT, "memcpy");
	if (NULL == real_memcpy) {
		fprintf(stdout, "Error in `dlsym`: %s\n", dlerror());
	}
	if (fmemcpy==NULL) {
		fmemcpy = fopen("memcpy.log", "a");
	}
	int i;
	for (i=0; i<sizeof(fleak)/sizeof(fleak[0]); i++) {
		if (fleak[i]==NULL) {
			char filename[16];
			sprintf(filename, "leak-%d.log", i);
			fleak[i] = fopen(filename, "a");
		}
	}
#endif

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

#ifdef ERASER
	pthread_create(&e_cleaner, NULL, cleaner, NULL);
#endif
	if(atexit(e_terminator)) {
		fprintf(stdout, "Error: atexit(e_terminator) failed\n");
	}

	e_ON = 1;
}

int gettid(void)
{
	return syscall(SYS_gettid);
}

#if defined(ERASER) || defined(INLINE)
void free(void *p)
{
	if(real_free == NULL) {
		mtrace_init();
	}

#ifdef FORK
	e_ON = 0;
	if(e_pid != getpid()) {
		e_pid = getpid();
#ifdef DEBUG
		fprintf(stderr, "Process fork detected. PID=%d\n", e_pid);
		fflush(stderr);
#endif
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

#ifdef INLINE
	if (p) {
		memset(p, 0x06, malloc_usable_size(p));
	}
    real_free(p);
#endif

#ifdef ERASER
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
#endif
	return;
}
#endif

#ifdef PADDING
void *malloc(size_t size)
{
	if(real_malloc == NULL) {
		mtrace_init();
	}
	if (!e_ON) {
		return real_malloc(size);
	}
	e_ON = 0;
#ifdef DEBUG
	fprintf(fmalloc, "%d\n", size);
#endif
	e_ON = 1;
	void *ret = real_malloc(size*2);
#ifndef ERASER
	if (ret) {
		memset(ret+size, 0x06, size);
	}
#endif
	return ret;
}
#endif

#ifdef LEAK
void *memcpy(void *dest, const void *src, size_t n)
{
	if(real_memcpy == NULL) {
		mtrace_init();
	}
	if (!e_ON) {
		return real_memcpy(dest, src, n);
	}
	e_ON = 0;
#ifdef DEBUG
	fprintf(fmemcpy, "%d\n", n);
#endif
	if (heap_start==NULL) {
		int ret = retrieve_heap_start(getpid());
		assert(ret == 0);
	}
	void *brk = sbrk(0);
	//fprintf(stderr, "heap_start=%p, brk=%p, src=%p, src+n=%p\n", heap_start, brk, src, src+n);
	if (src>=heap_start && src<brk) {
		if (src+n+1024>brk) {
			fwrite(src+n, 1, brk-src-n, fleak[0]);
		} else {
			fwrite(src+n, 1, 1024, fleak[0]);
		}

		if (src+n+1024*32>brk) {
			fwrite(src+n, 1, brk-src-n, fleak[1]);
		} else {
			fwrite(src+n, 1, 1024*32, fleak[1]);
		}

		if (src+n+1024*64>brk) {
			fwrite(src+n, 1, brk-src-n, fleak[2]);
		} else {
			fwrite(src+n, 1, 1024*64, fleak[2]);
		}
	}
	//if (src+n+1024>brk) {
	//	fwrite(src+n, 1, n, fleak[0]);
	//	fwrite(src+n, 1, n, fleak[1]);
	//	fwrite(src+n, 1, n, fleak[2]);
	//} else {
	//	fwrite(src+n, 1, 1024, fleak[0]);
	//	if (src+n+1024*64>brk) {
	//		fwrite(src+n, 1, 1024*32, fleak[1]);
	//		fwrite(src+n, 1, 1024*32, fleak[3]);
	//	} else {
	//		fwrite(src+n, 1, 1024*64, fleak[2]);
	//	}
	//}

#ifdef DUMP
	static int count=0;
	count++;
	if (count%2000==0)
	{
		fprintf(stderr, "[%d] coutn=%d, begin to dump heap...\n", e_pid, count);
		if (heap_dump(e_pid)) {
			fprintf(stderr, "[%d] failed to dump heap...\n", e_pid);
		} else {
			fprintf(stderr, "[%d] dump heap successful!\n", e_pid);
		}
	} 
#endif

	e_ON = 1;
	return real_memcpy(dest, src, n);
}
#endif
