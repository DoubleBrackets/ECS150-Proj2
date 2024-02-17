# Target library
lib := libuthread.a

#Object library
objs := queue.o uthread.o context.o preempt.o sem.o

CC := gcc
CFLAGS := -Wall -Wextra -Werror -MMD
CFLAGS += -g

ifneq ($(V), 1)
Q = @
endif

all: $(lib)

# Dependency tracking
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

## Create library from object files
$(lib): $(objs)
	@echo "AR RCS $@"
	$(Q)ar rcs $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "CLEAN"
	$(Q)rm -f $(lib) $(objs) $(deps)


