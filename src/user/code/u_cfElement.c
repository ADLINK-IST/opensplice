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

#include "os_report.h"

#include "u_user.h"
#include "u__cfElement.h"
#include "u__cfAttribute.h"
#include "u__cfData.h"
#include "u__cfValue.h"
#include "u__entity.h"
#include "v_cfNode.h"

#define U_CFELEMENT_SIZE (sizeof(C_STRUCT(u_cfElement)))

u_cfElement
u_cfElementNew(
    const u_participant participant,
    v_cfElement kElement)
{
    u_cfElement _this = NULL;

    assert(participant != NULL);
    assert(kElement);

    _this = u_cfElement(os_malloc(U_CFELEMENT_SIZE));
    if (_this)
    {
        u_cfNodeInit(u_cfNode(_this),participant,v_cfNode(kElement));
    }

    return _this;
}

void
u_cfElementFree(
    u_cfElement _this)
{
    assert(_this != NULL);

    u_cfNodeDeinit(u_cfNode(_this));
    memset(_this, 0, sizeof(U_CFELEMENT_SIZE));
    os_free(_this);
}

c_iter
u_cfElementGetChildren(
    const u_cfElement element)
{
    u_result      r;
    v_cfElement   ke;
    v_cfNode      child;
    u_cfNode      proxy;
    c_iter        kc;
    c_iter        children;
    u_participant p;

    assert(element != NULL);

    children = c_iterNew(NULL);
    r = u_cfNodeReadClaim(u_cfNode(element), (v_cfNode*)(&ke));
    if (r == U_RESULT_OK) {
        p = u_cfNodeParticipant(u_cfNode(element));
        kc = v_cfElementGetChildren(ke);
        child = c_iterTakeFirst(kc);
        while (child != NULL) {
            switch(v_cfNodeKind(child)) {
            case V_CFELEMENT:
                proxy = u_cfNode(u_cfElementNew(p, v_cfElement(child)));
                break;
            case V_CFATTRIBUTE:
                proxy = u_cfNode(u_cfAttributeNew(p, v_cfAttribute(child)));
                break;
            case V_CFDATA:
                proxy = u_cfNode(u_cfDataNew(p,v_cfData(child)));
                break;
            default:
                proxy = NULL;
                break;
            }
            c_iterInsert(children, proxy);
            child = c_iterTakeFirst(kc);
        }
        c_iterFree(kc);
        u_cfNodeRelease(u_cfNode(element));
    }
    return children;
}

c_iter
u_cfElementGetAttributes(
    const u_cfElement element)
{
    u_result      r;
    v_cfElement   ke;
    v_cfAttribute attr;
    c_iter        ka;
    c_iter        attributes;
    u_participant p;

    assert(element != NULL);

    attributes = c_iterNew(NULL);
    r = u_cfNodeReadClaim(u_cfNode(element), (v_cfNode*)(&ke));
    if (r == U_RESULT_OK) {
        p = u_cfNodeParticipant(u_cfNode(element));
        ka = v_cfElementGetAttributes(ke);
        attr = c_iterTakeFirst(ka);
        while (attr != NULL) {
            c_iterInsert(attributes, u_cfAttributeNew(p, attr));
            attr = c_iterTakeFirst(ka);
        }
        c_iterFree(ka);
        u_cfNodeRelease(u_cfNode(element));
    }
    return attributes;
}

u_cfAttribute
u_cfElementAttribute(
    const u_cfElement element,
    const os_char *name)
{
    u_result      r;
    v_cfElement   ke;
    v_cfAttribute attr;
    u_cfAttribute attribute;
    u_participant p;

    assert(element != NULL);
    assert(name != NULL);

    attribute = NULL;
    r = u_cfNodeReadClaim(u_cfNode(element), (v_cfNode*)(&ke));
    if (r == U_RESULT_OK) {
        p = u_cfNodeParticipant(u_cfNode(element));
        attr = v_cfElementAttribute(ke, name);

        if (attr) {
            attribute = u_cfAttributeNew(p, attr);
        }
        u_cfNodeRelease(u_cfNode(element));
    }
    return attribute;
}

u_bool
u_cfElementAttributeStringValue(
    const u_cfElement element,
    const os_char *attributeName,
    os_char **str)
{
    u_result r;
    u_bool result;
    v_cfElement ke;
    c_value value;
    c_value resultValue;

    assert(element != NULL);
    assert(str != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(element), (v_cfNode*)(&ke));
    if (r == U_RESULT_OK) {
        value = v_cfElementAttributeValue(ke, attributeName);
        result = u_cfValueScan(value, V_STRING, &resultValue);

        if (result == TRUE) {
            *str = resultValue.is.String;
        }
        u_cfNodeRelease(u_cfNode(element));
    }
    return result;
}

u_bool
u_cfElementAttributeBoolValue(
    const u_cfElement element,
    const os_char *attributeName,
    u_bool *b)
{
    u_result r;
    u_bool result;
    v_cfElement ke;
    c_value value;
    c_value resultValue;

    assert(element != NULL);
    assert(b != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(element), (v_cfNode*)(&ke));
    if (r == U_RESULT_OK) {
        value = v_cfElementAttributeValue(ke, attributeName);
        result = u_cfValueScan(value, V_BOOLEAN, &resultValue);

        if (result == TRUE) {
            *b = resultValue.is.Boolean;
        }
        u_cfNodeRelease(u_cfNode(element));
    }
    return result;
}

u_bool
u_cfElementAttributeLongValue(
    const u_cfElement element,
    const os_char *attributeName,
    os_int32 *lv)
{
    u_result r;
    u_bool result;
    v_cfElement ke;
    c_value value;
    c_value resultValue;

    assert(element != NULL);
    assert(lv != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(element), (v_cfNode*)(&ke));
    if (r == U_RESULT_OK) {
        value = v_cfElementAttributeValue(ke, attributeName);
        result = u_cfValueScan(value, V_LONG, &resultValue);

        if (result == TRUE) {
            *lv = resultValue.is.Long;
        }
        u_cfNodeRelease(u_cfNode(element));
    }
    return result;
}

u_bool
u_cfElementAttributeULongValue(
    const u_cfElement element,
    const os_char *attributeName,
    os_uint32 *ul)
{
    u_result r;
    u_bool result;
    v_cfElement ke;
    c_value value;
    c_value resultValue;

    assert(element != NULL);
    assert(ul != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(element), (v_cfNode*)(&ke));
    if (r == U_RESULT_OK) {
        value = v_cfElementAttributeValue(ke, attributeName);
        result = u_cfValueScan(value, V_ULONG, &resultValue);

        if (result == TRUE) {
            *ul = resultValue.is.ULong;
        }
        u_cfNodeRelease(u_cfNode(element));
    }
    return result;
}

u_bool
u_cfElementAttributeSizeValue(
    const u_cfElement element,
    const os_char *attributeName,
    u_size *size)
{
    u_result r;
    u_bool result;
    v_cfElement ke;
    c_value value;

    assert(element != NULL);
    assert(size != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(element), (v_cfNode*)(&ke));
    if (r == U_RESULT_OK) {
        value = v_cfElementAttributeValue(ke, attributeName);

        if (value.kind == V_STRING){
           result = u_cfDataSizeValueFromString(value.is.String,size);
        } else {
            OS_REPORT(OS_ERROR, "u_cfElementAttributeSizeValue", 0, "Value is not a string");
            assert(value.kind == V_STRING);
        }

        u_cfNodeRelease(u_cfNode(element));
    }
    return result;
}

u_bool
u_cfElementAttributeFloatValue(
    const u_cfElement element,
    const os_char *attributeName,
    os_float *f)
{
    u_result r;
    u_bool result;
    v_cfElement ke;
    c_value value;
    c_value resultValue;

    assert(element != NULL);
    assert(f != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(element), (v_cfNode*)(&ke));
    if (r == U_RESULT_OK) {
        value = v_cfElementAttributeValue(ke, attributeName);
        result = u_cfValueScan(value, V_FLOAT, &resultValue);

        if (result == TRUE) {
            *f = resultValue.is.Float;
        }
        u_cfNodeRelease(u_cfNode(element));
    }
    return result;
}

c_iter
u_cfElementXPath(
    const u_cfElement element,
    const os_char *xpathExpr)
{
    u_result      r;
    v_cfElement   ke;
    v_cfNode      vChild;
    c_iter        vChildren;
    c_iter        children;
    u_participant p;
    u_cfNode      proxy;

    assert(element != NULL);
    assert(xpathExpr != NULL);

    children = c_iterNew(NULL);
    r = u_cfNodeReadClaim(u_cfNode(element), (v_cfNode*)(&ke));
    if (r == U_RESULT_OK) {
        p = u_cfNodeParticipant(u_cfNode(element));
        vChildren = v_cfElementXPath(ke, xpathExpr);
        vChild = c_iterTakeFirst(vChildren);
        while (vChild != NULL) {
            switch(v_cfNodeKind(vChild)) {
                case V_CFELEMENT:
                    proxy = u_cfNode(u_cfElementNew(p, v_cfElement(vChild)));
                    break;
                case V_CFATTRIBUTE:
                    proxy = u_cfNode(u_cfAttributeNew(p, v_cfAttribute(vChild)));
                    break;
                case V_CFDATA:
                    proxy = u_cfNode(u_cfDataNew(p,v_cfData(vChild)));
                    break;
                default:
                    proxy = NULL;
                    break;
            }
            children = c_iterInsert(children, proxy);
            vChild = c_iterTakeFirst(vChildren);
        }
        c_iterFree(vChildren);
        u_cfNodeRelease(u_cfNode(element));
    }
    return children;
}
