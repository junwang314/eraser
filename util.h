#ifndef __UTIL_H
#define __UTIL_H

#include <sched.h>
#include <assert.h>

#define QUEUE_SIZE 1024

/* per-application-thread queue data structure*/
typedef struct e_thread_queue {
	unsigned produceCount;
	unsigned consumeCount;
	void **queue;
	struct e_thread_queue *next;
	pthread_t tid;
} e_thread_queue;

static __thread int e_ON = 1;
static __thread e_thread_queue *e_thq = NULL;

static e_thread_queue *e_thread_queue_head = NULL;

static inline void e_thread_queue_init(e_thread_queue * q, pthread_t tid)
{
	q->produceCount = 0;
	q->consumeCount = 0;
	q->queue = malloc(QUEUE_SIZE*sizeof(void *));
	//q->next field should be populated with __sync_bool_compare_and_swap()
	q->tid = tid;
}

/* e_ON is 0 when calling this function */
e_thread_queue* e_get_thread_queue()
{
	e_thread_queue *q, *oldhead;
	pthread_t self = pthread_self();
	//try to reuse a e_thread_queue first
	for (q = e_thread_queue_head; q!=NULL; q=q->next) {
		if (q->tid == 0 && __sync_bool_compare_and_swap(&q->tid, 0, self)) {
			return q;
		}
	}
	//allocate new queue
	q = (e_thread_queue *)malloc(sizeof(*q));
	assert(q);
	//insert new queue as new head
	do {
		oldhead = e_thread_queue_head;
		q->next = oldhead;
	} while(!__sync_bool_compare_and_swap(&e_thread_queue_head, oldhead, q));
	e_thread_queue_init(q, self);
#ifdef DEBUG
	fprintf(stderr, "%s: %lu, q=%p\n", __func__, self, q);
#endif
	return q;
}

static inline unsigned toIndex (unsigned i)
{
	return (unsigned)(i & (QUEUE_SIZE-1));
}

/* e_ON is 0 when calling this function */
void e_queue_enque(void *p, e_thread_queue *q)
{
	while (q->produceCount - q->consumeCount >= QUEUE_SIZE) {
		//sched_yeild();
#ifdef DEBUG
		fprintf(stdout, "%s: queue full\n", __func__);
#endif
		sleep(0);
	} 
	q->queue[toIndex(q->produceCount)] = p;
	q->produceCount++;
	//fprintf(stderr, "%s: p=%p\n", __func__, p);
}

void *e_queue_deque(e_thread_queue *q)
{
	void *p;
	if (q->produceCount == q->consumeCount) {
		return NULL;
	}
	p = q->queue[toIndex(q->consumeCount)];
	q->consumeCount++;
	//fprintf(stdout, "%s: p=%p\n", __func__, p);
	return p;
}

#endif // __UTIL_H
