INCLDIR = ./include
CC = gcc
CFLAGS = -O2
CFLAGS += -I$(INCLDIR)

OBJDIR = obj

_DEPS = ewpdef.h
DEPS = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = task6_main.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS)) 

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

task6: $(OBJS)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o*~ core $(INCLDIR)/*~
