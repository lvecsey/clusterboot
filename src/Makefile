
CC=gcc

all : clusterboot await-shutdown send-shutdown

clusterboot : CFLAGS+=-Wall -O2

clusterboot : CFLAGS+=-I$(HOME)/gsl-1.15/include

clusterboot : LDFLAGS=-L$(HOME)/gsl-1.15/lib

clusterboot : LIBS=-lrt -lgsl -lgslcblas

clusterboot : clusterboot.o clusterboot.h cluster_node.h cluster_time.h show_address.o udp6_shutdown.o show_address.h udp6_shutdown.h show_items.o show_items.h process.o process.h send_etherwake.o send_etherwake.h
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

await-shutdown : CFLAGS=-O2 -Wall

await-shutdown : LIBS=-lrt

await-shutdown : await-shutdown.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

send-shutdown : CFLAGS=-O2 -Wall

send-shutdown : LIBS=-lrt

send-shutdown : send-shutdown.o udp6_shutdown.o show_address.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

