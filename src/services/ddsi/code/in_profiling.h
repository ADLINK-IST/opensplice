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
#ifndef IN_PROFILING_H
#define IN_PROFILING_H

#include "in_configuration.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef IN_PROFILING

#define PR(name) PROF_##name

typedef enum in_profilingClass_e {
    PR(BridgeWrite),   PR(BridgeRead_1),    PR(BridgeRead_2),
    PR(PlugWrite),     PR(PlugRead_1),      PR(PlugRead_2),
    PR(Frag),          PR(Defrag),          PR(DefragCleanUp),
    PR(SendTo),        PR(RecvFrom),
    PR(Serialization), PR(Deserialization),
    PR(Count) /* Last element, keep this at the end */
} in_profilingClass;

/* You can use thes functions as is, but using the macro's below are preferred
 * Note: this function is implemented in in_configuration in order to speed
 * up the testing etc. */
void in_profilingLapStart(in_profilingClass profilingClass);
void in_profilingLapStop(in_profilingClass profilingClass);

#define IN_PROF_LAPSTART(class) in_profilingLapStart(PR(class))
#define IN_PROF_LAPSTOP(class)  in_profilingLapStop(PR(class))

#else /* IN_PROFILING */

#define IN_PROF_LAPSTART(class)
#define IN_PROF_LAPSTOP(class)

#endif /* IN_PROFILING */


#if defined (__cplusplus)
}
#endif

#endif /* IN_PROFILING_H */
