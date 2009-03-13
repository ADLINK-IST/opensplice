
#ifndef NW_RUNNABLE_H
#define NW_RUNNABLE_H

#include "nw_commonTypes.h"

NW_CLASS(nw_runnable);

void        nw_runnableStart(
                nw_runnable runnable);
                
void        nw_runnableStop(
                nw_runnable runnable);
                
void        nw_runnableFree(
                nw_runnable runnable);

#endif /* NW_RUNNABLE_H */

