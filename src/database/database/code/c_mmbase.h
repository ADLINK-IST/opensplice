
#ifndef C_MM_H
#define C_MM_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef struct c_mm_s       *c_mm;
typedef struct c_mmStatus_s c_mmStatus;

struct c_mmStatus_s {
    c_long size;
    c_long used;
    c_long maxUsed;
    c_long garbage;
    c_long count;
    c_long fails;
};

OS_API c_mm c_mmCreate (void *address, c_long size);
OS_API c_mmStatus c_mmState (c_mm mm);
   
void  c_mmDestroy (c_mm mm);
void *c_mmAddress (c_mm mm);

void *c_mmMalloc  (c_mm mm, c_long   size);
void  c_mmFree    (c_mm mm, void *memory);

void *c_mmBind    (c_mm mm, const c_char *name, void *memory);
void  c_mmUnbind  (c_mm mm, const c_char *name);
void *c_mmLookup  (c_mm mm, const c_char *name);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* C_MM_H */
