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

#include "cf_attribute.h"
#include "cf_node.h"
/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
cf_attribute
cf_attributeNew (
    const c_char *name,
    c_value value)
{
    cf_attribute attr;

    assert(name != NULL);

    attr = cf_attribute(os_malloc((os_uint32)C_SIZEOF(cf_attribute)));
    cf_attributeInit(attr, name, value);

    return attr;
}

void
cf_attributeInit (
    cf_attribute attribute,
    const c_char *name,
    c_value value)
{
    assert(attribute != NULL);
    assert(name != NULL);

    cf_nodeInit(cf_node(attribute), CF_ATTRIBUTE, name);
    attribute->value.kind = value.kind;
    switch (value.kind) {
    case V_BOOLEAN:
    case V_OCTET:
    case V_SHORT:
    case V_LONG:
    case V_LONGLONG:
    case V_USHORT:
    case V_ULONG:
    case V_ULONGLONG:
    case V_FLOAT:
    case V_DOUBLE:
    case V_CHAR:
        attribute->value.is = value.is;
    break;
    case V_STRING:
        attribute->value.is.String = os_strdup(value.is.String);
    break;
    case V_WCHAR:
    case V_WSTRING:
    case V_FIXED:
    case V_OBJECT:
    case V_UNDEFINED:
    case V_COUNT:
    default:
        attribute->value.kind = V_UNDEFINED;
        assert(0); /* catch undefined attribute */
    break;
    }
}

void
cf_attributeDeinit (
    cf_attribute attribute)
{
    assert(attribute != NULL);

    switch (attribute->value.kind) {
    case V_BOOLEAN:
    case V_OCTET:
    case V_SHORT:
    case V_LONG:
    case V_LONGLONG:
    case V_USHORT:
    case V_ULONG:
    case V_ULONGLONG:
    case V_FLOAT:
    case V_DOUBLE:
    case V_CHAR:
    break;
    case V_STRING:
        os_free(attribute->value.is.String);
    break;
    case V_WCHAR:
    case V_WSTRING:
    case V_FIXED:
    case V_OBJECT:
    case V_UNDEFINED:
    case V_COUNT:
    default:
        assert(0); /* catch undefined attribute */
    break;
    }

    cf_nodeDeinit(cf_node(attribute));
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
c_value
cf_attributeValue(
    cf_attribute attribute)
{
    assert(attribute != NULL);

    return attribute->value;
}
