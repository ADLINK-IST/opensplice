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

#ifndef U__CFVALUE_H
#define U__CFVALUE_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

c_bool
u_cfValueScan(
    c_value value,
    c_valueKind valueKind,
    c_value *valuePtr);

#if defined (__cplusplus)
}
#endif

#endif /* U__CFVALUE_H */
