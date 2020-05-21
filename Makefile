CC=gcc
CFLAGS= -Wall -g -I. -lm
# DEPS = scheduler.h
OBJ = scheduler.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

scheduler: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	$(RM) scheduler.o
