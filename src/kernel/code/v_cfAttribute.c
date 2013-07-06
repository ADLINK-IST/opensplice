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

#include "v_cfAttribute.h"
#include "v_cfNode.h"
#include "v_kernel.h"

#include "os_report.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_cfAttribute
v_cfAttributeNew (
    v_configuration config,
    const c_char *name,
    c_value value)
{
    v_cfAttribute attr;

    assert(C_TYPECHECK(config, v_configuration));
    assert(name != NULL);

    if (value.kind != V_UNDEFINED) {
        attr = v_cfAttribute(v_cfNodeNew(config, V_CFATTRIBUTE));
        v_cfAttributeInit(attr, config, name, value);
    } else {
        attr = NULL;
    }

    return attr;
}

void
v_cfAttributeInit (
    v_cfAttribute attribute,
    v_configuration config,
    const c_char *name,
    c_value value)
{
    assert(C_TYPECHECK(attribute, v_cfAttribute));
    assert(name != NULL);

    v_cfNodeInit(v_cfNode(attribute), config, V_CFATTRIBUTE, name);

    attribute->value = value;
    switch (value.kind) {
    case V_STRING:
        attribute->value.is.String = c_stringNew(c_getBase(c_object(config)),
                                                 value.is.String);
    break;
    case V_UNDEFINED:
    case V_BOOLEAN: case V_OCTET:
    case V_SHORT:   case V_LONG:   case V_LONGLONG:
    case V_USHORT:  case V_ULONG:  case V_ULONGLONG:
    case V_FLOAT:   case V_DOUBLE:
    case V_CHAR:    
    case V_WCHAR:   case V_WSTRING:
    case V_FIXED:   case V_OBJECT:
    default:
        /* nothing to copy */
        OS_REPORT_1(OS_ERROR,
                    "kernel", 0,
                    "Unknown value (%d) type given at creation of "
                    "configuration attribute.",
                     value.kind);
    break;
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
c_value
v_cfAttributeValue(
    v_cfAttribute attribute)
{
    assert(C_TYPECHECK(attribute, v_cfAttribute));

    return attribute->value;
}
