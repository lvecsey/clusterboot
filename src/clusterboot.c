#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

#include "cluster_node.h"
#include "cluster_time.h"

#include "clusterboot.h"

#include <gsl/gsl_rng.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int compare_time(const void *a, const void *b) {

  clusterboot_t *aboot1 = (clusterboot_t*) a, *bboot1 = (clusterboot_t*) b;

  cluster_time_t *atime1, *btime1;
  
  assert(aboot1->time != NULL && bboot1->time != NULL);

  atime1 = aboot1->time;
  btime1 = bboot1->time;

  if (atime1->startup.tv_sec < btime1->startup.tv_sec) return -1;

  if (atime1->startup.tv_sec > btime1->startup.tv_sec) return 1;

  assert(atime1->startup.tv_sec == btime1->startup.tv_sec);

  if (atime1->startup.tv_nsec < btime1->startup.tv_nsec) return -1;

  if (atime1->startup.tv_nsec > btime1->startup.tv_nsec) return 1;

  assert(atime1->startup.tv_nsec == btime1->startup.tv_nsec);

  return 0;

}

int main(int argc, char *argv[]) {

  char *line = NULL;
  size_t len = 0;
  ssize_t read_bytes;

  long int num_entries = argc>1 ? strtol(argv[1], NULL, 10) : 50;

  clusterboot_t *items = malloc(sizeof(clusterboot_t) * num_entries);

  clusterboot_t *fill = items, *end_fill = items + num_entries;

  cluster_node_t *node;

  gsl_rng *r;

  if (items==NULL) {
    fprintf(stderr, "%s: Trouble allocating %ld items.\n", __FUNCTION__, num_entries);
    return -1;
  }

  r = gsl_rng_alloc(gsl_rng_taus);
  if (r==NULL) {
    fprintf(stderr, "%s: Trouble allocating random number generator.\n", __FUNCTION__);
    return -1;
  }

  {

    int read_bytes;

    int urandom_fd = open("/dev/urandom", O_RDONLY);

    unsigned long int s;

    if (urandom_fd==-1) return -1;

    read_bytes = read(urandom_fd, &s, sizeof(unsigned long int));
    if (read_bytes != sizeof(unsigned long int)) return -1;

    gsl_rng_set(r, s);

    close(urandom_fd);

  }

  while ((read_bytes = getline(&line, &len, stdin)) != -1) {
    
    if (len>0) {
      node = malloc(sizeof(cluster_node_t));
      if (node==NULL) {
	fprintf(stderr, "%s: Trouble allocating cluster_node.\n", __FUNCTION__);
	return -1;
      }
      sscanf(line, "%x:%x:%x:%x:%x:%x %x%x:%x%x:%x%x:%x%x:%x%x:%x%x", node->mac_address, node->mac_address+1, node->mac_address+2, node->mac_address+3, node->mac_address+4, node->mac_address+5, node->ipv6, node->ipv6+1, node->ipv6+2, node->ipv6+3, node->ipv6+4, node->ipv6+5, node->ipv6+6, node->ipv6+7, node->ipv6+8, node->ipv6+9, node->ipv6+10, node->ipv6+11);
      fill++;
      if (fill == end_fill) {
	break;
      }
    }

  }

  if (fill < end_fill) {
    num_entries = (fill - items);
  }

  printf("%s: Read %ld entries.\n", __FUNCTION__, num_entries);

  {

    int time_fills = num_entries;

    cluster_time_t *ct;

    for (fill = items; time_fills>0; time_fills--) {

      ct = malloc(sizeof(cluster_time_t));
      if (ct==NULL) {
	fprintf(stderr, "%s: Trouble with malloc of cluster_time.\n", __FUNCTION__);
	return -1;
      }

      clock_gettime(CLOCK_MONOTONIC, &ct->startup);
      ct->startup.tv_sec += gsl_rng_uniform_int(r, 82800);

      ct->shutdown.tv_sec = ct->startup.tv_sec + 3600;
      ct->shutdown.tv_nsec = ct->startup.tv_nsec;

    }

  }

  qsort(items, num_entries, sizeof(clusterboot_t), compare_time);

  return 0;

}
