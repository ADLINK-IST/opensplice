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

#ifndef V_GROUPINSTANCE_H
#define V_GROUPINSTANCE_H

#include "v_kernel.h"
#include "v_group.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API v_message
v_groupInstanceCreateMessage(
    v_groupInstance _this);

OS_API v_message
v_groupInstanceCreateTypedInvalidMessage(
    v_groupInstance _this,
    v_message untypedMsg);

OS_API v_registration
v_groupInstanceGetRegistration(
    v_groupInstance instance,
    v_gid gidTemplate,
    v_matchIdentityAction predicate);

#undef OS_API

#endif
