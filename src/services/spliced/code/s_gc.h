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
#ifndef S_GARBAGECOLLECTOR_H
#define S_GARBAGECOLLECTOR_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "s_types.h"

#define s_garbageCollector(o) ((s_garageCollector)o)
    
s_garbageCollector
s_garbageCollectorNew(
    spliced daemon);
    
void
s_garbageCollectorFree(
    s_garbageCollector gc);

int
s_garbageCollectorWaitForActive(
    s_garbageCollector gc);

#if defined (__cplusplus)
}
#endif

#endif /* S_GARBAGECOLLECTOR_H */
