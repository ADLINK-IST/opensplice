/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include "v_cfNode.h"
#include "v_kernel.h"
#include "v_configuration.h"
#include "os_report.h"

#include "v_cfElement.h"
#include "v_cfAttribute.h"
#include "v_cfData.h"

v_cfNode
v_cfNodeNew(
    v_configuration config,
    v_cfKind kind)
{
    v_cfNode node;
    c_type type;

    assert(C_TYPECHECK(config, v_configuration));

    switch (kind) {
    case V_CFELEMENT:
        type = c_resolve(c_getBase(config), "kernelModuleI::v_cfElement");
    break;
    case V_CFATTRIBUTE:
        type = c_resolve(c_getBase(config), "kernelModuleI::v_cfAttribute");
    break;
    case V_CFDATA:
        type = c_resolve(c_getBase(config), "kernelModuleI::v_cfData");
    break;
    case V_CFNODE:
    default:
        OS_REPORT(
            OS_ERROR, OS_FUNCTION, V_RESULT_ILL_PARAM,
            "Illegal kind (%d) specified",
            kind);
        assert(FALSE);
        type = NULL;
    break;
    }

    if (type != NULL) {
        node = c_new(type);
    } else {
        OS_REPORT(
            OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
            "Failed to resolve v_cfNode sub-type identified by kind=%d.",
            kind);
        assert(FALSE);
        node = NULL;
    }
    /* init is done by specific class itself, this is just a
     * convenience function!
     */

    return node;
}

void
v_cfNodeInit (
    v_cfNode node,
    v_configuration config,
    v_cfKind kind,
    const c_char *name)
{
    assert(C_TYPECHECK(node, v_cfNode));
    assert(name != NULL);

    node->kind = kind;
    node->name = c_stringNew(c_getBase(node), name);
    node->id = v_configurationIdNew(config);
    node->configuration = config;
}

const c_char *
v_cfNodeGetName (
    v_cfNode node)
{
    assert(node != NULL);
    assert(C_TYPECHECK(node, v_cfNode));

    return (const c_char *)node->name;
}

v_cfKind
v_cfNodeKind(
    v_cfNode node)
{
    assert(node != NULL);
    assert(C_TYPECHECK(node, v_cfNode));

    return node->kind;
}

v_configuration
v_cfNodeConfiguration(
    v_cfNode node)
{
    assert(node != NULL);
    assert(C_TYPECHECK(node, v_cfNode));

    return node->configuration;
}
