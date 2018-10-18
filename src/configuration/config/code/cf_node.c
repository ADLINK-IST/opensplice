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

#include "vortex_os.h"

#include "cf_node.h"

#include "cf_element.h"
#include "cf_attribute.h"
#include "cf_data.h"
/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
void
cf_nodeInit (
    cf_node node,
    cf_kind kind,
    const c_char *name)
{
    assert(node != NULL);
    assert(name != NULL);

    node->kind = kind;
    node->name = os_strdup(name);
}

void
cf_nodeFree (
    cf_node node)
{
    assert(node != NULL);

    if (node != NULL) {
        switch (node->kind) {
        case CF_ELEMENT: cf_elementDeinit(cf_element(node)); break;
        case CF_DATA: cf_dataDeinit(cf_data(node)); break;
        case CF_ATTRIBUTE: cf_attributeDeinit(cf_attribute(node)); break;
        case CF_NODE: /* abstract */
        default:
            assert (FALSE); /* catch undefined behaviour */
            break;
        }
        os_free(node);
    }
}

void
cf_nodeDeinit (
    cf_node node)
{
    assert(node != NULL);

    os_free(node->name);
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
const c_char *
cf_nodeGetName (
    cf_node node)
{
    assert(node != NULL);

    return (const c_char *)node->name;
}

cf_kind
cf_nodeKind (
    cf_node node)
{
    assert (node);

    return node->kind;
}
