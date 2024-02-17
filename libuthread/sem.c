#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"

struct semaphore
{
	queue_t wait_queue;
	int count;
};

typedef struct semaphore semaphore;

sem_t sem_create(size_t count)
{

	sem_t new_sem = malloc(sizeof(semaphore));

	if (new_sem == NULL)
	{
		return NULL;
	}

	new_sem->wait_queue = queue_create();
	if (new_sem->wait_queue == NULL)
	{
		free(new_sem);
		return NULL;
	}

	new_sem->count = count;

	return new_sem;
}

int sem_destroy(sem_t sem)
{

	if (sem == NULL || queue_length(sem->wait_queue) > 0)
	{
		return -1;
	}

	queue_destroy(sem->wait_queue);
	free(sem);

	return 0;
}

int sem_down(sem_t sem)
{

	if (sem == NULL)
	{
		return -1;
	}

	if (sem->count == 0)
	{
		queue_enqueue(sem->wait_queue, uthread_current());
		uthread_block();
	}
	else
	{
		sem->count--;
	}

	return 0;
}

int sem_up(sem_t sem)
{

	if (sem == NULL)
	{
		return -1;
	}

	sem->count++;

	// unblock next in queue
	// make sure to decrement to prevent stealing before the scheduler runs
	struct uthread_tcb *next_thread = NULL;

	queue_dequeue(sem->wait_queue, (void **)&next_thread);

	if (next_thread != NULL && sem->count > 0)
	{
		sem->count--;
		uthread_unblock(next_thread);
	}

	return 0;
}
