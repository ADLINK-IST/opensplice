
#ifndef V__STATUS_H
#define V__STATUS_H

#include "v_status.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

enum v_statusLiveliness {
    V_STATUSLIVELINESS_UNKNOWN,
    V_STATUSLIVELINESS_ALIVE,
    V_STATUSLIVELINESS_NOTALIVE,
    V_STATUSLIVELINESS_DELETED,
    V_STATUSLIVELINESS_COUNT /* always the last! */
};

OS_API c_bool
v_statusNotifyInconsistentTopic (
    v_status _this);

OS_API c_bool
v_statusNotifyDataAvailable (
    v_status _this);

OS_API c_bool
v_statusNotifySampleLost (
    v_status _this);

OS_API c_bool
v_statusNotifyLivelinessLost (
    v_status _this);

OS_API c_bool
v_statusNotifyDeadlineMissed (
    v_status _this,
    c_object h);

OS_API c_bool
v_statusNotifyIncompatibleQos (
    v_status _this,
    v_policyId id);

OS_API c_bool
v_statusNotifyLivelinessChanged (
    v_status _this,
    c_long activeInc,
    c_long inactiveInc,
    v_gid instanceHandle);

OS_API c_bool
v_statusNotifySampleRejected (
    v_status _this,
    v_sampleRejectedKind r,
    v_gid instanceHandle);

#undef OS_API

#endif
