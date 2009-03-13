
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
