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

#include "v_cfNode.h"
#include "v_kernel.h"
#include "v_configuration.h"
#include "os_report.h"

#include "v_cfElement.h"
#include "v_cfAttribute.h"
#include "v_cfData.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
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
        type = c_resolve(c_getBase(config), "kernelModule::v_cfElement");
    break;
    case V_CFATTRIBUTE:
        type = c_resolve(c_getBase(config), "kernelModule::v_cfAttribute");
    break;
    case V_CFDATA:
        type = c_resolve(c_getBase(config), "kernelModule::v_cfData");
    break;
    case V_CFNODE:
    default:
        OS_REPORT_1(OS_ERROR,
                    "v_cfNodeNew",0,
                    "Illegal kind (%d) specified",
                    kind);
        assert(FALSE); 
        type = NULL;
    break;
    }

    if (type != NULL) {
        node = c_new(type);
        if (node == NULL) {
            OS_REPORT(OS_ERROR,
                      "v_cfNodeNew",0,
                      "Failed to allocate v_cfNode object.");
            assert(FALSE);
        }
    } else {
        OS_REPORT_1(OS_ERROR,
                    "v_cfNodeNew",0,
                    "Failed to resolve v_cfNode sub-type identified by kind=%d.",
                    kind);
        assert(FALSE);
        node = NULL;
    }
    /* init is done by specific class itself, this is just a 
       convenience function! */

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

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
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
