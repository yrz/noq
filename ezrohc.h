#ifndef _EZROHC
#define _EZROHC

#include <rohc/rohc.h>
#include <rohc/rohc_comp.h>
#include <rohc/rohc_decomp.h>

typedef 
struct ezrohc_s
{
  int s;
  struct rohc_comp *c; /* compressor */
  struct rohc_decomp *d; /* decompressor */
  struct rohc_buf in;
  struct rohc_buf *out;
} ezrohc_h;

ezrohc_h *ezrohc_init();
int ezrohc_comp(ezrohc_h *h, uint8_t *in, uint8_t *out, int len_in);
int ezrohc_decomp(ezrohc_h *h, uint8_t *in, uint8_t *out, int len_in);
int ezrohc_free();

#endif
