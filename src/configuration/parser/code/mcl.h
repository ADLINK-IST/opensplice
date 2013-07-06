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

#ifndef CFG_H
#define CFG_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_CLASS(cfg_memoryClaimList);

typedef void (*cfg_memoryClaimListFreeFunc)(void *);

cfg_memoryClaimList
cfg_memoryClaimListNew();
void
cfg_memoryClaimListFree(
    cfg_memoryClaimList mcl);

void *
cfg_memoryClaimListAdd(
    cfg_memoryClaimList mcl,
    void *memory,
    cfg_memoryClaimListFreeFunc freeFunc);
void *
cfg_memoryClaimListRemove(
    cfg_memoryClaimList mcl,
    void *memory);

void
cfg_memoryClaimListReleaseAll(
    cfg_memoryClaimList mcl);
c_long
cfg_memoryClaimListClaimCount(
    cfg_memoryClaimList mcl);


#if defined (__cplusplus)
}
#endif

#endif /* CFG_H */
