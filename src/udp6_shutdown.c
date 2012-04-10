#include <stdio.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <string.h>
#include <time.h>

#include "show_address.h"

int udp6_shutdown(int s, unsigned char *ipv6) {

  int len;
  int r;

  unsigned char packet[256];
  
  struct sockaddr_in6 sa6;

  unsigned short destination_port = 6496;

  unsigned short ns_destination_port;

  int sendto_retval;

  struct timespec ts;

  u_int64_t cmd_poweroff = 0x4000;

  memset(&sa6,0,sizeof(sa6));
  memcpy(sa6.sin6_addr.s6_addr, ipv6, 16);

  ns_destination_port = htons(destination_port);
  memcpy(&sa6.sin6_port, &ns_destination_port, sizeof(unsigned short));
  sa6.sin6_family = AF_INET6;

  len = sizeof(sa6);
  
  clock_gettime(CLOCK_REALTIME, &ts);

  {
  
    struct timespec ts_be64;

    u_int64_t cmd_be64;

    ts_be64.tv_sec = htobe32(ts.tv_sec);
    ts_be64.tv_nsec = htobe32(ts.tv_nsec);

    cmd_be64 = htobe64(cmd_poweroff);

    memcpy(packet, &ts_be64, sizeof(struct timespec));

    memcpy(packet + sizeof(struct timespec), &cmd_be64, sizeof(u_int64_t));

  }

  r = sizeof(struct timespec) + sizeof(u_int64_t);

  show_address(ipv6);

  sendto_retval = sendto(s,packet,r,0,(const struct sockaddr *) &sa6,len);
  if (sendto_retval==-1) {
    perror("sendto");
    return -1;
  }

  return 0;

}
