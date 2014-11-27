#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdarg.h>

#include <socket.h>
#include <ip4.h>
#include <str.h>

#define u16 uint16_t
#define u8 uint8_t
#define u64 uint64_t

#include "dbg.h"
#include "tun_alloc.h"
#include "nic.h"
#include "ezrohc.h"

#define VERSION "v0.1"

#define PORT 55555

#define IP_HDR_LEN 20
#define ETH_HDR_LEN 14
#define ARP_PKT_LEN 28

#define M UINT64_C(1000000)

int debug;
char *progname;
char *remote_ip = NULL;
char *eth_name = NULL, *tun_name = NULL;
char local_ip[16];
unsigned short int port = PORT;
u16 rport = PORT;

/* ex main */
int tun, net;
int net_mtu, tun_mtu;
u8 *buf, *pkt_tun, *pkt_net;
u8 *decomp_net, *decomp_tun;
char raddr[4], laddr[4];
ezrohc_h *h;

/* TCM-TF mechanic */
#define LXT_LIM 64
#define LXT 0x40
#define PFF 0x80
#define MUX_LEN 1
u64 current_us, last_us, diff_us;
u16 size_mux_pkt = 0, p_size_mux_pkt = 0;
u16 pkts = 0;
#define MAX_PKTS 100
#define MAX_TIME UINT64_C(10000000)
#define MAX_SIZE 1400
u16 lim_pkts = 0;
u64 lim_time = 0;
u64 lim_size = 0;
u64 lim_peri = 0;
struct timeval tv;

void u(void);

void
dbg (const char *msg, ...)
{
  if (debug)
  {
    va_list argp;
    fprintf (stderr, "DBG: ");
    va_start (argp, msg);
    vfprintf (stderr, msg, argp);
    va_end (argp);
    fprintf (stderr, "\n");
  }
}

void
eu (const char *err, ...)
{
  va_list argp;

  va_start (argp, err);
  vfprintf (stderr, err, argp);
  va_end (argp);
  puts ("");
  u ();
}

int
tun_read (int fd, u8 *buf, int n)
{
  int nread;

  if ((nread = read (fd, (char *)buf, n)) < 0)
    e ("tun read()");
  return nread;
}

int
tun_write (int fd, u8 *buf, int n)
{
  /* XXX should check if all the pkt has been sent inside tun_write */
  int r;

  if ((r = write (fd, (char *)buf, n)) < 0)
    e ("tun_write()");
  return r;
}

u64
ts_us (void)
{
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return tv.tv_sec * M + tv.tv_usec;
}


void
u(void)
{
  fprintf (stderr, "Usage:\n");
  fprintf (stderr,
	   "%s -i <tun name> -e <if name> -c <peerIP> [-p <port>] [-D] ",
	   progname);
  fprintf (stderr, "[-n #packets] [-s size] [-t nsec]\n");
  exit (1);
}

void
process_opt (int argc, char **argv)
{
  int option;

  while ((option = getopt (argc, argv, "Dhi:e:c:p:n:s:t:")) > 0)
  {
    switch (option)
    {
    case 'D':
      debug = 1;
      break;
    case 'h':
      u ();
      break;
    case 'i':
      tun_name = optarg;
      break;
    case 'e':
      eth_name = optarg;
      break;
    case 'c':
      remote_ip = optarg;
      break;
    case 'p':
      port = atoi (optarg);
      break;
    case 'n':
      lim_pkts = atoi (optarg);
      break;
    case 's':
      lim_size = atoi (optarg);
      break;
    case 't':
      lim_time = atoi (optarg);
      lim_time *= 1000;
      break;
    default:
      eu("\n");
    }
  }

  argv += optind;
  argc -= optind;

  if (argc > 0) eu ("unknown argument %s\n", argv[0]);
  if (!lim_pkts || lim_pkts > MAX_PKTS) lim_pkts = MAX_PKTS;
  if (!lim_size || lim_size > MAX_SIZE) lim_size = MAX_SIZE;
  if (!lim_time || lim_time > MAX_TIME) lim_time = MAX_TIME;
  if (!lim_peri || lim_peri > MAX_TIME) lim_peri = MAX_TIME;
  tv.tv_sec = 0;
  tv.tv_usec = lim_peri;
  if (remote_ip == NULL) eu ("remote IP not specified");
  if (tun_name == NULL) eu ("tun interface not specified");
  if (eth_name == NULL) eu ("eth interface not specified");

  return;
}

void
net_send (void)
{
  int r=0, s=0, i=0;
  
  while(s < size_mux_pkt)
  {
    r = socket_send4 (net, (char *)buf, size_mux_pkt, raddr, rport);
    if(r == -1) e ("net_send()");
    s += r;
    i++;
  }
  dbg ("net_send(): %d bytes sent in %d iteration", s, i);
  size_mux_pkt = 0;
  pkts = 0;
  last_us = ts_us ();
}

u16
decode_len (u8 *buf)
{
  u16 len;

  len = buf[0];
  if (buf[0] & LXT)
  {
    len = len ^ LXT;
    len = (len << 8) + (u8) buf[1];
  }
  dbg ("decode_len() 0x%x %x -> %d", (u8) buf[0], (u8) buf[1], len);
  return len;
}

void
proc_tun_pkt (void)
{
  u32 r;
  u16 mux_len = 1;

  r = tun_read (tun, decomp_tun, net_mtu);
  r = ezrohc_comp(h, decomp_tun, pkt_tun, r);
  
  p_size_mux_pkt = size_mux_pkt + r + mux_len;
  if (r >= LXT_LIM) p_size_mux_pkt++;
  if (p_size_mux_pkt > net_mtu) net_send ();
  pkts++;
  if (r < LXT_LIM) buf[size_mux_pkt] = r;
  else
  {
    buf[size_mux_pkt] = (r >> 8) + LXT;
    buf[size_mux_pkt + 1] = r % 256;
    mux_len++;
    decode_len (buf + size_mux_pkt);
  }
  memcpy (buf + size_mux_pkt + mux_len, pkt_tun, r);
  size_mux_pkt = p_size_mux_pkt;

  current_us = ts_us ();
  diff_us = current_us - last_us;

  if (pkts >= lim_pkts || size_mux_pkt >= lim_size || diff_us >= lim_time)
  {
    dbg("sending %d/%d pkts\n", pkts, lim_pkts);
    dbg("sending %u/%llu size\n", size_mux_pkt, lim_size);
    dbg("sending %llu/%llu time\n", diff_us, lim_time);
    dbg("sending %llu %llu last\n", current_us, last_us);
    net_send ();
  }
}

void
proc_net_pkt (void)
{
  u32 r;
  char ip[4];
  u16 port;
  u16 pkt_len = 0, decomp_len = 0;
  off_t offset;

  r = socket_recv4 (net, (char *)pkt_net, net_mtu, ip, &port);
  if (r == -1)
    e ("proc_net_pkt");

  offset = 0;
  while (offset < r)
  {
    if (pkt_net[offset] & PFF)
    {
      offset = r;
      dbg ("proc_net_pkt() BAD PFF");
    }
    else
    {
      if ((pkt_len = decode_len (pkt_net + offset)) >= LXT_LIM)
	offset++;
      offset++;
      decomp_len = ezrohc_decomp(h, pkt_net+offset, decomp_net, pkt_len);
      tun_write (tun, decomp_net, decomp_len);
      offset += pkt_len;
    }
  }
}

void
period_expired (void)
{
  if (pkts > 0) net_send ();
}

int
main (int argc, char **argv)
{

  int maxfd;
  int r;
  fd_set rd_set;

  progname = argv[0];
  process_opt (argc, argv);

  if ((tun = tun_alloc (tun_name)) < 0) e("tun_alloc()");
  if ((net = socket_udp4 ()) < 0) e("socket()");
  if (scan_ip4 (remote_ip, raddr) != str_len (remote_ip)) e("IP parse error");
  if (nic_if (net, eth_name) < 0) e("failed to find interface");
  if (nic_ip4 (net, eth_name, laddr) < 0) e("failed to find IP address");
  if (!(net_mtu = nic_mtu (net, eth_name))) e("failed to get eth MTU");
  if (!(tun_mtu = nic_mtu (net, tun_name))) e("failed to get tun MTU");

  buf = malloc (net_mtu);
  decomp_tun = malloc (tun_mtu);
  pkt_tun = malloc (tun_mtu);
  pkt_net = malloc (net_mtu);
  decomp_net = malloc (net_mtu);
  h = ezrohc_init(net_mtu);

  if (scan_ip4 (remote_ip, raddr) != str_len (remote_ip)) e("IP parse error");
  if (socket_bind4 (net, laddr, port) < 0) e("bind()");

  maxfd = (tun > net) ? tun : net;

  /* XXX setting period */

  dbg ("%s %s", progname, VERSION);
  while (1)
  {
    FD_ZERO (&rd_set);
    FD_SET (tun, &rd_set);
    FD_SET (net, &rd_set);
    r = select (maxfd + 1, &rd_set, NULL, NULL, &tv);

    if (r < 0 && errno == EINTR)
      continue;
    if (r < 0)
      e ("select()");

    if (FD_ISSET (tun, &rd_set)) proc_tun_pkt ();
    else if (FD_ISSET (net, &rd_set)) proc_net_pkt ();
    else period_expired ();
  }
  free (buf);
  return (0);
}
