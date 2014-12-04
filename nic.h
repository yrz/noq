#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#include "int.h"

int nic_if(int fd, const char *ifname);
int nic_mtu(int fd, const char *ifname);
int nic_ip4(int fd, const char *ifname, char ip[4]);
