
#include <stdio.h>

int show_address(unsigned char *ipv6_address) {

  int count = 7;

  while(count>0) {
    printf("%02x%02x:", ipv6_address[0], ipv6_address[1]);
    ipv6_address+=2;
    count--;
  }

  printf("%02x%02x\n", ipv6_address[0], ipv6_address[1]);

  return 0;

}
