#include "tun_alloc.h"

int tun_alloc(char *dev)
{
  struct ifreq ifr;
  int tun_fd, err;

  if( (tun_fd = open("/dev/net/tun", O_RDWR)) < 0 )
  {
    perror("open /dev/net/tun: ");
    return -1;
  }

  memset(&ifr, 0, sizeof(ifr));

/* Flags: IFF_TUN   - TUN device (no Ethernet headers) 
 *        IFF_TAP   - TAP device  
 *
 *        IFF_NO_PI - Do not provide packet information  ???
 */ 
  
  ifr.ifr_flags = IFF_TUN|IFF_NO_PI; 
  if( *dev )
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  if( (err = ioctl(tun_fd, TUNSETIFF, (void *) &ifr)) < 0 )
  {
    perror("enabling TUNSETIFF");
    close(tun_fd);
    return err;
  }
  strcpy(dev, ifr.ifr_name);

  if(ioctl(tun_fd, TUNSETPERSIST, 1) < 0)
  {
    perror("enabling TUNSETPERSIST");
    return -1;
  }

  return tun_fd;
} 

int tun_del(char *dev)
{
  struct ifreq ifr;
  int tun_fd, err;

  if( (tun_fd = open("/dev/net/tun", O_RDWR)) < 0 )
    return -1;

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = IFF_TUN; 
  if( *dev )
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  if( (err = ioctl(tun_fd, TUNSETIFF, (void *) &ifr)) < 0 )
  {
    perror("enabling TUNSETIFF");
    close(tun_fd);
    return err;
  }
  strcpy(dev, ifr.ifr_name);

  if(ioctl(tun_fd, TUNSETPERSIST, 0) < 0)
  {
    perror("disabling TUNSETPERSIST");
    return -1;
  }

  return tun_fd;
} 
