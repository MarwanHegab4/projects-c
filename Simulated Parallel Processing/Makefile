CC = gcc
CFLAGS = -Wall -g

EXEC = main core
SRCS = main.c core.c
HEADERS = core.h signals.h

OBJS_MAIN = main.o
OBJS_CORE = core.o

# object files
OBJS = $(OBJS_MAIN) $(OBJS_CORE)

all: $(EXEC)

main: $(OBJS_MAIN)
	$(CC) $(CFLAGS) -o main $(OBJS_MAIN)

core: $(OBJS_CORE)
	$(CC) $(CFLAGS) -o core $(OBJS_CORE)
	chmod +x core

main.o: main.c $(HEADERS)
	$(CC) $(CFLAGS) -c main.c

core.o: core.c $(HEADERS)
	$(CC) $(CFLAGS) -c core.c

# New target for creating submission.zip
submission: clean
	zip submission.zip $(SRCS) $(HEADERS) Makefile

clean:
	rm -f *.o $(EXEC) submission.zip

.PHONY: all clean submission

