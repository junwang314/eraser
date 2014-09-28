#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

/* Forward declarations.  */
struct malloc_chunk;
typedef struct malloc_chunk* mchunkptr;
/*
   INTERNAL_SIZE_T is the word-size used for internal bookkeeping
   of chunk sizes.

   The default version is the same as size_t.
*/
#define INTERNAL_SIZE_T size_t
/* The corresponding word size */
#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))

/*
   This struct declaration is misleading (but accurate and necessary).
   It declares a "view" into memory allowing access to necessary
   fields at known offsets from a given base. See explanation below.
 */

struct malloc_chunk {

    INTERNAL_SIZE_T      prev_size;  /* Size of previous chunk (if free).  */
    INTERNAL_SIZE_T      size;       /* Size in bytes, including overhead. */

    struct malloc_chunk* fd;         /* double links -- used only if free. */
    struct malloc_chunk* bk;

    /* Only used for large blocks: pointer to next larger size.  */
    struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
    struct malloc_chunk* bk_nextsize;
};

/* conversion from malloc headers to user pointers, and back */

#define chunk2mem(p)   ((void*)((char*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))


/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */
#define PREV_INUSE 0x1

/* extract inuse bit of previous chunk */
#define prev_inuse(p)       ((p)->size & PREV_INUSE)


/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */
#define IS_MMAPPED 0x2

/* check for mmap()'ed chunk */
#define chunk_is_mmapped(p) ((p)->size & IS_MMAPPED)


/* size field is or'ed with NON_MAIN_ARENA if the chunk was obtained
   from a non-main arena.  This is only set immediately before handing
   the chunk to the user, if necessary.  */
#define NON_MAIN_ARENA 0x4

/* check for chunk from non-main arena */
#define chunk_non_main_arena(p) ((p)->size & NON_MAIN_ARENA)

/*
   Bits to mask off when extracting size

Note: IS_MMAPPED is intentionally not masked off from size field in
macros for which mmapped chunks should never be seen. This should
cause helpful core dumps to occur if it is tried by accident by
people extending or adapting this malloc.
 */
#define SIZE_BITS (PREV_INUSE|IS_MMAPPED|NON_MAIN_ARENA)

/* Get size, ignoring use bits */
#define chunksize(p)         ((p)->size & ~(SIZE_BITS))


static void* (*real_free)(void *)=NULL;
static int do_trace=1;


static void mtrace_init(void)
{
    real_free = dlsym(RTLD_NEXT, "free");
    if (NULL == real_free) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
}

int gettid(void)
{
    return syscall(SYS_gettid);
}

void free(void *p)
{
    if(real_free==NULL) {
        mtrace_init();
        do_trace=1;
    }

    size_t size;
    memset(p, 0, malloc_usable_size(p));
    real_free(p);

    if (do_trace) {
        do_trace=0;
        //fprintf(stderr, "%d/%d: free(%p)\n", getpid(), gettid(), p);
        do_trace=1;
        return;
    } else  {
        //real_free(p);
        return;
    }
}
