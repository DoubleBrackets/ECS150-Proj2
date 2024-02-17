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

	for (int i = 0; i < 5; i++)
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
		// you've activated my trap card yugi, prepare to have your process blocked by my while loop of greed!
	}

	printf("thread1\n");
}

int main(void)
{

	uthread_run(true, thread1, NULL);

	return 0;
}
