
clusterboot : CFLAGS=-Wall -O2

clusterboot : LIBS=-lrt

clusterboot : clusterboot.o clusterboot.h cluster_node.h cluster_time.h
	$(CC) -o $@ $^ $(LIBS)

