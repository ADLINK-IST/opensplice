#ifndef NN_BITSET_H
#define NN_BITSET_H

#include <assert.h>
#include <string.h>

#include "nn_inline.h"

#if NN_HAVE_C99_INLINE
#include "nn_bitset.template"
#else
int nn_bitset_isset (unsigned numbits, const unsigned *bits, unsigned idx);
void nn_bitset_set (unsigned numbits, unsigned *bits, unsigned idx);
void nn_bitset_clear (unsigned numbits, unsigned *bits, unsigned idx);
void nn_bitset_zero (unsigned numbits, unsigned *bits);
#endif

#endif /* NN_BITSET_H */
