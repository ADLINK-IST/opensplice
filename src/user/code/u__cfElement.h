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

#ifndef U__CFELEMENT_H
#define U__CFELEMENT_H

#include "u__cfNode.h"
#include "u_cfElement.h"

#include "v_cfElement.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(u_cfElement) {
    C_EXTENDS(u_cfNode);
};

#define u_cfElement(o) ((u_cfElement)(o))

u_cfElement
u_cfElementNew(
    u_participant participant,
    v_cfElement kElement);

#if defined (__cplusplus)
}
#endif

#endif /* U__CFELEMENT_H */
