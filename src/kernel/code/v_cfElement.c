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

#include "v_cfElement.h"
#include "v_cfNode.h"
#include "v_cfAttribute.h"
#include "v_kernel.h"

#include "c_collection.h"

#define XPATH_SEPERATOR '/'

struct getAttributeArg {
    const c_char  *name;
    v_cfAttribute  attr;
};

struct getChildrenArg {
    c_char     *tagName;
    c_char    *attribName;
    c_char    *attribValue;
    c_bool     attribNegate;
    c_iter     children;
};

/**************************************************************
 * Private functions
 **************************************************************/

static c_bool
attributesToIterator(
    c_object o,
    c_voidp arg)
{
    c_iter *c = (c_iter *)arg;
    *c = c_iterInsert(*c, o);
    return TRUE;
}

static c_bool
childsToIterator(
    c_object o,
    c_voidp arg)
{
    c_iter *c = (c_iter *)arg;
    *c = c_iterInsert(*c, o);
    return TRUE;
}

static c_bool
getAttribute(
    c_object o,
    c_voidp arg)
{
    struct getAttributeArg *a = (struct getAttributeArg *)arg;
    c_bool result;

    assert(a != NULL);

    if (strcmp(v_cfNodeGetName(v_cfNode(o)), a->name) == 0) {
        a->attr = v_cfAttribute(o);
        result = FALSE; /* stop the walk */
    } else {
        result = TRUE;
    }

    return result;
}

static c_bool
getChildren(
    c_object o,
    c_voidp arg)
{
    struct getChildrenArg *a = (struct getChildrenArg *)arg;
    c_value value;
    c_bool isEqual;

    if (a->tagName == NULL) {
        a->children = c_iterInsert(a->children, o);
    } else {
        if (strcmp(v_cfNodeGetName(v_cfNode(o)), a->tagName) == 0) {
            if (a->attribName == NULL) {
                a->children = c_iterInsert(a->children, o);
            } else {
                value = v_cfElementAttributeValue(v_cfElement(o), a->attribName);
                if (value.kind == V_STRING) {
                    isEqual = (strcmp(a->attribValue, value.is.String) == 0);
                    if (isEqual != a->attribNegate) {
                        /* Add if isEqual && !a->attribNegate or
                               if !isEqual && a->attribNegate (XOR) */
                        a->children = c_iterInsert(a->children, o);
                    }
                } else {
                    assert(value.kind == V_UNDEFINED);
                    /* If this is a not-relationship,
                       add the element to the result */
                    if (a->attribNegate) {
                        a->children = c_iterInsert(a->children, o);
                    }
                }
            }
        }
    }
    return TRUE; /* always finish the walk */
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_cfElement
v_cfElementNew (
    v_configuration config,
    const c_char *tagName)
{
    v_cfElement el;

    assert(C_TYPECHECK(config, v_configuration));
    assert(tagName != NULL);

    el = v_cfElement(v_cfNodeNew(config, V_CFELEMENT));
    v_cfElementInit(el, config, tagName);

    return el;
}

void
v_cfElementInit (
    v_cfElement element,
    v_configuration config,
    const c_char *tagName)
{
    c_type attrType;
    c_type nodeType;
    const c_char *keyList;
    
    assert(C_TYPECHECK(element, v_cfElement));
    assert(tagName != NULL);

    v_cfNodeInit(v_cfNode(element), config, V_CFELEMENT, tagName);

    attrType = c_resolve(c_getBase(element), "kernelModule::v_cfAttribute");
    nodeType = c_resolve(c_getBase(element), "kernelModule::v_cfNode");
    keyList = "name";

    element->attributes = c_tableNew(attrType, keyList);
    element->children = c_setNew(nodeType);
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
v_cfAttribute
v_cfElementAddAttribute (
    v_cfElement element,
    v_cfAttribute attribute)
{
    v_cfAttribute attr;

    assert(C_TYPECHECK(element, v_cfElement));
    assert(C_TYPECHECK(attribute, v_cfAttribute));

    attr = v_cfAttribute(c_insert(element->attributes, 
                                 c_object(attribute)));

    return attr;
}

v_cfNode
v_cfElementAddChild(
    v_cfElement element,
    v_cfNode child)
{
    v_cfNode ch;

    assert(C_TYPECHECK(element, v_cfElement));
    assert(C_TYPECHECK(child, v_cfNode));

    ch = v_cfNode(c_insert(element->children, 
                          c_object(child)));

    return ch;
}

c_iter
v_cfElementGetAttributes(
    v_cfElement element)
{
    c_iter i = NULL;

    assert(C_TYPECHECK(element, v_cfElement));

    c_walk(element->attributes, attributesToIterator, &i);

    return i;
}

c_iter
v_cfElementGetChildren(
    v_cfElement element)
{
    c_iter i = NULL;

    assert(C_TYPECHECK(element, v_cfElement));

    c_walk(element->children, childsToIterator, &i);

    return i;
}

v_cfAttribute
v_cfElementAttribute(
    v_cfElement element,
    const c_char *attributeName)
{
    struct getAttributeArg arg;

    assert(C_TYPECHECK(element, v_cfElement));
    assert(attributeName != NULL);

    arg.name = attributeName;
    arg.attr = NULL;
    c_walk(element->attributes, getAttribute, &arg);

    assert(C_TYPECHECK(arg.attr, v_cfAttribute));

    return arg.attr;
}

c_value
v_cfElementAttributeValue(
    v_cfElement element,
    const c_char *attributeName)
{
    v_cfAttribute attr;
    c_value value;

    assert(C_TYPECHECK(element, v_cfElement));
    assert(attributeName != NULL);

    attr = v_cfElementAttribute(element, attributeName);
    
    if (attr != NULL) {
        value = v_cfAttributeValue(attr);
    } else {
        value = c_undefinedValue();
    }
    return value;
}

c_iter
v_cfElementXPath(
    v_cfElement element,
    const c_char *xpathExpr)
{
    c_iter result;
    const c_char *posInExpr;
    const c_char *slash;
    char *attribEnd;
    c_ulong length;
    struct getChildrenArg arg;
    c_long nrToProcess;
    v_cfNode node;

    assert(C_TYPECHECK(element, v_cfElement));
    assert(xpathExpr != NULL);
 
    result = c_iterNew(element);
    nrToProcess = 1;
    posInExpr = xpathExpr;
    slash = strchr(posInExpr, XPATH_SEPERATOR);
    while (nrToProcess > 0) {
        node = c_iterTakeFirst(result);
        nrToProcess--;
        if (node->kind == V_CFELEMENT) { /* do not process data elements */
            if (slash) {

            	length = C_ADDRESS(slash) - C_ADDRESS(posInExpr);
            } else {
                length = strlen(posInExpr);
            }
            arg.children = c_iterNew(NULL);
            arg.tagName = (c_char *)os_malloc(length + 1U);
            os_strncpy(arg.tagName, posInExpr, length);
            arg.tagName[length] = 0;
            
            /* Look for selection criteria based on attribute value
             * Example XPath expression:
             * /aaa/bbb[@name='value']/ccc or /aaa/bbb[@name!='value']/ccc */
            arg.attribName = strchr(arg.tagName, '[');
            if (arg.attribName != NULL) {
                *arg.attribName = '\0';
                arg.attribName = &(arg.attribName[1]);
                assert(*arg.attribName == '@');
                arg.attribName = &(arg.attribName[1]);
                arg.attribValue = strchr(arg.attribName, '!');
                if (arg.attribValue != NULL) {
                    arg.attribNegate = TRUE;
                    *arg.attribValue = '\0';
                    arg.attribValue = &arg.attribValue[1];
                    assert(*arg.attribValue == '=');
                } else {
                    arg.attribNegate = FALSE;
                    arg.attribValue = strchr(arg.attribName, '=');
                }
                assert(arg.attribValue != NULL);
                *arg.attribValue = '\0';
                arg.attribValue = &arg.attribValue[1];
                assert(*arg.attribValue == '\'');
                arg.attribValue = &(arg.attribValue[1]);
                attribEnd = strchr(arg.attribValue, '\'');
                assert(attribEnd != NULL);
                *attribEnd = '\0';
                assert(attribEnd[1] == ']');
            }
        
            c_walk(v_cfElement(node)->children, getChildren, &arg);
            os_free(arg.tagName);
            if (slash) { 
                nrToProcess += c_iterLength(arg.children);
            }
            /* now append */
            node = v_cfNode(c_iterTakeFirst(arg.children));
            while (node != NULL) {
                c_iterAppend(result, node);
                node = v_cfNode(c_iterTakeFirst(arg.children));
            }
            c_iterFree(arg.children);

            if (slash) {

                posInExpr = (const c_char *)(C_ADDRESS(slash) + 1U);
                slash = strchr(posInExpr, XPATH_SEPERATOR);
            }
        }
    }

    return result;
}
