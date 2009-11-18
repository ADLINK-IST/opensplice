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

#ifndef V__DOMAIN_H
#define V__DOMAIN_H

#include "v_domain.h"

#define v_domainInterest(o) (C_CAST(o,v_domainInterest))

c_bool
v_domainExpressionIsAbsolute(
    const c_char* expression);

c_bool
v_domainStringMatchesExpression(
    const c_char* string,
    const c_char* expression);

v_domainInterest
v_domainInterestNew (
    v_kernel kernel,
    const char *domainExpression);

#endif
