
#include <stdio.h>
#include <assert.h>

#include <unistd.h>

#include <time.h>

#include "clusterboot.h"

#include "udp6_shutdown.h"

int send_etherwake(char *interface, unsigned char *mac_address) {

  char string[50];

  int retval;

  sprintf(string, "etherwake -i %s %x:%x:%x:%x:%x:%x", interface, mac_address, mac_address+1, mac_address+2, mac_address+3, mac_address+4, mac_address+5);

  retval = system(string);

  if (retval!=0) {
    fprintf(stderr, "%s: Tried to startup mac address and failed. retval=%d\n", __FUNCTION__, retval);
  }

  return retval;

}

int process(int s, char *interface, clusterboot_t *items, int num_entries) {

  int startup_count_down = num_entries;

  int shutdown_count_down = num_entries;

  useconds_t sleep_interval = 3000;

  struct timespec now;
    
  struct timespec *startup_time, *shutdown_time;

  clusterboot_t *fill = items, *end_fill = items + num_entries;

  cluster_node_t *node;

  clusterboot_t *startup_current = items, *shutdown_current = items;

  int retval;

  assert(interface!=NULL && items != NULL && num_entries>0);

  if (startup_current->time != NULL) {
    startup_time = &startup_current->time->startup;
  }

  if (shutdown_current->time != NULL) {
    shutdown_time = &shutdown_current->time->shutdown;
  }

  assert(startup_time!=NULL && shutdown_time!=NULL);

  for ( ; startup_count_down > 0 && shutdown_count_down > 0; ) {

    clock_gettime(CLOCK_MONOTONIC, &now);

    usleep(sleep_interval);

    if (startup_time->tv_sec > now.tv_sec && startup_count_down > 0) {

      cluster_node_t *n = startup_current->node;

      retval = send_etherwake(interface, n->mac_address);

      startup_count_down--;

      if (startup_count_down > 0) {

	startup_current++;
	startup_time = &startup_current->time->startup;

      }

    }

    if (shutdown_time->tv_sec > now.tv_sec && shutdown_count_down > 0) {

      cluster_node_t *shutdown_node = shutdown_current->node;

      assert(shutdown_node != NULL);

      udp6_shutdown(s, shutdown_node->ipv6);

      shutdown_count_down--;

      if (shutdown_count_down > 0) {
	shutdown_current++;
	shutdown_time = &shutdown_current->time->shutdown;
      }

    }

  }

  return 0;

}
