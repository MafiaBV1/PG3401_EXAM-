#Makefile template - From PG3401_Lecture_04_Ponters S.65

INCLDIR = ./include
CC = gcc
CFLAGS = -O2 -include stddef.h 
CFLAGS += -I$(INCLDIR)

OBJDIR = obj

_DEPS = task4_prime.h
DEPS = $(parsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = task4_threads.o task4_prime.o xtea.o ipc.o 
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

all: task4_threads

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

task4_threads: $(OBJS)
	gcc -o $@ $^ $(CFLAGS) -lpthread

.PHONY: clean
clean:
	rm -f $(OBJDIR/*.o *~ core $(INCLDIR)/*~
