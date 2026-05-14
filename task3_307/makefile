#Makefile template - From PG3401_Lecture_04_Pointers S.65

INCLDIR = ./include
CC = gcc
CFLAGS = -ansi -pedantic -O2 -Wall -Wextra
CFLAGS += -I$(INCLDIR)

OBJDIR = obj 

_DEPS = task3.h
DEPS = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = task3_main.o task3_list.o 
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

all: task3_grading 

$(OBJDIR)/task3_main.o: task3_main.c $(DEPS)
	$(CC) -c -o obj/task3_main.o task3_main.c $(CFLAGS) 

$(OBJDIR)/task3_list.o: task3_list.c $(DEPS)
	$(CC) -c -o obj/task3_list.o task3_list.c $(CFLAGS) 
	
task3_grading: $(OBJS)
	gcc -o task3_grading obj/task3_main.o obj/task3_list.o $(CFLAGS)
	
.PHONY: clean 
clean: 
	rm -f $(OBJDIR)/*.o*~ core $(INCLDIR)/*~
