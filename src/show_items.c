
#include <stdio.h>
#include <assert.h>

#include "clusterboot.h"

int show_items(clusterboot_t *items, int num_items) {

  cluster_node_t *node;
  cluster_time_t *time;

  assert(items!=NULL);

  while(num_items>0) {

    node = items->node;
    time = items->time;

    printf("%x:%x:%x:%x:%x:%x %lu %lu\n", node->mac_address, node->mac_address+1, node->mac_address+2, node->mac_address+3, node->mac_address+4, node->mac_address+5, time->startup.tv_sec, time->shutdown.tv_sec);

    items++;
    num_items--;

  }

}
