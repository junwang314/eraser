CFLAGS  = -Wall -O2 -shared -fPIC -march=native 
LDFLAGS = -ldl -pthread # "-ldl" is for dlsym().

all:test mfree.so mfree_fork.so mfree_spec.so mfree_fork_dump.so

test:test.c
	gcc -Wl,--no-as-needed -ldl -g test.c

mfree.so:mfree.c cleaner.h util.h
	gcc $(CFLAGS) -DDEBUG -g -o mfree.so mfree.c $(LDFLAGS)

mfree_fork.so:mfree.c cleaner.h util.h
	gcc $(CFLAGS) -DFORK -DDEBUG -g -o mfree_fork.so mfree.c $(LDFLAGS)

mfree_fork_dump.so:mfree.c cleaner.h util.h heap_dump.h
	gcc $(CFLAGS) -DFORK -DDUMP -DDEBUG -g -o mfree_fork_dump.so mfree.c $(LDFLAGS)

mfree_spec.so:mfree.c cleaner.h util.h
	gcc $(CFLAGS) -DSPEC -g -o mfree_spec.so mfree.c $(LDFLAGS)

run:all
	LD_PRELOAD=./mfree_fork.so ./a.out

clean:
	rm *.so *.out
