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

#include "v_cfData.h"
#include "v_cfNode.h"
#include "v_kernel.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_cfData
v_cfDataNew (
    v_configuration config,
    c_value value)
{
    v_cfData data;

    assert(C_TYPECHECK(config, v_configuration));
    assert(value.kind != V_UNDEFINED);

    data = v_cfData(v_cfNodeNew(config, V_CFDATA));
    v_cfDataInit(data, config, value);

    if (data->value.kind == V_UNDEFINED) {
        c_free(data);
        data = NULL;
    }

    return data;
}

void
v_cfDataInit (
    v_cfData data,
    v_configuration config,
    c_value value)
{
    assert(C_TYPECHECK(data, v_cfData));
    assert(value.kind != V_UNDEFINED);

    v_cfNodeInit(v_cfNode(data), config, V_CFDATA, V_CFDATANAME);

    data->value.kind = value.kind;
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
        data->value.is = value.is;
    break;
    case V_STRING:
        data->value.is.String = c_stringNew(c_getBase(data), value.is.String);
    break;
    case V_WCHAR:
    case V_WSTRING:
    case V_FIXED:
    case V_OBJECT:
    case V_UNDEFINED:
    case V_COUNT:
    default:
        data->value.kind = V_UNDEFINED;
        assert(0); /* not supported! */

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
v_cfDataValue(
    v_cfData data)
{
    assert(C_TYPECHECK(data, v_cfData));

    return data->value;
}
