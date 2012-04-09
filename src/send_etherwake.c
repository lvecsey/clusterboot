#include <stdio.h>
#include <stdlib.h>

int send_etherwake(char *interface, unsigned char *mac_address) {

  char string[50];

  int retval;

  sprintf(string, "etherwake -i %s %x:%x:%x:%x:%x:%x", interface, mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

  retval = system(string);

  if (retval!=0) {
    fprintf(stderr, "%s: Tried to startup mac address and failed. retval=%d\n", __FUNCTION__, retval);
  }

  return retval;

}
