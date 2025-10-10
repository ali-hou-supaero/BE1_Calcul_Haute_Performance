CC=gcc
CFLAGS=-Wall -Werror -std=gnu99 -fopenmp -Iinclude
LDFLAGS=-lrt -lm -fopenmp -lz
CFLAGS_OMP=
.PHONY: all clean

all: kmeans compress

kmeans : kmeans.o main.o
	$(CC) -o $@ $^ $(LDFLAGS)

compress: spng.o kmeans.o compress.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o : src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean :
	rm -f spng.o kmeans.o main.o kmeans compress.o compress
