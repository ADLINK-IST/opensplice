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

#ifndef V__GROUPENTRY_H
#define V__GROUPENTRY_H

#include "v_kernel.h"

c_bool
v_groupEntryApplyUnregisterMessage(
    v_groupEntry _this,
    v_message unregisterMsg,
    v_registration registration);

#endif /* V__GROUPENTRY_H */
