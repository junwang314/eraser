CC = gcc
CFLAGS  = -Wall -O0 -shared -fPIC -march=native 
LDFLAGS = -ldl -pthread # "-ldl" is for dlsym().
CFLAGS_TEST = -Wl,--no-as-needed -ldl -g

all:test mfree.so mfree_fork.so SPEC heap_dump experiment-effectiveness experiment-performance

#########################################################################
test:test.c
	$(CC) $(CFLAGS_TEST) test.c

mfree.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DDEBUG -g -o mfree.so mfree.c $(LDFLAGS)

mfree_fork.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DFORK -DDEBUG -g -o mfree_fork.so mfree.c $(LDFLAGS)

#########################################################################
# SPEC
SPEC:spec_mfree_padding.so spec_mfree_inline.so spec_mfree_eraser.so spec_mfree_eraser_plus.so

spec_mfree_padding.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DPADDING -DSPEC -g -o spec_mfree_padding.so mfree.c -ldl

spec_mfree_inline.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DINLINE -DSPEC -g -o spec_mfree_inline.so mfree.c -ldl

spec_mfree_eraser.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DSPEC -g -o spec_mfree_eraser.so mfree.c $(LDFLAGS)

spec_mfree_eraser_plus.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DPADDING -DSPEC -g -o spec_mfree_eraser_plus.so mfree.c $(LDFLAGS)

#########################################################################
# heap dump
heap_dump:mfree_fork_dump.so
mfree_fork_dump.so:mfree.c cleaner.h util.h heap_dump.h
	$(CC) $(CFLAGS) -DERASER -DFORK -DLEAK -DDUMP -DDEBUG -g -o mfree_fork_dump.so mfree.c $(LDFLAGS)


#########################################################################
# case study - effectiveness
experiment-effectiveness:mfree_baseline.so mfree_padding.so mfree_inline.so mfree_eraser.so mfree_eraser_plus.so

mfree_baseline.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DDEBUG -DLEAK -g -o mfree_baseline.so mfree.c $(LDFLAGS)

mfree_padding.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DDEBUG -DPADDING -DLEAK -g -o mfree_padding.so mfree.c $(LDFLAGS)

mfree_inline.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DDEBUG -DINLINE -DLEAK -g -o mfree_inline.so mfree.c $(LDFLAGS)

mfree_eraser.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DFORK -DDEBUG -DLEAK -g -o mfree_eraser.so mfree.c $(LDFLAGS)

mfree_eraser_plus.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DFORK -DDEBUG -DPADDING -DLEAK -g -o mfree_eraser_plus.so mfree.c $(LDFLAGS)

check:test_baseline test_padding test_inline test_eraser test_eraser_plus

test_baseline:test_baseline.c mfree_baseline.so
	$(CC) $(CFLAGS_TEST) -o test_baseline.out $<
	LD_PRELOAD=./mfree_baseline.so ./test_baseline.out

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
# case study - performance
experiment-performance:perf_mfree_padding.so perf_mfree_inline.so perf_mfree_eraser.so perf_mfree_eraser_plus.so

perf_mfree_padding.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DPADDING -g -o perf_mfree_padding.so mfree.c -ldl

perf_mfree_inline.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DINLINE -g -o perf_mfree_inline.so mfree.c -ldl

perf_mfree_eraser.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DFORK -g -o perf_mfree_eraser.so mfree.c $(LDFLAGS)

perf_mfree_eraser_plus.so:mfree.c cleaner.h util.h
	$(CC) $(CFLAGS) -DERASER -DFORK -DPADDING -g -o perf_mfree_eraser_plus.so mfree.c $(LDFLAGS)

#########################################################################
test3.out:test3.c
	$(CC) $(CFLAGS_TEST) -o test3.out test3.c

#########################################################################
run:all
	LD_PRELOAD=./mfree_fork.so ./a.out

clean:
	rm *.so *.out
