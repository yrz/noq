#include "nic.h"

int nic_if(int fd, const char *ifname)
{
  struct ifreq ifr;

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
  return ioctl(fd, SIOCGIFINDEX, &ifr);
}

int nic_ip4(int fd, const char *ifname, char ip[4])
{
  struct ifreq ifr;

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
  if(ioctl(fd, SIOCGIFADDR, &ifr) < 0)
    return -1;
  else
    *(u32*)ip = *(u32 *)&(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr);

  return 0;
}

int nic_mtu(int fd, const char *ifname)
{
  struct ifreq ifr;

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
  if(ioctl(fd, SIOCGIFMTU, &ifr) < 0)
    return -1;
  else
    return ifr.ifr_mtu;
}
