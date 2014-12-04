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

    printf("malloc_usable_size=%d\n", malloc_usable_size(p));
    free(p);
    printf("free:\t");
    print(p, SZ);

	sleep(1);
    p = (unsigned char *)malloc(SZ);
    printf("malloc:\t");
    print(p, SZ);

	char buf[1024];
	memcpy(buf, p, 1024);
    return 0;
}
