# User-Level Thread Library Report

## Queue API 
### Implementation
The queue was implemented using a two-directional linked list. We defined a
simple node structure with a pointer to the next and previous node, as well as a
`void*` variable to hold the generic data.

The backwards pointer ended up being fairly useless, but we left it there since
no harm.

Nothing particularly interesting about the implementation; the iterate function
is just a while loop that follows the linked list and calls the provided
function pointer on each node's data.

Since the queue was not required to be thread safe, there are no safeguards
against race conditions inside the queue library itself.
### Testing
Testing was done by extending the provided skeleton queue_tester, and adding
test cases for each function in the API. The tests are simple and mostly consist of
adding/removing items from the queue while also asserting the length or the dequeued values.
## uthread API
### Implementation

The uthread implementation is roughly structured as so:

A single thread is represented by the `uthread_tcb` thread control block struct, which contains
- A `void*` pointer to its own stack
- Its current state (Running, Ready, Exited, or Blocked)
- A `uthread_ctx_t` variable; the user thread context

The `void*` stack pointer is only used for freeing the stack later.

all non-executing tcbs are kept inside a single thread queue, alongside two additional
variables;
- one to track the currently executing thread
- one to track the idle thread

For ease of implementation, the queue and tcb variables are left in global
scope; our thread library is essentially a service/singleton (which seems to be
implied based on how the API handles `uthread_run`)

Upon running the thread library, the idle thread is created in a non-standard
manner since no extra
context creation is needed; the
idle thread's context is just the main thread that is already running, so on context switching
the idle thread tcb's `uthread_ctx` will save the correct context.

The nqueue is created, preemption is started, etc.

The `init` thread is then created through `uthread_create` and executed. Once the idle thread is context
switched out of, the program will only return to it when all the threads have
exited/completed, upon which the `uthread_run` function will finally return
(after cleaning up)
#### Thread Creation
Thread creation is implemented in `uthread_create`.
First, the tcb struct is allocated alongside
the stack, and the state is set as ready.

The following steps are atomic; preemption is disabled since it uses shared
structures
- The newly created tcb is added to the queue
- The newly created tcb has its context initialized using the new stack and the
  provided function as the "entry point" (ignoring thread bootloader)

The newly created thread is not executed immediately; it simply goes to the end
of the queue as a "ready" thread.
#### Scheduling
The scheduling logic is almost entirely implemented inside of `uthread_yield`,
since the only time a new thread is scheduled for execution is when a thread
yields (either voluntarily or through preemption).

The scheduling logic is fairly straightforward round-robin:
First, The scheduler deqeues the next TCB in the queue
- If it's state is `Ready`, then we context switch to it and set its state to `Running`
- If it's state is `Blocked`, then we re-enqueue it and try the next TCB in the
  queue
- If it's state is `Exited`, then its already completed. We free its resources
  and try the next TCB in the queue

No state in the queue should ever be `Running` when the scheduling algorithm begins; the thread we're yielding from
is set to `Ready` and enqueued right before the scheduling logic starts, in case
it can be immediately executed again (e.g it is the only thread being run)

For simplicity, essentially the entire `uthread_yield` is atomic (preemption
disabled), since most of it interacts with the shared queue and other shared
state like the `executing_thread` variable.
#### Other features
Exiting a thread is simply setting its state to `Exited` and yielding; it will
be cleaned up in the scheduler, as explained above.

Blocking a thread, likewise, is simply setting its state to `Blocked' and
yielding.

Unblocking a thread is simply setting its state back to `Ready` to indicate that
it is unblocked and ready to be executed.
### Testing
Testing of the uthread API was done through running the given test files
- uthread_hello
- uthread_yield
and manually checking the results.

Further testing was implicitly done by the semaphor tests; we considered those
sufficient testing for the uthread api. 
## Semaphore API
### Implementation

The semaphore API was implemented using, at its core, a semaphore struct. This
struct stores the count of the semaphore, as well as a waiting queue for blocked
threads that request usage of the resource when its count is 0. For this queue, 
we reuse our own queue API. This queue is later used in the `sem_up()` function, where
we check if there is a waiting thread that needs access to the resource. If there
is, the semaphore should not increment and that thread should be immediately
unblocked. This prevents both problems of the corner case in the project 
document, as the thread that called `sem_down()` first will be the first to run
and will not starve.

As we added later in the phases, the semaphore API must have preempts disabled
for the `sem_down()` and `sem_up()` functions. This is in order to prevent the case in which
there are two threads calling either of the two functions on the same semaphore,
and one of them is interrupted in the function causing a race condition. As this
can result in an inconsistent value for the semaphore, we must call
`preempt_disable()` and `preempt_enable()` at the start and end of these functions.
### Testing
We tested our semaphore API using the following test files:
- sem_simple
- sem_count
- sem_buffer
- sem_prime
## Preemption
### Implementation
Preemption is implemented using `setitimer`. Upon starting preemption (called
inside `uthread_start`), the timer is started and set to an interval based on
the constant `HZ`. Every time the alarm triggers, the signal `SIGVTALRM` is fired.

In addition, a handler action is set up using `sigaction` to call
`preempt_handler` whenever `SIGVTALRM` is received. All this does is call
`uthread_yield`, forcing the current running thread to involuntarily yield
execution.

Enabling and disabling preemption is done using process signal masks; 
Enabling calls `sigprocmask` with the `SIG_BLOCK` parameter to indicate that
`SIGVTALRM` is to be added to the block mask.
Disabling does the same but with `SIG_UNBLOCK` to indicate that `SIGVTALRM` is
to be removed from the block mask.

Preemption Enabling/Disabling is used in some threading operations as mentioned above to make sure
sensitive segments are atomic.

Upon stopping preemption the timer is set back to default by setting the
interval and initial delay to 0. The handler is set back to default using
`SIG_DFL` as the handling function.
### Testing
Preemption is tested using a simple test case in `test_preempt.c`.
Thread 1 is started, creates thread 2, and begins a lengthy calculation that essentially spin
delays. Without preemption, this would block Thread 2 for some time. However,
with Preemption, we should see Thread 2 run and finish its short task before
Thread 1 finishes.

Without Preemption:
T1 Starts->T1 Finishes->T2 Starts->T2 Finishes

With Preemption
T1 Starts->Preemption->T2 Starts->T2 Finishes (usually)->T1 Finishes
## Makefile

The makefile for this project was much more advanced than the last project, as
we had many more files and compilation complexity, but we also had many more
tools at our disposal. To prevent repeating long strings of arguments, variables
were used for the targets, objects, and several command flags. We also use echo
commands for clean output, and a conditional variable for if the user wants more
detailed output. We also implement proper dependency tracking to ensure that
dependency files are only regenerated if needed.

## References/Sources
https://www.gnu.org/software/libc/manual/2.36/html_mono/libc.html#Blocking-Signals
https://www.gnu.org/software/libc/manual/2.36/html_mono/libc.html#Handler-Returns
https://www.gnu.org/software/libc/manual/2.36/html_mono/libc.html#Setting-an-Alarm
https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html
https://tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html
https://stackoverflow.com/questions/9302464/how-do-i-remove-a-signal-handler
https://man7.org/linux/man-pages/man2/sigaction.2.html
https://man7.org/linux/man-pages/man2/sigprocmask.2.html

