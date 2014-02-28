/* -*- c -*- */

#if defined SUPPRESS_BSWAP_INLINES && defined NN_C99_INLINE
#undef NN_C99_INLINE
#define NN_C99_INLINE
#endif

NN_C99_INLINE unsigned short bswap2u (unsigned short x)
{
  return (x >> 8) | (x << 8);
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
