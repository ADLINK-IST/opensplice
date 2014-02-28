/* -*- c -*- */

#include "q_unused.h"

#if defined SUPPRESS_BITSET_INLINES && defined NN_C99_INLINE
#undef NN_C99_INLINE
#define NN_C99_INLINE
#endif

NN_C99_INLINE int nn_bitset_isset (unsigned numbits, const unsigned *bits, unsigned idx)
{
  return idx < numbits && (bits[idx/32] & (1u << (31 - (idx%32))));
}

NN_C99_INLINE void nn_bitset_set (UNUSED_ARG_NDEBUG (unsigned numbits), unsigned *bits, unsigned idx)
{
  assert (idx < numbits);
  bits[idx/32] |= 1u << (31 - (idx%32));
}

NN_C99_INLINE void nn_bitset_clear (UNUSED_ARG_NDEBUG (unsigned numbits), unsigned *bits, unsigned idx)
{
  assert (idx < numbits);
  bits[idx/32] &= ~(1u << (31 - (idx%32)));
}

NN_C99_INLINE void nn_bitset_zero (unsigned numbits, unsigned *bits)
{
  memset (bits, 0, 4 * ((numbits + 31) / 32));
}

NN_C99_INLINE void nn_bitset_one (unsigned numbits, unsigned *bits)
{
  memset (bits, 0xff, 4 * ((numbits + 31) / 32));

  /* clear bits "accidentally" set */
  {
    const int k = numbits / 32;
    const int n = numbits % 32;
    bits[k] &= ~(~0u >> n);
  }
}

/* SHA1 not available (unoffical build.) */
