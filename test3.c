#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<malloc.h>
#include<assert.h>

#define SZ 1024*20

int test(){

	void *p[SZ];
	
	int i;
	for (i=0; i<SZ; i++) {
		p[i] = malloc(i+10);
		if (p[i]==0) {
			printf("malloc(%d) failed\n", i+10);
		}
		*(int *)p[i] = 12345;
		//printf(".");
	}

	int sz;
	for (i=0; i<SZ; i++) {
		if (p[i]) {
			sz = malloc_usable_size(p[i]);
			assert(sz>=i+10);
			free(p[i]);
		}
		//printf("+");
	}
	//printf("done!\n");
	return 0;
}

int main()
{
	int i;
	int ret;
	for (i=0; i<20; i++) {
		ret = test();
	}
	return ret;
}
