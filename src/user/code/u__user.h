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
#ifndef U__USER_H
#define U__USER_H

#include "u_user.h"
#include "u__domain.h"

#define OSRPT_CNTXT_USER "user layer"

typedef c_voidp u_domainActionArg;
typedef void (*u_domainAction)(u_domain domain, u_domainActionArg arg);

#define u_resultFromKernel(r) ((u_result)r)

u_result
u_userAddDomain(
    u_domain domain);

u_domain
u_userLookupDomain(
    const c_char *uri);

c_long
u_userRemoveDomain(
    u_domain domain);

c_address
u_userServer (
    c_long id);

c_long
u_userServerId (
    v_public o);

c_long
u_userProtectCount ();

#endif

