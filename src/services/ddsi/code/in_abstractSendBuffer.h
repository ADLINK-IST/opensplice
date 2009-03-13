/*
 * in_abstractSendBuffer.h
 *
 *  Created on: Feb 6, 2009
 *      Author: frehberg
 */

#ifndef IN_ABSTRACTSENDBUFFER_H_
#define IN_ABSTRACTSENDBUFFER_H_

#include "in_commonTypes.h"
#include "in_locator.h"

/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif

/* \brief padding octet */
#define IN_PADDING_OCTET ((in_octet)0x0)


/** \brief cast operation */
#define in_abstractSendBuffer(_obj) ((in_abstractSendBuffer)_obj)

/** \brief destructor
 *
 * Object will be destroyed if refcounter reaches 0 */
void
in_abstractSendBufferFree(in_abstractSendBuffer _this);

/** \brief return pointer to first octet in buffer */
in_octet*
in_abstractSendBufferBegin(in_abstractSendBuffer dataBuffer);

/** \brief return number of octets in buffer
 */
in_ulong
in_abstractSendBufferLength(in_abstractSendBuffer dataBuffer);

/**  \return constant locator */
in_locator
in_abstractSendBufferGetLocator(in_abstractSendBuffer _this);

/**  \return copies the locator */
void
in_abstractSendBufferSetLocator(in_abstractSendBuffer _this, in_locator locator);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif


#endif /* IN_ABSTRACTSENDBUFFER_H_ */
