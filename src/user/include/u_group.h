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

#define  u_group(o)  ((u_group)(o))

/* To be called from protected threads only */
OS_API u_group   u_groupCreate(
              v_group group,
              u_participant participant);

/* Functions taking care of the protection themselves */
OS_API u_result  
u_groupClaim(
    u_group group,
    v_group *kg);
    
OS_API u_result  
u_groupRelease(
    u_group group);
    
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
