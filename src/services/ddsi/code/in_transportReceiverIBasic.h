/*
 * in_transportReceiverIBasic.h
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

#ifndef IN_TRANSPORTREADERIBASIC_H_
#define IN_TRANSPORTREADERIBASIC_H_
#include "in__object.h"
#include "c_time.h"
#include "in_locator.h"
#include "in__configChannel.h"
/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif


/** \brief caster */
#define in_transportReceiverIBasic(_obj) \
    ((in_transportReceiverIBasic)_obj)

/** \brief constructor */
in_transportReceiverIBasic
in_transportReceiverIBasicNew(in_configChannel config);


/** \brief destructor */
void
in_transportReceiverIBasicFree(
		in_transportReceiverIBasic _this);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif



#endif /* IN_TRANSPORTREADERIBASIC_H_ */
