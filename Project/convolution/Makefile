CC = gcc
CFLAGS = -Wall -O0 -march=native -fno-tree-vectorize -mfpmath=sse
LIBS = -lm

OBJS = bmplib.o perfenv.o main.o proj.o

all: bmpfilter

bmpfilter: $(OBJS) bmplib.h perfenv.h proj.h
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o bmpfilter

clean: 
	-rm -f $(OBJS) bmpfilter
	-rm out_*
