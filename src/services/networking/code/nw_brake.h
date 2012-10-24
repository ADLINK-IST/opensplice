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

#ifndef NW_BRAKE_H
#define NW_BRAKE_H

#include "nw_commonTypes.h"

NW_CLASS(nw_brake);

nw_brake nw_brakeNew  (unsigned int maxThroughput,
                       unsigned int maxBurstSize,
                       unsigned int minSleepMSecs);
                       
void nw_brakeFree     (nw_brake brake);

void nw_brakeAddUnits (nw_brake brake,
                       unsigned int units);
                       
void nw_brakeSlowDown (nw_brake brake);

#endif
