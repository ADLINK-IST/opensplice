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

#include "v_cfData.h"
#include "v_cfNode.h"
#include "v_kernel.h"

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

c_value
v_cfDataValue(
    v_cfData data)
{
    assert(C_TYPECHECK(data, v_cfData));

    return data->value;
}
