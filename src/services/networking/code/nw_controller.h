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

#ifndef NW_CONTROLLER_H
#define NW_CONTROLLER_H

#include "nw_commonTypes.h"
#include "u_service.h"

NW_CLASS(nw_controller);

typedef void (*nw_controllerListener)(c_voidp usrData);

nw_controller nw_controllerNew   (u_service service, nw_controllerListener onFatal, c_voidp usrData);
void          nw_controllerStart (nw_controller controller);
void          nw_controllerStop  (nw_controller controller);
void          nw_controllerFree  (nw_controller controller);

/* Function for updating interest to the kernel */
void          nw_controllerUpdateHeartbeats(nw_controller controller);

#endif /* NW_CONTROLLER_H */

