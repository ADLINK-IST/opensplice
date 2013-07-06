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

#ifndef U__CFATTRIBUTE_H
#define U__CFATTRIBUTE_H

#include "u__cfNode.h"
#include "u_cfAttribute.h"

#include "v_cfAttribute.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(u_cfAttribute) {
    C_EXTENDS(u_cfNode);
};

#define u_cfAttribute(o) ((u_cfAttribute)(o))

u_cfAttribute
u_cfAttributeNew(
    u_participant participant,
    v_cfAttribute kAttribute);

#if defined (__cplusplus)
}
#endif

#endif /* U__CFATTRIBUTE_H */
