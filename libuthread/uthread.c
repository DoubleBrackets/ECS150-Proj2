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
	/* TODO Phase 2 */
	void *stack_pointer;
	thread_state state;
	uthread_ctx_t uctx;
};

typedef struct uthread_tcb uthread_tcb;

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

/* TODO Phase 2 */
int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	thread_queue = queue_create();

	// register current thread as the "idle"
	idle_thread = malloc(sizeof(uthread_tcb));

	// Create the initial thread
	uthread_create(func, arg);

	uthread_tcb *next_thread;

	// Context switch to the init thread
	// Special case since we're switching out of idle thread, which doesn't go in the queue
	int status = queue_dequeue(thread_queue, (void **)&next_thread);
	next_thread->state = RUNNING;
	executing_thread = next_thread;
	uthread_ctx_switch(&idle_thread->uctx, &next_thread->uctx);

	return 0;
}

/* TODO Phase 2 */
int uthread_create(uthread_func_t func, void *arg)
{
	// create new thread tcb
	uthread_tcb *new_tcb = malloc(sizeof(uthread_tcb));
	new_tcb->stack_pointer = uthread_ctx_alloc_stack();
	new_tcb->state = READY;

	// queue the new thread
	queue_enqueue(thread_queue, new_tcb);

	// initialize user thread context
	uthread_ctx_init(&new_tcb->uctx, new_tcb->stack_pointer, func, arg);
}

/* TODO Phase 2 */
void uthread_yield(void)
{
	// printf("Thread yielding\n");

	// Now we schedule the next thread
	// Debug
	// printf("The thread queue has %d threads\n", queue_length(thread_queue));
	// queue_iterate(thread_queue, debug_print_tcb);

	uthread_tcb *next_thread;

	// printf("Get next thread\n");
	// grab next thread in the queue
	int status = queue_dequeue(thread_queue, (void **)&next_thread);

	// No more threads in the queue, return to idle
	if (status == -1)
	{
		// printf("No more threads, exit\n");
		return 0;
	}

	next_thread->state = RUNNING;

	if (executing_thread->state == RUNNING)
	{
		// printf("Requeuing yielded thread\n");
		executing_thread->state = READY;

		// requeue the thread we're yielding
		queue_enqueue(thread_queue, executing_thread);
	}

	// Make sure to update executing_thread before we context switch
	uthread_tcb *previous_thread = executing_thread;
	executing_thread = next_thread;

	// switch to the next thread to run
	uthread_ctx_switch(&previous_thread->uctx, &next_thread->uctx);

	if (previous_thread->state == EXITED)
	{
		printf("Zombie thread, collecting\n");
		uthread_ctx_destroy_stack(executing_thread->stack_pointer);
		free(executing_thread);
	}

	// printf("Finished yielding\n");
}

/* TODO Phase 2 */
void uthread_exit(void)
{
	// mark as zombie for later collection
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
