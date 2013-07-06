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

#include "os.h"

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
