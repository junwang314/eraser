#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "cleaner.h"
#include "util.h"

static int do_fprintf=1;

static void mtrace_init(void)
{
    real_free = dlsym(RTLD_NEXT, "free");
    if (NULL == real_free) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
	pthread_create(&e_cleaner, NULL, cleaner, NULL);
}

int gettid(void)
{
    return syscall(SYS_gettid);
}

void free(void *p)
{
    if(real_free==NULL) {
        mtrace_init();
        do_fprintf=1;
    }

	if (e_ON) {
		e_enque(p);
		//putchar("e");
		//putchar("\n");
	} else {
		real_free(p);
		return;
	}

    if (do_fprintf) {
        do_fprintf=0;
        fprintf(stderr, "%d/%d: free(%p)\n", getpid(), gettid(), p);
        do_fprintf=1;
        return;
    } else  {
        //real_free(p);
        return;
    }
}
