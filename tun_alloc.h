#ifndef TUNALLOC_H
#define TUNALLOC_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <linux/if_tun.h>
#include <net/if.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int tun_alloc(char *dev);
int tun_del(char *dev);

#endif
