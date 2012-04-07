#include <stdio.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#include <errno.h>
#include <string.h>
#include <time.h>

#include "show_address.h"

#include "udp6_shutdown.h"

int main(int argc, char *argv[]) {

  int len;
  int r;
  int s;

  unsigned char *ipv6_address_string = argc>1 ? argv[1] : NULL;

  unsigned char ipv6_address[16];

  struct timespec ts;

  int retval;

  struct addrinfo hints;

  struct addrinfo *res;

  if (ipv6_address_string!=NULL) {

    struct addrinfo *rp;

    hints.ai_flags = 0;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 17;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    retval = getaddrinfo(ipv6_address_string, NULL, &hints, &res);
    
    if (retval != 0) {
      fprintf(stderr, "%s: Trouble with call to getaddrinfo.\n", __FUNCTION__);
      fprintf(stderr, "%s: gai_strerror = %s\n", __FUNCTION__, gai_strerror(retval));
      return -1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {

      memcpy(ipv6_address, ((char*) rp->ai_addr) + 8, sizeof(ipv6_address));

      break;

    }

  }
  else {
    fprintf(stderr, "%s: Please specify an ipv6 address.\n", __FUNCTION__);
    return -1;
  }

  s = socket(AF_INET6,SOCK_DGRAM,0);
  if (s == -1) {
    perror("socket");
    return -1;
  }

  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&retval, sizeof(retval)) < 0) {
    perror("setsockopt");
    return -1;
  }

  retval = udp6_shutdown(s, ipv6_address);

  if (retval != 0) {
    fprintf(stderr, "%s: Trouble with call to udp6_shutdown.\n", __FUNCTION__);
    return -1;
  }

  close(s);

  return 0;

}
