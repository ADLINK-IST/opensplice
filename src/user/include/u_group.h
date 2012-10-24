/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef U_GROUP_H
#define U_GROUP_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "v_kernel.h"
#include "v_group.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_group(o) \
        ((u_group)u_entityCheckType(u_entity(o), U_GROUP))

/* To be called from protected threads only */
OS_API u_group
u_groupCreate(
    v_group group,
    u_participant participant);

/* Functions taking care of the protection themselves */

OS_API u_result
u_groupFree(
    u_group _this);

OS_API u_group
u_groupNew(
    u_participant participant,
    const c_char *partitionName,
    const c_char *topicName,
    v_duration timeout);

OS_API u_result
u_groupFlush(
    u_group group);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_GROUP_H */
