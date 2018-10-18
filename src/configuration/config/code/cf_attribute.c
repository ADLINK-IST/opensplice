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
