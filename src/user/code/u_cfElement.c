
#include "os_report.h"

#include "u_user.h"
#include "u__cfElement.h"
#include "u__cfAttribute.h"
#include "u__cfData.h"
#include "u__cfValue.h"

#include "v_cfNode.h"

#define U_CFELEMENT_SIZE (sizeof(C_STRUCT(u_cfElement)))

u_result
u_cfElementClaim(
    u_cfElement _this,
    v_cfElement *ke)
{
    u_result r = U_RESULT_OK;

    if ((_this == NULL) || (ke == NULL)) {
        OS_REPORT(OS_ERROR, "u_cfElementClaim", 0, "Illegal parameter");
        r = U_RESULT_ILL_PARAM;
    } else {
        *ke = v_cfElement(u_cfNodeClaim(u_cfNode(_this)));
        if (*ke == NULL) {
            r = U_RESULT_INTERNAL_ERROR;
        }
    }
    return r;
}

u_result
u_cfElementRelease(
    u_cfElement _this)
{
    u_result r = U_RESULT_OK;

    if (_this == NULL) {
        OS_REPORT(OS_ERROR, "u_cfElementRelease", 0, "Illegal parameter");
        r = U_RESULT_ILL_PARAM;
    } else {
        u_cfNodeRelease(u_cfNode(_this));
    }
    return r;
}

u_cfElement
u_cfElementNew(
    u_participant participant,
    v_cfElement kElement)
{
    u_cfElement _this = NULL;

    if ((participant == NULL) || (kElement == NULL)) {
        OS_REPORT(OS_ERROR, "u_cfElementNew", 0, "Illegal parameter");
    } else {
        _this = u_cfElement(os_malloc(U_CFELEMENT_SIZE));
        u_cfNodeInit(u_cfNode(_this),participant,v_cfNode(kElement));
    }
    return _this;
}

void
u_cfElementFree(
    u_cfElement _this)
{
    if (_this != NULL) {
        u_cfNodeDeinit(u_cfNode(_this));
        memset(_this, 0, (size_t)sizeof(U_CFELEMENT_SIZE));
        os_free(_this);
    }
}

c_iter
u_cfElementGetChildren(
    u_cfElement element)
{
    u_result      r;
    v_cfElement   ke;
    v_cfNode      child;
    u_cfNode      proxy;
    c_iter        kc;
    c_iter        children;
    u_participant p;
    
    children = c_iterNew(NULL);
    if (element != NULL) {
        r = u_cfElementClaim(element, &ke);
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
            u_cfElementRelease(element);
        }
    }
    return children;
}

c_iter
u_cfElementGetAttributes(
    u_cfElement element)
{
    u_result      r;
    v_cfElement   ke;
    v_cfAttribute attr;
    c_iter        ka;
    c_iter        attributes;
    u_participant p;
    

    attributes = c_iterNew(NULL);
    if (element != NULL) {
        r = u_cfElementClaim(element, &ke);
        if (r == U_RESULT_OK) {
            p = u_cfNodeParticipant(u_cfNode(element));
            ka = v_cfElementGetAttributes(ke);
            attr = c_iterTakeFirst(ka);
            while (attr != NULL) {
                c_iterInsert(attributes, u_cfAttributeNew(p, attr));
                attr = c_iterTakeFirst(ka);
            }
            c_iterFree(ka);
            u_cfElementRelease(element);
        }
    }
    return attributes;
}

u_cfAttribute
u_cfElementAttribute(
    u_cfElement element,
    const c_char *name)
{
    u_result      r;
    v_cfElement   ke;
    v_cfAttribute attr;
    u_cfAttribute attribute;
    u_participant p;
    
    attribute = NULL;
    if (element != NULL) {
        r = u_cfElementClaim(element, &ke);
        if (r == U_RESULT_OK) {
            p = u_cfNodeParticipant(u_cfNode(element));
            attr = v_cfElementAttribute(ke, name);
            
            if(attr){
                attribute = u_cfAttributeNew(p, attr);
            }
            u_cfElementRelease(element);
        }
    }
    return attribute;
}

c_bool
u_cfElementAttributeStringValue(
    u_cfElement element,
    const c_char *attributeName,
    c_char **str)
{
    u_result r;
    c_bool result;
    v_cfElement ke;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((element != NULL) && (str != NULL)) {
        r = u_cfElementClaim(element, &ke);
        if (r == U_RESULT_OK) {
            value = v_cfElementAttributeValue(ke, attributeName);
            result = u_cfValueScan(value, V_STRING, &resultValue);

            if (result == TRUE) {
                *str = resultValue.is.String;
            }
            u_cfElementRelease(element);
        }
    }
    return result;
}

c_bool
u_cfElementAttributeBoolValue(
    u_cfElement element,
    const c_char *attributeName,
    c_bool *b)
{
    u_result r;
    c_bool result;
    v_cfElement ke;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((element != NULL) && (b != NULL)) {
        r = u_cfElementClaim(element, &ke);
        if (r == U_RESULT_OK) {
            value = v_cfElementAttributeValue(ke, attributeName);
            result = u_cfValueScan(value, V_BOOLEAN, &resultValue);

            if (result == TRUE) {
                *b = resultValue.is.Boolean;
            }
            u_cfElementRelease(element);
        }
    }
    return result;
}

c_bool
u_cfElementAttributeLongValue(
    u_cfElement element,
    const c_char *attributeName,
    c_long *lv)
{
    u_result r;
    c_bool result;
    v_cfElement ke;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((element != NULL) && (lv != NULL)) {
        r = u_cfElementClaim(element, &ke);
        if (r == U_RESULT_OK) {
            value = v_cfElementAttributeValue(ke, attributeName);
            result = u_cfValueScan(value, V_LONG, &resultValue);

            if (result == TRUE) {
                *lv = resultValue.is.Long;
            }
            u_cfElementRelease(element);
        }
    }
    return result;
}

c_bool
u_cfElementAttributeULongValue(
    u_cfElement element,
    const c_char *attributeName,
    c_ulong *ul)
{
    u_result r;
    c_bool result;
    v_cfElement ke;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((element != NULL) && (ul != NULL)) {
        r = u_cfElementClaim(element, &ke);
        if (r == U_RESULT_OK) {
            value = v_cfElementAttributeValue(ke, attributeName);
            result = u_cfValueScan(value, V_ULONG, &resultValue);

            if (result == TRUE) {
                *ul = resultValue.is.ULong;
            }
            u_cfElementRelease(element);
        }
    }
    return result;
}

c_bool
u_cfElementAttributeFloatValue(
    u_cfElement element,
    const c_char *attributeName,
    c_float *f)
{
    u_result r;
    c_bool result;
    v_cfElement ke;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((element != NULL) && (f != NULL)) {
        r = u_cfElementClaim(element, &ke);
        if (r == U_RESULT_OK) {
            value = v_cfElementAttributeValue(ke, attributeName);
            result = u_cfValueScan(value, V_FLOAT, &resultValue);

            if (result == TRUE) {
                *f = resultValue.is.Float;
            }
            u_cfElementRelease(element);
        }
    }
    return result;
}

c_iter
u_cfElementXPath(
    u_cfElement element,
    const c_char *xpathExpr)
{
    u_result      r;
    v_cfElement   ke;
    v_cfNode      vChild;
    c_iter        vChildren;
    c_iter        children;
    u_participant p;
    u_cfNode      proxy;

    children = c_iterNew(NULL);
    if ((element != NULL) && (xpathExpr != NULL)) {
        r = u_cfElementClaim(element, &ke);
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
            u_cfElementRelease(element);
        }
    }
    return children;
}
