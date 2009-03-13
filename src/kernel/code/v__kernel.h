#ifndef V__KERNEL_H
#define V__KERNEL_H

#include "v_kernel.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */


/* define v_kernelGetQos as macro, since this qos is readonly! */
#define v_kernelGetQos(_this) (c_keep(_this->qos))

OS_API c_bool
v_isEnabledStatistics (
    v_kernel _this,
    const char *categoryName);

OS_API void
v_lockShares (
    v_kernel _this);

OS_API void
v_unlockShares (
    v_kernel _this);

OS_API v_entity
v_addShareUnsafe (
    v_kernel _this,
    v_entity e);

OS_API v_entity
v_removeShare (
    v_kernel _this,
    v_entity e);

OS_API c_iter
v_resolveShare (
    v_kernel _this,
    const c_char *name);

#undef OS_API

#endif /* V__KERNEL_H */
