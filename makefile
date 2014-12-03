CC = gcc
CFLAGS  = -Wall -O2 -shared -fPIC -march=native 
LDFLAGS = -ldl -pthread # "-ldl" is for dlsym().
CFLAGS_TEST = -Wl,--no-as-needed -ldl -g

all:test mfree.so mfree_fork.so SPEC heap_dump experiment

#########################################################################
test:test.c
	$(CC) $(CFLAGS_TEST) test.c

mfree.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DDEBUG -g -o mfree.so mfree.c $(LDFLAGS)

mfree_fork.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DFORK -DDEBUG -g -o mfree_fork.so mfree.c $(LDFLAGS)

#########################################################################
# SPEC
SPEC:mfree_spec.so mfree_spec_inline.so

mfree_spec.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DSPEC -g -o mfree_spec.so mfree.c $(LDFLAGS)

mfree_spec_inline.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DINLINE -DSPEC -g -o mfree_spec_inline.so mfree.c $(LDFLAGS)

#########################################################################
# heap dump
heap_dump:mfree_fork_dump.so
mfree_fork_dump.so:mfree.c cleaner.h util.h heap_dump.h
	$(CC) $(CFLAGS) -DERASER -DFORK -DLEAK -DDUMP -DDEBUG -g -o mfree_fork_dump.so mfree.c $(LDFLAGS)


#########################################################################
# case study
experiment:mfree_padding.so mfree_inline.so mfree_eraser.so mfree_eraser_plus.so

mfree_padding.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DDEBUG -DPADDING -DLEAK -g -o mfree_padding.so mfree.c $(LDFLAGS)

mfree_inline.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DDEBUG -DINLINE -DLEAK -g -o mfree_inline.so mfree.c $(LDFLAGS)

mfree_eraser.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DFORK -DDEBUG -DLEAK -g -o mfree_eraser.so mfree.c $(LDFLAGS)

mfree_eraser_plus.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DFORK -DDEBUG -DPADDING -DLEAK -g -o mfree_eraser_plus.so mfree.c $(LDFLAGS)

check:test_padding test_inline test_eraser test_eraser_plus

test_padding:test_padding.c mfree_padding.so
	$(CC) $(CFLAGS_TEST) -o test_padding.out $<
	LD_PRELOAD=./mfree_padding.so ./test_padding.out

test_inline:test_inline.c mfree_inline.so
	$(CC) $(CFLAGS_TEST) -o test_inline.out $<
	LD_PRELOAD=./mfree_inline.so ./test_inline.out

test_eraser:test_eraser.c mfree_eraser.so
	$(CC) $(CFLAGS_TEST) -o test_eraser.out $<
	LD_PRELOAD=./mfree_eraser.so ./test_eraser.out 

test_eraser_plus:test_eraser_plus.c mfree_eraser_plus.so
	$(CC) $(CFLAGS_TEST) -o test_eraser_plus.out $<
	LD_PRELOAD=./mfree_eraser_plus.so ./test_eraser_plus.out 

#########################################################################
run:all
	LD_PRELOAD=./mfree_fork.so ./a.out

clean:
	rm *.so *.out
