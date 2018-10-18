/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
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
    const unsigned k = numbits / 32;
    const unsigned n = numbits % 32;
    bits[k] &= ~(~0u >> n);
  }
}
