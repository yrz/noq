#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "sck.h"

void 
u16_ub(const char *i, u16 *o)
{
  *o = ((u16)((u8)i[0])<<8)+((u8)i[1]);
}

void 
u16_pb(char *o, u16 i)
{
  o[0]=i>>8; o[1]=i&255;
}

void
bz(void *o, size_t l)
{
  register char *s=o;
  size_t i;
  for(i=0;i<l;++i) s[i]=0;
}

size_t 
sck_send4 (int s, const char *b, size_t l, const char ip[4], u16 p)
{
  struct sockaddr_in si;
  bz(&si, sizeof si);
  si.sin_family = AF_INET;
  u16_pb((char *) &si.sin_port, p);
  *((u32*)&si.sin_addr) = *((u32*)ip);
  return sendto(s,b,l,0,(void *)&si, sizeof si);
}

size_t 
sck_recv4 (int s, char *b, size_t l, char ip[4], u16 *p)
{
  struct sockaddr_in si;
  socklen_t len = sizeof si;
  size_t r;

  if ((r = recvfrom(s,b,l,0,(struct sockaddr *) &si,&len))<0) return -1; 
  if (ip) *(u32*)ip = *(u32*)&si.sin_addr;
  if (p) u16_ub((char *) &si.sin_port,p);
  return r;
}

int 
sck_udp4 (void)
{
  int s;
  s = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
  if(s == -1) return -1;
  return fcntl(s,F_SETFL,O_NONBLOCK);
}

int 
sck_bind4 (int s, char ip[4], u16 p)
{
  struct sockaddr_in si;
  bz(&si, sizeof si);
  si.sin_family = AF_INET;
  u16_pb((char *) &si.sin_port, p);
  if (ip) *(u32*)&si.sin_addr = *(u32*)ip;
  else si.sin_addr.s_addr = INADDR_ANY;
  return bind(s,(const struct sockaddr *)&si,sizeof si);
}

unsigned int
_scan_un(const char *s, unsigned char*d)
{
  const char *t=s;
  register unsigned char n=0;
  unsigned char c;
  
  for(;(c=*t-'0')<10; ++t) n+=((n*10)+c);

  if(t-s) *d=n;
  return t-s;
}

int 
scan_ip4 (const char *s, char ip[4])
{
  u32 l = 0;
  u8 u;
  int i;
  
  for(i=0;i<4;++i)
  {
    int j;
    l+=(j=_scan_un(s,&u))+1;
    if(!j) return 0;
    ip[i]=u; s+=j;
    if(i<3 && *s!='.') return 0; ++s;
  }
  return l-1;
}
