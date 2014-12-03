#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>

#define SZ 26

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
	void *p;
    p = malloc(SZ);
    printf("malloc size=%d\n", SZ);
    printf("malloc_usable_size=%d\n", malloc_usable_size(p));
	print(p, SZ*2);
    free(p);

	size_t sz = SZ*2;
    p = malloc(sz);
    printf("malloc size=%d\n", sz);
    printf("malloc_usable_size=%d\n", malloc_usable_size(p));
	print(p, sz*2);
    free(p);

	sz = SZ;
    p = malloc(sz);
    printf("malloc size=%d\n", sz);
    printf("malloc_usable_size=%d\n", malloc_usable_size(p));
	print(p, sz*2);
    free(p);

	sz = SZ/2;
    p = malloc(sz);
    printf("malloc size=%d\n", sz);
    printf("malloc_usable_size=%d\n", malloc_usable_size(p));
	print(p, sz*2);
    free(p);
    return 0;
}
