/*
 * Semaphore simple test
 *
 * Test the synchronization of three threads, by having them print messages in
 * a certain order.
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sem.h>
#include <uthread.h>

static void thread2(void *arg)
{
	(void)arg;

	for (int i = 0; i < 100; i++)
	{
		printf("thread2\n");
		uthread_yield();
	}
	printf("done\n");
}

static void thread1(void *arg)
{
	(void)arg;

	uthread_create(thread2, NULL);

	while (true)
	{
		// hang
	}

	printf("thread1\n");
}

int main(void)
{

	uthread_run(false, thread1, NULL);

	return 0;
}
