/*
 * Preemption simple test
 *
 * Test preemption by having a thread spin delay and attempt to starve the other thread
 *
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sem.h>
#include <uthread.h>

static void thread2(void *arg)
{
	(void)arg;

	for (int i = 0; i < 10; i++)
	{
		printf("thread2\n");
	}
	printf("done\n");
}

static void thread1(void *arg)
{
	(void)arg;

	uthread_create(thread2, NULL);

	for (int j = 0; j < 5; j++)
	{
		int dum;
		for (int i = 0; i < 10000000; ++i)
		{
			// spin
			dum = i;
			dum++;
		}
		printf("Thread 1 %d\n", dum);
	}
}

int main(void)
{

	uthread_run(true, thread1, NULL);

	return 0;
}
