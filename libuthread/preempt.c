#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

struct itimerval *timer_val;
struct sigaction *handler_action;

void preempt_handler(int signum)
{
	uthread_yield();
}

void preempt_disable(void)
{
	sigaddset(&handler_action->sa_mask, SIGVTALRM);
}

void preempt_enable(void)
{
	sigdelset(&handler_action->sa_mask, SIGVTALRM);
}

void preempt_start(bool preempt)
{
	if (!preempt)
	{
		return;
	}

	// setup handler
	handler_action = malloc(sizeof(struct sigaction));
	handler_action->sa_handler = preempt_handler;
	sigemptyset(&handler_action->sa_mask);
	handler_action->sa_flags = 0;

	sigaction(SIGVTALRM, handler_action, NULL);

	// setup timer
	timer_val = malloc(sizeof(struct itimerval));
	timer_val->it_interval.tv_usec = 1E6 / HZ;
	timer_val->it_value.tv_usec = 1E6 / HZ;

	setitimer(ITIMER_VIRTUAL, timer_val, NULL);
}

void preempt_stop(void)
{
	free(timer_val);
	free(handler_action);
}
