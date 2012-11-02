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
#ifndef IN_CONTROLLER_H
#define IN_CONTROLLER_H

#include "in_commonTypes.h"
#include "u_service.h"



in_controller
in_controllerNew(
    u_service service);

void
in_controllerStart(
    in_controller controller);

void
in_controllerStop(
    in_controller controller);

void
in_controllerFree(
    in_controller controller);

#endif /* IN_CONTROLLER_H */

