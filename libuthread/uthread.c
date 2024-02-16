#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

enum thread_state
{
	RUNNING,
	READY,
	EXITED
};

typedef enum thread_state thread_state;

struct uthread_tcb
{
	void *stack_pointer;
	thread_state state;
	uthread_ctx_t uctx;
};

typedef struct uthread_tcb uthread_tcb;

void free_thread(uthread_tcb *thread)
{
	// printf("Collecting thread\n");
	uthread_ctx_destroy_stack(thread->stack_pointer);
	free(thread);
}

// Use global state for the thread library (a bit like a singleton?)
queue_t thread_queue;
uthread_tcb *executing_thread;
uthread_tcb *idle_thread;

/* TODO Phase 2/3 */
struct uthread_tcb *uthread_current(void)
{
	return executing_thread;
}

void debug_print_tcb(queue_t q, void *thread)
{
	printf("Thread: State %d\n", ((uthread_tcb *)thread)->state);
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	thread_queue = queue_create();

	// register current thread as the "idle"
	idle_thread = malloc(sizeof(uthread_tcb));

	if (idle_thread == NULL)
	{
		return -1;
	}

	// Create the initial thread
	if (uthread_create(func, arg) == -1)
	{
		free(idle_thread);
		return -1;
	}

	uthread_tcb *next_thread;

	// Context switch to the init thread
	// Special case since we're switching out of idle thread, which doesn't go in the queue
	int status = queue_dequeue(thread_queue, (void **)&next_thread);
	if (status == -1)
	{
		free_thread(next_thread);
		free(idle_thread);
	}

	next_thread->state = RUNNING;
	executing_thread = next_thread;

	// Start execution of threads
	uthread_ctx_switch(&idle_thread->uctx, &next_thread->uctx);

	// Free remaining resources
	free(idle_thread);
	free_thread(executing_thread);
	queue_destroy(thread_queue);

	return 0;
}

int uthread_create(uthread_func_t func, void *arg)
{
	// create new thread tcb
	uthread_tcb *new_tcb = malloc(sizeof(uthread_tcb));

	if (new_tcb == NULL)
	{
		return -1;
	}

	new_tcb->stack_pointer = uthread_ctx_alloc_stack();

	if (new_tcb->stack_pointer == NULL)
	{
		// only need to free the tcb, since stack failed to malloc
		free(new_tcb);
		return -1;
	}

	new_tcb->state = READY;

	// queue the new thread
	if (queue_enqueue(thread_queue, new_tcb) == -1)
	{
		free_thread(new_tcb);
		return -1;
	}

	// initialize user thread context
	if (uthread_ctx_init(&new_tcb->uctx, new_tcb->stack_pointer, func, arg) == -1)
	{
		free_thread(new_tcb);
		return -1;
	}

	return 0;
}

void uthread_yield(void)
{
	// Now we schedule the next thread
	// Debug
	// printf("The thread queue has %d threads\n", queue_length(thread_queue));
	// queue_iterate(thread_queue, debug_print_tcb);

	uthread_tcb *next_thread;

	while (true)
	{
		// No more threads in the queue, return to idle
		if (queue_length(thread_queue) == 0)
		{
			uthread_ctx_switch(&executing_thread->uctx, &idle_thread->uctx);
		}

		int status = queue_dequeue(thread_queue, (void **)&next_thread);

		// Zombie thread, collect
		if (next_thread->state == EXITED)
		{
			free_thread(next_thread);
		}
		else
		{
			// found a ready thread, move on to context switching to it
			break;
		}
	}

	next_thread->state = RUNNING;

	// Requeue thread we're yielding from
	// If it's a zombie thread, make sure to leave the state untouched for collection later
	if (executing_thread->state == RUNNING)
	{
		executing_thread->state = READY;
	}
	queue_enqueue(thread_queue, executing_thread);

	// Make sure to update executing_thread before we context switch
	uthread_tcb *previous_thread = executing_thread;
	executing_thread = next_thread;

	// switch to the next thread to run
	uthread_ctx_switch(&previous_thread->uctx, &next_thread->uctx);
}

void uthread_exit(void)
{
	//  mark as zombie for later collection
	uthread_current()->state = EXITED;

	// yield for now
	uthread_yield();
}

/* TODO Phase 3 */
void uthread_block(void)
{
}

/* TODO Phase 3 */
void uthread_unblock(struct uthread_tcb *uthread)
{
}
