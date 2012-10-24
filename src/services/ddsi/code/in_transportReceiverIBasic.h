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
in_transportReceiverIBasicNew(
        in_configChannel config);


/** \brief constructor */
in_transportReceiverIBasic
in_transportReceiverIBasicNewDuplex(
        in_configChannel config,
        in_socket duplexSocket);

/** \brief destructor */
void
in_transportReceiverIBasicFree(
		in_transportReceiverIBasic _this);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif



#endif /* IN_TRANSPORTREADERIBASIC_H_ */
