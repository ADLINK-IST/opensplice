
#ifndef V__DOMAINADMIN_H
#define V__DOMAINADMIN_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "kernelModule.h"
#include "v__domain.h"

#define v_domainAdmin(o) (C_CAST(o,v_domainAdmin))

v_domainAdmin
v_domainAdminNew(
    v_kernel kernel);

void
v_domainAdminFree(
    v_domainAdmin _this);

c_bool
v_domainAdminFitsInterest(
    v_domainAdmin _this,
    v_domain d);

c_iter
v_domainAdminAdd(
    v_domainAdmin _this,
    const c_char *domainExpr);

c_iter
v_domainAdminRemove(
    v_domainAdmin _this,
    const c_char *domainExpr);

c_bool
v_domainAdminSet(
    v_domainAdmin _this,
    v_partitionPolicy domainExpressions,
    c_iter *addedDomains,
    c_iter *removedDomains);

c_iter
v_domainAdminLookupDomains(
    v_domainAdmin _this,
    const c_char *domainExpr);

c_bool
v_domainAdminWalkDomains(
    v_domainAdmin _this,
    c_action action,
    c_voidp arg);


#if defined (__cplusplus)
}
#endif

#endif /* V__DOMAINADMIN_H */
