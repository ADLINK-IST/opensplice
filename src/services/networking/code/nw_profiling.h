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

#ifndef NW_PROFILING_H
#define NW_PROFILING_H

#include "nw_configuration.h"

#ifdef NW_PROFILING

#define PR(name) PROF_##name

typedef enum nw_profilingClass_e {
    PR(BridgeWrite),   PR(BridgeRead_1),    PR(BridgeRead_2),
    PR(PlugWrite),     PR(PlugRead_1),      PR(PlugRead_2),
    PR(Frag),          PR(Defrag),          PR(DefragCleanUp),
    PR(SendTo),        PR(RecvFrom),
    PR(Serialization), PR(Deserialization),
    PR(Count) /* Last element, keep this at the end */
} nw_profilingClass;

/* You can use thes functions as is, but using the macro's below are preferred 
 * Note: this function is implemented in nw_configuration in order to speed
 * up the testing etc. */
void nw_profilingLapStart(nw_profilingClass profilingClass);
void nw_profilingLapStop(nw_profilingClass profilingClass);

#define NW_PROF_LAPSTART(class) nw_profilingLapStart(PR(class))
#define NW_PROF_LAPSTOP(class)  nw_profilingLapStop(PR(class))
    
#else /* NW_PROFILING */

#define NW_PROF_LAPSTART(class)
#define NW_PROF_LAPSTOP(class)

#endif /* NW_PROFILING */

#endif /* NW_PROFILING_H */
