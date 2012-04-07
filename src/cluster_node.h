#ifndef CLUSTER_NODE_H
#define CLUSTER_NODE_H

typedef struct {
  unsigned char mac_address[6];
  unsigned char ipv6[16];
} cluster_node_t;

#endif
