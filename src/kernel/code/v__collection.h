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

#ifndef V__COLLECTION_H
#define V__COLLECTION_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_collection.h"

void
v_collectionInit (
    v_collection _this,
    const c_char *name,
    v_statistics s,
    c_bool enable);

void
v_collectionDeinit (
    v_collection _this);

#if defined (__cplusplus)
}
#endif

#endif /* V__COLLECTION_H */
