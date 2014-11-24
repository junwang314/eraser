#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>

#define SZ 26

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


struct test {
	int a;
	int b;
	float c;
	void *d;
	int *f;
};

void print(unsigned char *p, int size)
{
    int i;
    for (i=0; i<size; i++) {
        printf("%02x ", p[i]);
    }
    printf("\n");
    return;
}

int main()
{
    int i;
    unsigned char *p;
    unsigned char *q;
    p = (unsigned char *)malloc(SZ);
    printf("malloc:\t");
    print(p, SZ);

    for (i=0; i<SZ; i++) {
        p[i] = 0xff;
    }
    printf("init:\t");
    print(p, SZ);

    mchunkptr chunk;
    chunk = mem2chunk(p);
    printf("chunk size=%d\n", chunksize(chunk));
    //memset(p, 0, malloc_usable_size(p));
    printf("malloc_usable_size=%d\n", malloc_usable_size(p));
    free(p);
    printf("free:\t");
    print(p, SZ);

    p = (unsigned char *)malloc(SZ);
    printf("malloc:\t");
    print(p, SZ);

	struct test *ptr = NULL;
	printf("%d, %d\n", sizeof(*ptr), sizeof(struct test));

	//for (i=0; i<10000; i++) {
	//	p = malloc(i);
	//	printf("%p\n", p);
	//	free(p);
	//}

    p = malloc(SZ);
    for (i=0; i<SZ; i++) {
        p[i] = 0xff;
    }
    q = malloc(2*SZ);
    print(q, 2*SZ);
    memcpy(q, p, SZ);
    print(q, 2*SZ);
    sleep(100);
    return 0;
}
