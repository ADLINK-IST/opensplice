#ifndef C__EXTENT_H
#define C__EXTENT_H

#include "c_extent.h"
#include "c_mmCache.h"
#include "c__metabase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define c_extentSync(o) ((c_extentSync)(o))

C_CLASS(c_extentSync);

/* NOTE:
 * the order of the attributes must obey the meta data ordering rules.
 */
C_STRUCT(c_extent) {
    c_mmCache cache;
    c_voidp   next;
    c_voidp   prev;
    c_type    type;
    c_bool    sync;
};

C_STRUCT(c_extentSync) {
    C_EXTENDS(c_extent);
    c_mutex mutex;
};

void
c_extentFree (
    c_extent _this);

c_voidp
c_extentMalloc (
    c_extent _this,
    c_long size);

c_type
c_extentRegister (
    c_base base);

#if defined (__cplusplus)
}
#endif

#endif 
