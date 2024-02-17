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
	EXITED,
	BLOCKED
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
	uthread_ctx_destroy_stack(thread->stack_pointer);
	free(thread);
}

// Use global state for the thread library (a bit like a singleton?)
queue_t ready_queue;
uthread_tcb *executing_thread;
uthread_tcb *idle_thread;

struct uthread_tcb *uthread_current(void)
{
	return executing_thread;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	ready_queue = queue_create();

	// register current thread as the "idle"
	idle_thread = malloc(sizeof(uthread_tcb));

	if (idle_thread == NULL)
	{
		queue_destroy(ready_queue);
		return -1;
	}

	// Create the initial thread
	if (uthread_create(func, arg) == -1)
	{
		free(idle_thread);
		queue_destroy(ready_queue);
		return -1;
	}

	uthread_tcb *next_thread;

	// Context switch to the init thread
	// Special case since we're switching out of idle thread, which doesn't go in the queue
	int status = queue_dequeue(ready_queue, (void **)&next_thread);
	if (status == -1)
	{
		free(idle_thread);
		queue_destroy(ready_queue);
		return -1;
	}

	next_thread->state = RUNNING;
	executing_thread = next_thread;

	preempt_start(preempt);

	// Disable preemption in the idle thread
	preempt_disable();

	// Start execution of threads
	uthread_ctx_switch(&idle_thread->uctx, &next_thread->uctx);

	preempt_stop();

	// free remaining resourecs
	free(idle_thread);
	queue_destroy(ready_queue);

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

	// queueing and setting up thread context should be atomic
	// being interrupted could result in a broken queue, or an uninitialized thread in the queue
	preempt_disable();

	// queue the new thread
	if (queue_enqueue(ready_queue, new_tcb) == -1)
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

	preempt_enable();

	return 0;
}

void uthread_yield(void)
{
	// Yielding threads should not be interrupted so that the next thread can be properly scheduled
	// If it is interrupted, the thread it tries to schedule next could be wrong
	preempt_disable();

	uthread_tcb *next_thread;

	// Requeue thread we're yielding from
	// If it's a zombie or blocked thread, make sure to leave the state untouched
	// We do this before dequeueing in case the yielding thread is the only one
	if (executing_thread->state == RUNNING)
	{
		executing_thread->state = READY;
	}
	queue_enqueue(ready_queue, executing_thread);

	int len = queue_length(ready_queue);
	for (int i = 0; i < len; i++)
	{
		queue_dequeue(ready_queue, (void **)&next_thread);

		// Zombie thread, collect
		if (next_thread->state == EXITED)
		{
			free_thread(next_thread);
		}
		else if (next_thread->state == BLOCKED)
		{
			// cycle to back of the queue
			queue_enqueue(ready_queue, next_thread);
		}
		else
		{
			// The only valid thread is the one we just yielded from, so just continue execution
			if (next_thread == executing_thread)
			{
				preempt_enable();
				return;
			}
			// found a ready thread, move on to context switching to it
			break;
		}
	}

	// No threads remaining in the queue, return to idle thread to finish
	if (queue_length(ready_queue) == 0)
	{
		uthread_ctx_switch(&executing_thread->uctx, &idle_thread->uctx);
	}

	next_thread->state = RUNNING;

	// Make sure to update executing_thread before we context switch
	uthread_tcb *previous_thread = executing_thread;
	executing_thread = next_thread;

	// switch to the next thread to run
	uthread_ctx_switch(&previous_thread->uctx, &next_thread->uctx);

	preempt_enable();
}

void uthread_exit(void)
{
	executing_thread->state = EXITED;
	uthread_yield();
}

void uthread_block(void)
{
	executing_thread->state = BLOCKED;
	uthread_yield();
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	uthread->state = READY;
}
