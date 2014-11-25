#include "ezrohc.h"
#include "dbg.h"
#include <stdarg.h>
#include <stdio.h>

const struct rohc_ts 
_time = { .sec = 0, .nsec = 0 };

static void
_print_rohc_traces (void *const priv_ctxt,
		    const rohc_trace_level_t level,
		    const rohc_trace_entity_t entity,
		    const int profile, const char *const format, ...);

static int
_rohc_rand(const struct rohc_comp *const c, void *const user_ctx)
{
  return rand ();
}

ezrohc_h *
ezrohc_init (size_t buf_size)
{
  ezrohc_h *h;
  h = malloc (sizeof (ezrohc_h));
  if(h == NULL) e("malloc ezrohc_h Out of memory");
  h->max_len = buf_size;

  h->c = rohc_comp_new2 (ROHC_SMALL_CID, ROHC_SMALL_CID_MAX, _rohc_rand, NULL);
  if(h->c == NULL) e("");
  h->s = rohc_comp_set_traces_cb2(h->c, _print_rohc_traces, NULL);
  h->s = rohc_comp_enable_profiles (h->c,
				    ROHC_PROFILE_UNCOMPRESSED,
				    ROHC_PROFILE_IP,
				    ROHC_PROFILE_TCP,
				    ROHC_PROFILE_UDP,
				    ROHC_PROFILE_ESP,
				    ROHC_PROFILE_RTP,
				    ROHC_PROFILE_UDPLITE, -1);
  if(!(h->s)) e("rohc_comp_enable_profiles:");

  h->d = rohc_decomp_new2(ROHC_SMALL_CID, ROHC_SMALL_CID_MAX, ROHC_O_MODE);
  if(h->d == NULL) e("");
  h->s = rohc_decomp_set_traces_cb2(h->d, _print_rohc_traces, NULL);
  h->s = rohc_decomp_enable_profiles (h->d,
				      ROHC_PROFILE_UNCOMPRESSED,
				      ROHC_PROFILE_IP,
				      ROHC_PROFILE_TCP,
				      ROHC_PROFILE_UDP,
				      ROHC_PROFILE_ESP,
				      ROHC_PROFILE_RTP,
				      ROHC_PROFILE_UDPLITE, -1);
  if(!(h->s)) e("rohc_decomp_enable_profiles:");
  if(!(h->s)) e("");

  return h;
}

void
dump(uint8_t *in, size_t len)
{
  int i;
  puts("packet:");
  for(i = 0; i < len; i++)
  {
    printf("%02x ", in[i]);
    if(i%8 == 7) puts("");
  }
  puts("");
}

int 
ezrohc_comp(ezrohc_h *h, uint8_t *in, uint8_t *out, int len_in)
{
  const struct rohc_buf 
  buf_in = rohc_buf_init_full(in, len_in, _time);
  struct rohc_buf 
  buf_out = rohc_buf_init_empty(out, h->max_len);
  h->s = rohc_compress4(h->c, buf_in, &buf_out);
  if(h->s != 0 && h->s != ROHC_OK) e("compression NOT OK");

  return buf_out.len;
}

int 
ezrohc_decomp(ezrohc_h *h, uint8_t *in, uint8_t *out, int len_in)
{
  const struct rohc_buf 
  buf_in = rohc_buf_init_full(in, len_in, _time);
  struct rohc_buf 
  buf_out = rohc_buf_init_empty(out, h->max_len);
  h->s = rohc_decompress3(h->d, buf_in, &buf_out, NULL, NULL);
  if(h->s != 0 && h->s != ROHC_OK) e("decompression NOT OK");

  return buf_out.len;
}

static void
_print_rohc_traces (void *const priv_ctxt,
		    const rohc_trace_level_t level,
		    const rohc_trace_entity_t entity,
		    const int profile, const char *const format, ...)
{
  return;
#ifndef NDEBUG
  va_list args;
  va_start (args, format);
  vfprintf (stdout, format, args);
  va_end (args);
#endif
}
