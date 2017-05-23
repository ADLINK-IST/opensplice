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

#if defined SUPPRESS_BSWAP_INLINES && defined NN_C99_INLINE
#undef NN_C99_INLINE
#define NN_C99_INLINE
#endif

NN_C99_INLINE unsigned short bswap2u (unsigned short x)
{
  return (unsigned short) ((x >> 8) | (x << 8));
}

NN_C99_INLINE unsigned bswap4u (unsigned x)
{
  return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
}

NN_C99_INLINE unsigned long long bswap8u (unsigned long long x)
{
  const unsigned newhi = bswap4u ((unsigned) x);
  const unsigned newlo = bswap4u ((unsigned) (x >> 32));
  return ((unsigned long long) newhi << 32) | (unsigned long long) newlo;
}

NN_C99_INLINE void bswapSN (nn_sequence_number_t *sn)
{
  sn->high = bswap4 (sn->high);
  sn->low = bswap4u (sn->low);
}


/* SHA1 not available (unoffical build.) */
