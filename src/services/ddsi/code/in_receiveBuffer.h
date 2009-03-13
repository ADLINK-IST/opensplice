/*
 * in_abstractReceiveBufferBasic.h
 *
 *  Created on: Feb 8, 2009
 *      Author: frehberg
 */

#ifndef IN_RECEIVEBUFFER_H_
#define IN_RECEIVEBUFFER_H_

#include "in_abstractReceiveBuffer.h"
#include "in_commonTypes.h"

/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif

#define in_receiveBuffer(_obj) \
	(in_receiveBuffer)(_obj)

/** \brief constructor
 * \param poritive integer between 0..2^16-1 */
in_receiveBuffer
in_receiveBufferNew(in_long capacity);

/** \brief destructor */
void
in_receiveBufferFree(in_receiveBuffer _this);

/* \brief reset the length attribute and check locator's refcounter is set to 1 */
void
in_receiveBufferReset(in_receiveBuffer _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_RECEIVEBUFFER_H_ */
