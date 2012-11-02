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
#ifndef IN_TRANSPORTPAIRIBASIC_H_
#define IN_TRANSPORTPAIRIBASIC_H_

#include "in_transport.h"
#include "in__configChannel.h"

/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif

#define in_transportPairIBasic(_o) \
	((in_transportPairIBasic)_o)


/** \brief constructor */
OS_STRUCT(in_transportPairIBasic)*
in_transportPairIBasicNew(const in_configChannel configChannel);


/** \brief decrease refcount */
void
in_transportPairIBasicFree(in_transportPairIBasic _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif




#endif /* IN_TRANSPORTPAIRIBASIC_H_ */
