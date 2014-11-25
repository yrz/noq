#include "ezrohc.h"
#include "dbg.h"
#include <stdarg.h>
#include <stdio.h>

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
  h->out = malloc(sizeof (struct rohc_buf));
/*  h->out->data = b; */
  h->out->max_len = buf_size;
  
  if(h == NULL) e("");

  h->c = rohc_comp_new2 (ROHC_SMALL_CID, ROHC_SMALL_CID_MAX, _rohc_rand, NULL);
  if(h->c == NULL) e("");
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
  h->s = rohc_decomp_enable_profiles (h->d,
				      ROHC_PROFILE_UNCOMPRESSED,
				      ROHC_PROFILE_IP,
				      ROHC_PROFILE_TCP,
				      ROHC_PROFILE_UDP,
				      ROHC_PROFILE_ESP,
				      ROHC_PROFILE_RTP,
				      ROHC_PROFILE_UDPLITE, -1);
  if(!(h->s)) e("rohc_decomp_enable_profiles:");
  h->s = rohc_decomp_set_traces_cb2(h->d, _print_rohc_traces, NULL);
  if(!(h->s)) e("");

  return h;
}

int 
ezrohc_comp(ezrohc_h *h, uint8_t *in, uint8_t *out, int len_in)
{
  rohc_buf_reset(&(h->in));
  rohc_buf_reset(h->out);
  h->in.data = in;
  h->out->data = out;
  h->in.len = len_in;
  h->s = rohc_compress4(h->c, h->in, h->out);
  if(h->s != ROHC_OK) e("compression NOT OK");

  return h->out->len;
}

int 
ezrohc_decomp(ezrohc_h *h, uint8_t *in, uint8_t *out, int len_in)
{
  rohc_buf_reset(&(h->in));
  rohc_buf_reset(h->out);
  h->in.data = in;
  h->out->data = out;
  h->in.len = len_in;
  h->s = rohc_decompress3(h->d, h->in, h->out, NULL, NULL);
  if(h->s != ROHC_OK) e("decompression NOT OK");

  return h->out->len;
}


static void
_print_rohc_traces (void *const priv_ctxt,
		    const rohc_trace_level_t level,
		    const rohc_trace_entity_t entity,
		    const int profile, const char *const format, ...)
{
#ifndef NDEBUG
    va_list args;
    va_start (args, format);
    vfprintf (stdout, format, args);
    va_end (args);
#endif
}
