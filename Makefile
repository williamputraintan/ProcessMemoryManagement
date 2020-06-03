CC=gcc
CFLAGS= -Wall -g -I. -lm
DEPS = constant.h inputFunc.h queue.h processingFunc.h helperProcessingFunc.h
OBJ = scheduler.o inputFunc.o queue.o processingFunc.o helperProcessingFunc.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

scheduler: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	$(RM) scheduler.o
