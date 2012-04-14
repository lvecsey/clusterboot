#include <stdio.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <string.h>
#include <time.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <linux/reboot.h>

#include "poweroff_proc.h"

int main(int argc, char *argv[]) {

  int len;
  int r;

  unsigned char packet[256];
  struct sockaddr_in6 sa6;
  int s;

  unsigned short listen_port = 6496;

  unsigned short ns_listen_port;

  struct timespec receive_ts_be64, receive_ts, ts;

  u_int64_t cmd_poweroff = 0x4000;

  u_int64_t cmd;

  u_int64_t cmd_be64;

  int retval;

  s = socket(AF_INET6,SOCK_DGRAM,0);
  if (s == -1) {
    perror("socket");
    return -1;
  }

  memset(&sa6,0,sizeof(sa6));
  ns_listen_port = htons(listen_port);
  memcpy(&sa6.sin6_port, &ns_listen_port, sizeof(unsigned short));
  sa6.sin6_family = AF_INET6;

  if (bind(s,(struct sockaddr *) &sa6,sizeof sa6) == -1) {
    perror("bind");
    return -1;
  }

  if (system(NULL) == 0) {
    printf("%s: Warning; system shell not available.\n", __FUNCTION__);
  }

  for (;;) {
    len = sizeof(sa6);
    r = recvfrom(s,packet,sizeof packet,0,(struct sockaddr *) &sa6,&len);
    if (r >= sizeof(u_int64_t)) {

      memcpy(&cmd_be64, packet, sizeof(u_int64_t));

      if (r >= sizeof(u_int64_t) + sizeof(struct timespec)) {

	memcpy(&receive_ts_be64, packet + sizeof(u_int64_t), sizeof(struct timespec));
	receive_ts.tv_sec = be32toh(receive_ts_be64.tv_sec);
	receive_ts.tv_nsec = be32toh(receive_ts_be64.tv_nsec);

      }

      cmd = be64toh(cmd_be64);

      clock_gettime(CLOCK_REALTIME, &ts);

      printf("%s: Received packet, length=%d\n", __FUNCTION__, r);

      if (cmd == cmd_poweroff) {

	retval = system("./system-shutdown.sh");

	usleep(1000000);

	retval = reboot(LINUX_REBOOT_CMD_POWER_OFF);

	if (retval == -1) {
	  perror("reboot");
	  poweroff_proc();
	  return -1;
	}

	poweroff_proc();

      }

    }

  }

  return 0;

}
