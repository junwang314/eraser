all:test mfree.so

test:test.c
	gcc -Wl,--no-as-needed -ldl -g test.c

mfree.so:mfree.c cleaner.h util.h
	gcc -ldl -pthread -shared -fPIC -g -o mfree.so mfree.c 

run:all
	LD_PRELOAD=./mfree.so ./a.out

clean:
	rm *.so *.out
