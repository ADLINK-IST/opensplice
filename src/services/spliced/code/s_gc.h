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
