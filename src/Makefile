
CC=gcc

clusterboot : CFLAGS+=-Wall -O2

clusterboot : CFLAGS+=-I$(HOME)/gsl-1.15/include

clusterboot : LDFLAGS=-L$(HOME)/gsl-1.15/lib

clusterboot : LIBS=-lrt -lgsl -lgslcblas

clusterboot : clusterboot.o clusterboot.h cluster_node.h cluster_time.h
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

