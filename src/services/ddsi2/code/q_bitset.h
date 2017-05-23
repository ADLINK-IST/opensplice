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
