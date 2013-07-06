/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/**@file api/cm/xml/code/cmx__waitset.h
 * 
 * Offers internal routines on a waitset.
 */
#ifndef CMX__WAITSET_H
#define CMX__WAITSET_H

#include "c_typebase.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_waitset.h"

c_char* cmx_waitsetInit         (v_waitset entity);

#if defined (__cplusplus)
}
#endif

#endif /*CMX__WAITSET_H*/
