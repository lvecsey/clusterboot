#include <stdio.h>
#include <assert.h>

#include <stdlib.h>

int poweroff_proc() {

  int retval;

  retval = system("echo o > /proc/sysrq-trigger");

  if (retval != 0) {
    fprintf(stderr, "%s: Trouble with call to system for echo statement.\n", __FUNCTION__);
    return -1;
  }

  return 0;

}
