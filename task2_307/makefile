#Makefile template - From PG3401_Lecture_04_Pointers S.65

INCLDIR = ./include
CC		= gcc
CFLAGS	= -O2
CFLAGS += -I$(INCLDIR)

OBJDIR = obj

# List all header files under 
_DEPS = task2.h	
DEPS = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

# List all sourcefiles under, but giv their '.o' file convention 
_OBJS = task2_main.o task2_fib.o task2_prim.o task2_kvad.o task2_cube.o \
		task2_perf.o task2_abun.o task2_def.o task2_odd.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))


#all: pgexam26_output

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	

# Change output filename under 
pgexam26_output: $(OBJS)
	gcc -o $@ $^ $(CFLAGS) -lm

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~
