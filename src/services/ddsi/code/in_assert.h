/*
 * in_assert.h
 *
 *  Created on: Feb 24, 2009
 *      Author: frehberg
 */

#ifndef IN_ASSERT_H_
#define IN_ASSERT_H_

#include <assert.h>

#if defined (__cplusplus)
extern "C" {
#endif

/** */
#define in_assert_not_implemented  assert(!"not implemented yet")

/** */
#define in_assert_never_reached assert(!"never reached")

#if defined (__cplusplus)
}
#endif


#endif /* IN_ASSERT_H_ */
