
#include <stdio.h>
#include <assert.h>

#include <unistd.h>
#include <stdlib.h>

#include <time.h>

#include "clusterboot.h"

#include "udp6_shutdown.h"

#include "send_etherwake.h"

int process(int s, char *interface, clusterboot_t *items, int num_entries) {

  int startup_count_down = num_entries;

  int shutdown_count_down = num_entries;

  useconds_t sleep_interval = 3000;

  struct timespec process_time;

  struct timespec now;
    
  struct timespec *startup_time, *shutdown_time;

  clusterboot_t *startup_current = items, *shutdown_current = items;

  int retval;

  assert(interface!=NULL && items != NULL && num_entries>0);

  if (startup_current->time == NULL || shutdown_current->time == NULL) {
    fprintf(stderr, "%s: Initial startup and shutdown times are nonexistent.\n", __FUNCTION__);
    return -1;
  }

  startup_time = &startup_current->time->startup;
  shutdown_time = &shutdown_current->time->shutdown;

  assert(startup_time!=NULL && shutdown_time!=NULL);

  clock_gettime(CLOCK_MONOTONIC, &process_time);

  process_time.tv_sec += 86400;

    do {

    clock_gettime(CLOCK_MONOTONIC, &now);

    usleep(sleep_interval);

    if (now.tv_sec > startup_time->tv_sec && startup_count_down > 0) {

      cluster_node_t *startup_node = startup_current->node;

      assert(startup_node != NULL);

      printf("%s: Sending etherwake startup to %s on interface %s.\n", __FUNCTION__, startup_node->mac_address, interface);

      retval = send_etherwake(interface, startup_node->mac_address);

      startup_count_down--;

      if (startup_count_down > 0) {

	startup_current++;
	startup_time = &startup_current->time->startup;

      }

    }

    if (now.tv_sec > shutdown_time->tv_sec && shutdown_count_down > 0) {

      cluster_node_t *shutdown_node = shutdown_current->node;

      assert(shutdown_node != NULL);

      printf("%s: Sending udp6_shutdown to ipv6 address ending in %x%x.\n", __FUNCTION__, shutdown_node->ipv6[14], shutdown_node->ipv6[15]);

      udp6_shutdown(s, shutdown_node->ipv6);

      shutdown_count_down--;

      if (shutdown_count_down > 0) {
	shutdown_current++;
	shutdown_time = &shutdown_current->time->shutdown;
      }

    }

  } while (process_time.tv_sec > now.tv_sec);

  return 0;

}

