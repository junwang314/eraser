#ifndef __UTIL_H
#define __UTIL_H

#include <sched.h>

static __thread int e_ON = 1;


#define QUEUE_ELEMENTS 100
#define QUEUE_SIZE (QUEUE_ELEMENTS + 1)
static void* e_queue[QUEUE_SIZE];
int e_head, e_tail;

void e_enque(void *p)
{
	if (!p) {
		return;
	}

	if(e_head == (( e_tail - 1 + QUEUE_SIZE) % QUEUE_SIZE))
	{
		exit(-1);
		return; /* e_queue Full*/
	}
	e_queue[e_head] = p;
	e_head = (e_head + 1) % QUEUE_SIZE;
}

void *e_deque()
{
	void *p;
	while(e_head == e_tail)
	{
		sched_yield(); /* e_queue Empty - nothing to get*/
	}
	p = e_queue[e_tail];
	e_tail = (e_tail + 1) % QUEUE_SIZE;
	return p;
}

#endif // __UTIL_H
