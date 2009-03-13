
#ifndef V__LEASEMANAGER_H
#define V__LEASEMANAGER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "kernelModule.h"
#include "v_leaseManager.h"

v_leaseManager
v_leaseManagerNew(
    v_kernel k);

void
v_leaseManagerFree(
    v_leaseManager _this);

v_lease
v_leaseManagerRegister(
    v_leaseManager  _this,
    v_public        objectToLease,
    v_duration      leaseDuration,
    v_leaseActionId actionId,
    c_long          repeatCount);

void
v_leaseManagerDeregister(
    v_leaseManager _this,
    v_lease lease);

#if defined (__cplusplus)
}
#endif

#endif /* V__LEASEMANAGER_H */
