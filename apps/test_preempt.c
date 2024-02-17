/*
 * Preemption simple test
 *
 * Test preemption by having a thread never yield
 *
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

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

	while (true)
	{
		// hang
	}
}

int main(void)
{

	uthread_run(true, thread1, NULL);

	return 0;
}
