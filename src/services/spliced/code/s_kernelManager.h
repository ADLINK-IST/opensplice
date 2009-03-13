#ifndef S_KERNELMANAGER_H
#define S_KERNELMANAGER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "s_types.h"

#define s_kernelManager(o) ((s_kernelManager)o)

s_kernelManager
s_kernelManagerNew(
   spliced daemon);
void
s_kernelManagerFree(
    s_kernelManager km);

/* wait for kernel manager to become active */
int
s_kernelManagerWaitForActive(
    s_kernelManager km);


#if defined (__cplusplus)
}
#endif

#endif /* S_KERNELMANAGER_H */
