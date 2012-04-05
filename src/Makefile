
CC=gcc

all : clusterboot await-shutdown

clusterboot : CFLAGS+=-Wall -O2

clusterboot : CFLAGS+=-I$(HOME)/gsl-1.15/include

clusterboot : LDFLAGS=-L$(HOME)/gsl-1.15/lib

clusterboot : LIBS=-lrt -lgsl -lgslcblas

clusterboot : clusterboot.o clusterboot.h cluster_node.h cluster_time.h show_address.o udp6_shutdown.o show_address.h udp6_shutdown.h
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

await-shutdown : CFLAGS=-O2 -Wall

await-shutdown : LIBS=-lrt

await-shutdown : await-shutdown.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
