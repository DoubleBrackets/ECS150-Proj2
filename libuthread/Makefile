# Target library
lib := libuthread.a

CC := gcc
CFLAGS := -Wall -Wextra # -Werror
CFLAGS += -g

all: $(lib)

## Create library from object files
$(lib): queue.o uthread.o context.o preempt.o sem.o
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(lib) queue.o uthread.o context.o preempt.o sem.o


