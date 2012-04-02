#ifndef CLUSTERBOOT_H
#define CLUSTERBOOT_H

#include "cluster_node.h"
#include "cluster_time.h"

typedef struct {
  cluster_node_t *node;
  cluster_time_t *time;
} clusterboot_t;

#endif
