#ifndef NN_BITSET_H
#define NN_BITSET_H

#include <assert.h>
#include <string.h>

#include "q_inline.h"

#if NN_HAVE_C99_INLINE && !defined SUPPRESS_BITSET_INLINES
#include "q_bitset_template.c"
#else
#if defined (__cplusplus)
extern "C" {
#endif
int nn_bitset_isset (unsigned numbits, const unsigned *bits, unsigned idx);
void nn_bitset_set (unsigned numbits, unsigned *bits, unsigned idx);
void nn_bitset_clear (unsigned numbits, unsigned *bits, unsigned idx);
void nn_bitset_zero (unsigned numbits, unsigned *bits);
void nn_bitset_one (unsigned numbits, unsigned *bits);
#if defined (__cplusplus)
}
#endif
#endif

#endif /* NN_BITSET_H */

/* SHA1 not available (unoffical build.) */
