#ifndef __UTIL_H
#define __UTIL_H

#include <sched.h>
#include <semaphore.h>

static __thread int e_ON = 1;


#define QUEUE_ELEMENTS 5000
#define QUEUE_SIZE (QUEUE_ELEMENTS + 1)
static void* e_queue[QUEUE_SIZE];
static int e_head, e_tail;
static sem_t sem_full;
static sem_t sem_empty;

static struct timespec interval;

void e_queue_init()
{
	e_head = 0;
	e_tail = 0;
	sem_init(&sem_empty, 0, QUEUE_ELEMENTS);
	sem_init(&sem_full, 0, 0);
	interval.tv_sec = 1;
	interval.tv_nsec = 0;
}

void e_queue_enque(void *p)
{
	sem_wait(&sem_empty);
	e_queue[e_head] = p;
	e_head = (e_head + 1) % QUEUE_SIZE;
	//fprintf(stderr, "%s: p=%p\n", __func__, p);
	sem_post(&sem_full);
}

void *e_queue_deque()
{
	void *p;
	if (sem_timedwait(&sem_full, &interval)) {
		return NULL;
	}
	p = e_queue[e_tail];
	e_tail = (e_tail + 1) % QUEUE_SIZE;
	//fprintf(stdout, "%s: p=%p\n", __func__, p);
	sem_post(&sem_empty);
	return p;
}

#endif // __UTIL_H
