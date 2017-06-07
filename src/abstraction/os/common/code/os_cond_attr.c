/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/** \file os/common/code/os_cond_attr.c
 *  \brief Common condition variable attributes
 *
 * Implements os_condAttrInit and sets attributes
 * to platform independent values:
 * - scope is OS_SCOPE_SHARED
 */

#include <assert.h>
#include "os_cond.h"

/** \brief Initialize condition variable attribute
 *
 * Set \b condAttr->scopeAttr to \b OS_SCOPE_PRIVATE
 */
_Post_satisfies_(condAttr->scopeAttr == OS_SCOPE_PRIVATE)
void
os_condAttrInit (
    _Out_ os_condAttr *condAttr)
{
    assert (condAttr != NULL);
    condAttr->scopeAttr = OS_SCOPE_PRIVATE;
}

