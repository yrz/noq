#ifndef __YSOCKET_H
#define __YSOCKET_H

#include <stdlib.h>
#include "int.h"

size_t sck_send4 (int, const char *, size_t, const char ip[4], u16);
size_t sck_recv4 (int, char *, size_t, char ip[4], u16 *);
int sck_udp4 (void);
int sck_bind4 (int, char ip[4], u16);
int scan_ip4 (const char *, char ip[4]);

#endif
