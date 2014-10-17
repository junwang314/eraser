all:test mfree.so mfree_fork.so

test:test.c
	gcc -Wl,--no-as-needed -ldl -g test.c

mfree.so:mfree.c cleaner.h util.h
	gcc -ldl -pthread -shared -fPIC -g -o mfree.so mfree.c 

mfree_fork.so:mfree.c cleaner.h util.h
	gcc -ldl -pthread -shared -fPIC -g -o mfree_fork.so mfree.c -DFORK

run:all
	LD_PRELOAD=./mfree_fork.so ./a.out

clean:
	rm *.so *.out
