/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "v__domainAdmin.h"
#include "v_kernel.h"
#include "v_entity.h"

#include "v__policy.h"

#include "os.h"

#define CLASSNAME "kernelModule::v_domainAdmin"

/**************************************************************
 * Private functions
 **************************************************************/
static c_bool
v_domainAdminAddDomain(
    v_domainAdmin da,
    const char *domainName,
    v_domain *newDomain)
{
    c_bool result = TRUE;
    v_domain domain, found;

    assert(v_domainExpressionIsAbsolute(domainName));
    assert(newDomain != NULL);

    domain = v_domainNew(v_objectKernel(da), domainName, NULL);
    found = c_tableInsert(da->domains, domain);
    if (found != domain) {
        c_free(domain);
        result = FALSE;
        *newDomain = NULL;
    } else {
        /* Do not free domain here because it is returned */
        *newDomain = domain;
    }

    return result;
}

static c_bool
v_domainAdminAddDomainExpression(
    v_domainAdmin da,
    const char *domainExpr)
{
    c_bool result = TRUE;
    v_domainInterest domainInterest, found;

    assert(!v_domainExpressionIsAbsolute(domainExpr));

    domainInterest = v_domainInterestNew(v_objectKernel(da), domainExpr);
    found = c_tableInsert(da->domainInterests, domainInterest);
    if (found != domainInterest) {
        result = FALSE;
    }
    c_free(domainInterest);

    return result;
}

static c_bool
v_domainAdminRemoveDomain(
    v_domainAdmin da,
    const char *domainName,
    v_domain *domainRemoved)
{
    c_bool result;
    v_domain domain, found;
    q_expr expr;
    c_collection q;
    c_iter list;
    c_value params[1];

    assert(v_domainExpressionIsAbsolute(domainName));

    expr = (q_expr)q_parse("name like %0");
    params[0] = c_stringValue((char *)domainName);
    q = c_queryNew(da->domains, expr, params);
    q_dispose(expr);
    list = c_select(q,0);
    assert(c_iterLength(list) <= 1);
    domain = v_domain(c_iterTakeFirst(list));
    if (domain) {
       found = c_tableRemove(da->domains, domain, NULL, NULL);
       *domainRemoved = found;
       c_free(domain);
       result = TRUE;
    } else {
       *domainRemoved = NULL;
       result = FALSE;
    }
    c_free(q);
    c_iterFree(list);

    return result;
}

static c_bool
v_domainAdminRemoveDomainExpression(
    v_domainAdmin da,
    const char *domainExpr)
{
    c_bool result = FALSE;
    v_domainInterest domainInterest, found;
    q_expr expr;
    c_collection q;
    c_iter list;
    c_value params[1];

    assert(!v_domainExpressionIsAbsolute(domainExpr));

    expr = (q_expr)q_parse("expression like %0");
    params[0] = c_stringValue((char *)domainExpr);
    q = c_queryNew(da->domainInterests, expr, params);
    q_dispose(expr);
    list = c_select(q,0);
    assert(c_iterLength(list) <= 1);
    domainInterest = v_domainInterest(c_iterTakeFirst(list));
    while (domainInterest) {
       result = TRUE;
       found = c_tableRemove(da->domainInterests, domainInterest, NULL, NULL);
       c_free(domainInterest);
       c_free(found);
       domainInterest = v_domainInterest(c_iterTakeFirst(list));
    }
    c_free(q);
    c_iterFree(list);

    return result;
}

static c_bool
checkDomainInterest(
    c_object o,
    c_voidp arg)
{
    v_domainInterest domainInterest = (v_domainInterest)o;
    v_domain domain = v_domain(arg);
    c_bool result = TRUE;

    if (v_domainStringMatchesExpression(v_partitionName(domain), domainInterest->expression)) {
        result = FALSE;
    }

    return result;
}

struct resolveDomainsArg {
    v_kernel kernel;
    c_iter   *adomains;
};

static c_bool
resolveDomains(
    c_object o,
    c_voidp arg)
{
    v_domainInterest di = v_domainInterest(o);
    struct resolveDomainsArg *a = (struct resolveDomainsArg *)arg;
    c_iter list;
    v_domain d;

    list = v_resolveDomains(a->kernel, di->expression);
    d = v_domain(c_iterTakeFirst(list));
    while (d != NULL) {
        *(a->adomains) = c_iterInsert(*(a->adomains), d);
        /* The reference is transferred from list to a->adomains,
         * so no c_free(d).
         */
        d = v_domain(c_iterTakeFirst(list));
    }
    c_iterFree(list);

    return TRUE; /* never break the walk */
}

struct updateDomainsArg {
    c_iter *adomains;
    c_iter *rdomains;
};
#include <v_entity.h>
static c_bool
updateDomains(
    c_object o,
    c_voidp arg)
{
    v_domain d = v_domain(o);
    struct updateDomainsArg *a = (struct updateDomainsArg *)arg;

    if (c_iterContains(*(a->adomains), (void *)d)) {
        c_iterTake(*(a->adomains), (void *)d);
        /* remove since we already know the domain */
        c_free(d);

    } else {
        /* add to remove list, we are no longer interested */
        *(a->rdomains) = c_iterInsert(*(a->rdomains), c_keep(d));
    }

    return TRUE; /* never break the walk */
}

static void
removeDomain(
    void *o,
    c_iterActionArg arg)
{
    v_domain d = v_domain(o);
    c_table t = (c_table)arg;
    v_domain found;

    found = c_tableRemove(t, d, NULL, NULL);
    /* 'found' might be NULL, since domains are only added when to this
     * administration when a group exists for that domain
     */
    c_free(found);
}

static void
addDomain(
    void *o,
    c_iterActionArg arg)
{
    v_domain d = v_domain(o);
    c_table t = (c_table)arg;
    v_domain found;

    found = c_tableInsert(t, d);
    if (found != d) {
        c_free(found);
    }
}

/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
v_domainAdmin
v_domainAdminNew(
    v_kernel kernel)
{
    v_domainAdmin da;
    c_base base;

    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(kernel);
    da = v_domainAdmin(v_objectNew(kernel, K_DOMAINADMIN));
    if (da != NULL) {
        da->domains         = c_tableNew(v_kernelType(kernel, K_DOMAIN),"name");
        da->domainInterests = c_tableNew(v_kernelType(kernel, K_DOMAININTEREST), "expression");
        c_mutexInit(&da->mutex,SHARED_MUTEX);

        if ((da->domains == NULL) || (da->domainInterests == NULL)) {
            c_free(da);
            da = NULL;
        }
    }
    return da;
}

void
v_domainAdminFree(
    v_domainAdmin da)
{
    assert(C_TYPECHECK(da,v_domainAdmin));

    if (da != NULL) {
        c_free(da);
    }
}

c_bool
v_domainAdminFitsInterest(
    v_domainAdmin da,
    v_domain d)
{
    c_bool result;

#if 1
    c_mutexLock(&da->mutex);
#endif
    result = !c_tableWalk(da->domainInterests, checkDomainInterest, d);
#if 1
    c_mutexUnlock(&da->mutex);
#endif

    return result;
}

c_iter
v_domainAdminAdd(
    v_domainAdmin da,
    const c_char *domainExpr)
{
/* domainExpr: expression or absolute domain name */
    c_iter domains;
    v_domain d;

    domains = NULL;
    c_mutexLock(&da->mutex);
    if (v_domainExpressionIsAbsolute(domainExpr)) {
        v_domainAdminAddDomain(da, domainExpr, &d);
        domains = c_iterNew(d);
    } else {
        if (v_domainAdminAddDomainExpression(da, domainExpr)) {
            domains = v_resolveDomains(v_objectKernel(da), domainExpr);
            c_iterWalk(domains, addDomain, (c_voidp)da->domains);
        } /* else expression already member */
    }
    c_mutexUnlock(&da->mutex);

    return domains;
}

c_iter
v_domainAdminRemove(
    v_domainAdmin da,
    const c_char *domainExpr)
{
    /* domainExpr: expression or absolute domain name */
    c_iter domains;
    v_domain d;

    domains = NULL;
    c_mutexLock(&da->mutex);
    if (v_domainExpressionIsAbsolute(domainExpr)) {
        v_domainAdminRemoveDomain(da, domainExpr, &d);
        domains = c_iterNew(d);
    } else {
        if (v_domainAdminRemoveDomainExpression(da, domainExpr)) {
            domains = v_resolveDomains(v_objectKernel(da), domainExpr);
            c_iterWalk(domains, removeDomain, (c_voidp)da->domains);
        } /* else expression already member */
    }
    c_mutexUnlock(&da->mutex);

    return domains;
}

c_bool
v_domainAdminSet(
    v_domainAdmin da,
    v_partitionPolicy domainExpressions,
    c_iter *addedDomains,
    c_iter *removedDomains)
{
    c_iter                   dexpressions;    /* iterator of domain expressions */
    c_char                   *dexpr;          /* domain expression */
    v_domainInterest         di;
    struct resolveDomainsArg resolveArg;
    struct updateDomainsArg  updateArg;

    assert(C_TYPECHECK(da, v_domainAdmin));
    assert(removedDomains != NULL);
    assert(addedDomains != NULL);

    *removedDomains = NULL;
    *addedDomains = NULL;

    resolveArg.kernel = v_objectKernel(da);

    c_mutexLock(&da->mutex);
    /*
     * The absolute domain names will be added at the end of
     * the algorithm.
     * The domain expressions in the parameter of domainExpressions,
     * replace the existing in da->domainInterests.
     */
    c_free(da->domainInterests);
    da->domainInterests = c_tableNew(v_kernelType(resolveArg.kernel, K_DOMAININTEREST),
                              "expression");
    assert(c_count(da->domainInterests) == 0);
    dexpressions = v_partitionPolicySplit(domainExpressions);
    if (dexpressions == NULL) {
        /* switch to default */
        *addedDomains = c_iterInsert(*addedDomains, v_domainNew(resolveArg.kernel, "", NULL));
    } else {
        dexpr = (c_char *)c_iterTakeFirst(dexpressions);
        while (dexpr != NULL) {
            if (v_domainExpressionIsAbsolute(dexpr)) {
                *addedDomains = c_iterInsert(*addedDomains,
                    v_domainNew(resolveArg.kernel, dexpr, NULL));
                 /* ref transferred to addedDomains */
            } else {
                di = v_domainInterestNew(resolveArg.kernel, (const c_char *)dexpr);
                c_tableInsert(da->domainInterests, di);
                c_free(di);
            }
            os_free(dexpr);
            dexpr = (c_char *)c_iterTakeFirst(dexpressions);
        }
        c_iterFree(dexpressions);
    }

    /*
     * The given expressions are now divided across
     * 'addeddomains' and 'da->domainInterests'.
     * Now first add domains to 'addeddomains' that fit the
     * expressions in 'da->domainInterests'.
     */
    resolveArg.adomains = addedDomains;
    c_tableWalk(da->domainInterests, resolveDomains, (c_voidp)&resolveArg);

    /*
     * Now 'addeddomains' contains all domains to be added
     * by the publisher/subscriber.
     * 'da->domains' contains the old set of domains.
     * We must check whether those domains must remain in
     * the set or must be removed.
     * For every domain in 'da->domains' do
     *    if domain in 'addeddomains' then remove from 'addeddomains'
     *    else add to 'removeddomains'
     * For every domain in 'removeddomains' remove from 'da->domains'.
     */
    updateArg.adomains = addedDomains;
    updateArg.rdomains = removedDomains;
    c_tableWalk(da->domains, updateDomains, (c_voidp)&updateArg);

    c_iterWalk(*removedDomains, removeDomain, (c_voidp)da->domains);

    /*
     * The da->domains now contains domains that still comply to new
     * partitionPolicy. So all domains in added domains, must be added
     * to da->domains, so it reflects all connected domains.
     */
    c_iterWalk(*addedDomains, addDomain, (c_voidp)da->domains);
    c_mutexUnlock(&da->mutex);

    return TRUE;
}

c_bool
v_domainAdminDomainExists(
    v_domainAdmin da,
    const c_char *name)
{
    v_domain found;
    C_STRUCT(v_entity) template;

    assert(da != NULL);
    assert(C_TYPECHECK(da,v_domainAdmin));

    template.name = c_stringNew(c_getBase(da), name);
    c_mutexLock(&da->mutex);
    found = c_find(da->domains,&template);
    c_mutexUnlock(&da->mutex);
    c_free(template.name);
    
    if (found) {
        c_free(found);
        return TRUE;
    }
    return FALSE;
}

c_iter
v_domainAdminLookupDomains(
    v_domainAdmin da,
    const c_char *domainExpr)
{
    c_iter list;
    c_collection q;
    q_expr expr;
    c_value params[1];

    assert(da != NULL);
    assert(C_TYPECHECK(da,v_domainAdmin));

    expr = (q_expr)q_parse("name like %0");
    params[0] = c_stringValue((c_char *)domainExpr);
    c_mutexLock(&da->mutex);
    q = c_queryNew(da->domains,expr,params);
    list = c_select(q,0);
    c_mutexUnlock(&da->mutex);
    q_dispose(expr);
    c_free(q);

    return list;
}

c_bool
v_domainAdminWalkDomains(
    v_domainAdmin da,
    c_action action,
    c_voidp arg)
{
    c_bool result;

    c_mutexLock(&da->mutex);
    result = c_tableWalk(da->domains, action, arg);
    c_mutexUnlock(&da->mutex);

    return result;
}

/**************************************************************
 * Public functions
 **************************************************************/
