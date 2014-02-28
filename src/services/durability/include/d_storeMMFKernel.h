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

#ifndef D_STOREKERNEL_H
#define D_STOREKERNEL_H

#include "durabilityModule2.h"
#include "d_groupInfo.h"
#include "d_nameSpace.h"
#include "d_store.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_DURABILITY
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
* \brief The <code>d_storeMMFKernel</code> cast method.
*
* This method casts an object to a <code>d_storeMMFKernel</code> object.
* Before the cast is performed, if compiled with the NDEBUG flag not set,
* the type of the object is checked to be <code>d_storeMMFKernel</code> or
* one of its subclasses.
*/
#define d_storeMMFKernel(o)  (C_CAST(o,d_storeMMFKernel))

d_storeMMFKernel
d_storeMMFKernelAttach(
    c_base base,
    const c_char *name);

d_storeMMFKernel
d_storeMMFKernelNew(
    c_base b,
    const c_char *name);

d_groupInfo
d_storeMMFKernelGetGroupInfo(
    d_storeMMFKernel _this,
    const char *partition,
    const char *topic);

d_storeResult
d_storeMMFKernelAddGroupInfo(
    d_storeMMFKernel _this,
    const d_group group);

d_storeResult
d_storeMMFKernelMarkNameSpaceComplete(
    d_storeMMFKernel kernel,
    const d_nameSpace nameSpace,
    const c_bool isComplete);

d_storeResult
d_storeMMFKernelIsNameSpaceComplete(
    d_storeMMFKernel kernel,
    const d_nameSpace nameSpace,
    c_bool* isComplete);

d_storeResult
d_storeMMFKernelGetQuality(
    d_storeMMFKernel kernel,
    const d_nameSpace nameSpace,
    d_quality* quality);

d_storeResult
d_storeMMFKernelBackup(
    d_storeMMFKernel kernel,
    const d_store store,
    const d_nameSpace nameSpace);

d_storeResult
d_storeMMFKernelBackupRestore(
    d_storeMMFKernel kernel,
    const d_store store,
    const d_nameSpace nameSpace);

d_storeResult
d_storeMMFKernelUpdateQuality(
    d_storeMMFKernel kernel,
    d_quality quality);

d_storeResult
d_storeMMFKernelDeleteNonMatchingGroups(
    d_storeMMFKernel _this,
    c_string partitionExpr,
    c_string topicExpr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /*D_STOREMEMMAPFILE_H*/
