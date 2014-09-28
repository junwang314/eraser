all:test mfree.so

test:test.c
	gcc -ldl test.c

mfree.so:mfree.c
	gcc -shared -fPIC -g -o mfree.so mfree.c 

run:all
	LD_PRELOAD=./mfree.so ./a.out
