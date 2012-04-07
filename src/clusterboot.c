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

#include <sys/socket.h>

#include <netdb.h>
#include <errno.h>

#include <string.h>

#include "udp6_shutdown.h"

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

void clean_end(unsigned char *s) {

  while (*s) {
    if (*s == '\r') { *s = 0; break; }
    if (*s == '\n') { *s = 0; break; }
    s++;
  }

}
 
int main(int argc, char *argv[]) {

  char *line = NULL;
  size_t len = 0;
  ssize_t read_bytes;

  long int num_entries = argc>1 ? strtol(argv[1], NULL, 10) : 50;

  clusterboot_t *items = malloc(sizeof(clusterboot_t) * num_entries);

  clusterboot_t *fill = items, *end_fill = items + num_entries;

  cluster_node_t *node;

  clusterboot_t *startup_current = items, *shutdown_current = items;

  char *env_CLUSTERBOOT_INTERFACE = getenv("CLUSTERBOOT_INTERFACE");

  char *interface = env_CLUSTERBOOT_INTERFACE != NULL ? env_CLUSTERBOOT_INTERFACE : "eth2";

  char *ipv6_address_string;

  gsl_rng *r;

  int s;

  int retval;

  struct addrinfo hints;

  struct addrinfo *res;

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

  s = socket(AF_INET6, SOCK_DGRAM, 0);
  if (s==-1) {
    perror("socket");
    fprintf(stderr, "%s: Trouble creating ipv6 udp socket.\n", __FUNCTION__);
    return -1;
  }

  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&retval, sizeof(retval)) < 0) {
    perror("setsockopt");
    return -1;
  }


  while ((read_bytes = getline(&line, &len, stdin)) != -1) {
    
    if (len>0) {

      ipv6_address_string = strchr(line, ' ');

      node = malloc(sizeof(cluster_node_t));
      if (node==NULL) {
	fprintf(stderr, "%s: Trouble allocating cluster_node.\n", __FUNCTION__);
	return -1;
      }

      if (ipv6_address_string!=NULL) {

	ipv6_address_string++;

	struct addrinfo *rp;

	hints.ai_flags = 0;
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 17;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;

	clean_end(ipv6_address_string);

	printf("%s: Reading ipv6_address_string=%s\n", __FUNCTION__, ipv6_address_string);

	retval = getaddrinfo(ipv6_address_string, NULL, &hints, &res);
    
	if (retval != 0) {
	  fprintf(stderr, "%s: Trouble with call to getaddrinfo.\n", __FUNCTION__);
	  fprintf(stderr, "%s: gai_strerror = %s\n", __FUNCTION__, gai_strerror(retval));
	  return -1;
	}

	for (rp = res; rp != NULL; rp = rp->ai_next) {

	  memcpy(node->ipv6, ((char*) rp->ai_addr) + 8, sizeof(node->ipv6));

	  break;

	}

      }

      sscanf(line, "%x:%x:%x:%x:%x:%x", node->mac_address, node->mac_address+1, node->mac_address+2, node->mac_address+3, node->mac_address+4, node->mac_address+5);

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

      fill->time = ct;

      fill++;

    }

  }

  qsort(items, num_entries, sizeof(clusterboot_t), compare_time);

  {

    int startup_count_down = num_entries;

    int shutdown_count_down = num_entries;

    useconds_t sleep_interval = 3000;

    struct timespec now;
    
    struct timespec *startup_time, *shutdown_time;

    char string[50];

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

	sprintf(string, "etherwake -i %s %x:%x:%x:%x:%x:%x", interface, n->mac_address, n->mac_address+1, n->mac_address+2, n->mac_address+3, n->mac_address+4, n->mac_address+5);

	retval = system(string);

	if (retval!=0) {
	  fprintf(stderr, "%s: Tried to startup mac address and failed. retval=%d\n", __FUNCTION__, retval);
	}

	startup_current++;
	startup_count_down--;
	startup_time = startup_current->time;

      }

      if (shutdown_time->tv_sec > now.tv_sec && shutdown_count_down > 0) {

	cluster_node_t *shutdown_node = shutdown_current->node;

	assert(shutdown_node != NULL);

	sscanf(line, "%x:%x:%x:%x:%x:%x %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x", node->mac_address, node->mac_address+1, node->mac_address+2, node->mac_address+3, node->mac_address+4, node->mac_address+5, node->ipv6, node->ipv6+1, node->ipv6+2, node->ipv6+3, node->ipv6+4, node->ipv6+5, node->ipv6+6, node->ipv6+7, node->ipv6+8, node->ipv6+9, node->ipv6+10, node->ipv6+11, node->ipv6+12, node->ipv6+13, node->ipv6+14, node->ipv6+15);

	fill++;
	if (fill == end_fill) {
	  break;
	}
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

      fill->time = ct;

      fill++;

    }

  }

  qsort(items, num_entries, sizeof(clusterboot_t), compare_time);

  {

    int startup_count_down = num_entries;

    int shutdown_count_down = num_entries;

    useconds_t sleep_interval = 3000;

    struct timespec now;
    
    struct timespec *startup_time, *shutdown_time;

    char string[50];

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

	sprintf(string, "etherwake -i %s %x:%x:%x:%x:%x:%x", interface, n->mac_address, n->mac_address+1, n->mac_address+2, n->mac_address+3, n->mac_address+4, n->mac_address+5);

	retval = system(string);

	if (retval!=0) {
	  fprintf(stderr, "%s: Tried to startup mac address and failed. retval=%d\n", __FUNCTION__, retval);
	}

	startup_current++;
	startup_count_down--;
	startup_time = &startup_current->time->startup;

      }

      if (shutdown_time->tv_sec > now.tv_sec && shutdown_count_down > 0) {

	cluster_node_t *shutdown_node = shutdown_current->node;

	assert(shutdown_node != NULL);

	udp6_shutdown(s, shutdown_node->ipv6);

	shutdown_current++;
	shutdown_count_down--;
	shutdown_time = &shutdown_current->time->shutdown;

      }

    }

  }

  close(s);

  return 0;

}
