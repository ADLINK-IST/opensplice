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

#ifndef V__SERVICE_H
#define V__SERVICE_H

#include "v_service.h"
#include "v_event.h"

void
v_serviceNotify(
    v_service _this,
    v_event event,
    c_voidp userData);

#endif /* V__SERVICE_H */
