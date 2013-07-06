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
