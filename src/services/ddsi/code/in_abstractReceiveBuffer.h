/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef IN_ABSTRACTRECEIVEBUFFER_H_
#define IN_ABSTRACTRECEIVEBUFFER_H_

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
#define in_abstractReceiveBuffer(_obj) ((in_abstractReceiveBuffer)_obj)


/** \brief decrease refcount */
void
in_abstractReceiveBufferFree(in_abstractReceiveBuffer _this);


/** \brief return pointer to first octet in buffer */
in_octet*
in_abstractReceiveBufferBegin(in_abstractReceiveBuffer dataBuffer);

/** \brief return number of octets in buffer
 */
os_size_t
in_abstractReceiveBufferLength(in_abstractReceiveBuffer dataBuffer);

/**  \return constant locator */
in_locator
in_abstractReceiveBufferGetSender(in_abstractReceiveBuffer _this);

/**  \return copies the locator */
void
in_abstractReceiveBufferSetSender(in_abstractReceiveBuffer _this, in_locator locator);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif


#endif /* IN_ABSTRACTRECEIVEBUFFER_H_ */
