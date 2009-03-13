
#ifndef U__KERNEL_H
#define U__KERNEL_H

#include "u_participant.h"
#include "u_kernel.h"

u_result
u_kernelClaim (
    u_kernel _this,
    v_kernel *kernel);

u_result
u_kernelRelease (
    u_kernel _this);

u_result
u_kernelAdd (
    u_kernel _this,
    u_participant p);

u_result
u_kernelRemove (
    u_kernel _this,
    u_participant p);

u_result
u_kernelDetachParticipants (
    u_kernel _this);

c_bool
u_kernelCheckHandleServer (
    u_kernel _this,
    c_long serverId);

#endif

