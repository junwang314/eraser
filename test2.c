#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(){

	int *p;
	int i;
	
	p = malloc(sizeof(int) * 5);

	for(i = 0; i < 5; i++){
		p[i] = i;
	}
	
	printf("p[2] before free is %d (expected 2)\n", p[2]);
	
	free(p);
	
	// The cleaner thread needs sometime to clear the memory
	sleep(2);
	
	printf("p[2] after free is %d (expected 0)\n", p[2]);
	
}
