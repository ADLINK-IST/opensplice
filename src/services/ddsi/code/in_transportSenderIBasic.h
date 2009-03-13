/*
 * in_transportSenderIBasic.h
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

#ifndef IN_TRANSPORTWRITERIBASIC_H_
#define IN_TRANSPORTWRITERIBASIC_H_
#include "in__object.h"
#include "in_abstractSendBuffer.h"
#include "os_time.h"
#include "in_locator.h"
#include "in__configChannel.h"
/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif


/** */
#define in_transportSenderIBasic(_o) \
    ((in_transportSenderIBasic)_o)

/** \brief constructor */
in_transportSenderIBasic
in_transportSenderIBasicNew(in_configChannel config);


/** \brief destructor */
void
in_transportSenderIBasicFree(
		in_transportSenderIBasic _this);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif



#endif /* IN_TRANSPORTWRITERIBASIC_H_ */
