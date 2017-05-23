/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
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

/* SHA1 not available (unoffical build.) */
