#ifndef CLUSTER_TIME_H
#define CLUSTER_TIME_H

#include <time.h>

typedef struct {
  struct timespec startup;
  struct timespec shutdown;
} cluster_time_t;

#endif
