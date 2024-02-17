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
sigset_t *block_alarm;

void preempt_handler()
{
	uthread_yield();
}

void preempt_disable(void)
{
	sigprocmask(SIG_BLOCK, block_alarm, NULL);
}

void preempt_enable(void)
{
	sigprocmask(SIG_UNBLOCK, block_alarm, NULL);
}

void preempt_start(bool preempt)
{
	// signal block mask for later
	block_alarm = malloc(sizeof(sigset_t));
	sigemptyset(block_alarm);
	sigaddset(block_alarm, SIGVTALRM);

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

	if (preempt)
	{
		setitimer(ITIMER_VIRTUAL, timer_val, NULL);
	}
}

void preempt_stop(void)
{
	// set timer back to default
	timer_val->it_interval.tv_usec = 0;
	timer_val->it_value.tv_usec = 0;
	setitimer(ITIMER_VIRTUAL, timer_val, NULL);

	// set handler back to default
	handler_action->sa_handler = SIG_DFL;
	sigaction(SIGVTALRM, handler_action, NULL);

	free(timer_val);
	free(handler_action);
	free(block_alarm);
}
