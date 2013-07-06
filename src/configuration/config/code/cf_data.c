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

#include "cf_data.h"
#include "cf_node.h"
/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
cf_data
cf_dataNew (
    c_value value)
{
    cf_data d;

    assert(value.kind != V_UNDEFINED);

    d = cf_data(os_malloc((os_uint32)C_SIZEOF(cf_data)));
    cf_dataInit(d, value);

    return d;
}

void
cf_dataInit (
    cf_data data,
    c_value value)
{
    assert(data != NULL);
    assert(value.kind != V_UNDEFINED);
    
    cf_nodeInit(cf_node(data), CF_DATA, CF_DATANAME);

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
        data->value.is = data->value.is;
    break;
    case V_STRING:
        data->value.is.String = os_strdup(value.is.String);
    break;
    case V_WCHAR:
    case V_WSTRING:
    case V_FIXED:
    case V_OBJECT:
    case V_UNDEFINED:
    case V_COUNT:
    default:
        data->value.kind = V_UNDEFINED;
        assert(0); /* catch undefined value */        
    break;
    }
}

void
cf_dataDeinit (
    cf_data data)
{
    assert(data != NULL);

    switch (data->value.kind) {
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
      /* nothing to free */;
    break;
    case V_STRING:
        os_free(data->value.is.String);
    break;
    case V_WCHAR:
    case V_WSTRING:
    case V_FIXED:
    case V_OBJECT:
    case V_UNDEFINED:
    case V_COUNT:
    default:
        assert(0); /* catch undefined behaviour */
    break;
    }
    data->value.kind = V_UNDEFINED;

    cf_nodeDeinit(cf_node(data));
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
c_value
cf_dataValue(
    cf_data data)
{
    assert(data != NULL);

    return data->value;
}
