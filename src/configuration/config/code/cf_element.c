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

#include "cf_element.h"
#include "cf_node.h"
#include "cf_attribute.h"
#include "cf__nodeList.h"

struct getNodeArg {
    const c_char *name;
    cf_node      node;
};

C_STRUCT(cf_element) {
    C_EXTENDS(cf_node);
    cf_nodeList attributes;
    cf_nodeList children;
};

/**************************************************************
 * Private functions
 **************************************************************/

static unsigned int
attributesToIterator(
    c_object o,
    c_iter *c)
{
    *c = c_iterInsert(*c, o);
    return 1;
}

static unsigned int
childsToIterator(
    c_object o,
    c_iter *c)
{
    *c = c_iterInsert(*c, o);
    return 1;
}

static unsigned int
getNode(
    c_object o,
    struct getNodeArg *arg)
{
    unsigned int result;
    c_long cmp;

    assert(arg != NULL);
    cmp = strcmp(cf_nodeGetName(cf_node(o)), arg->name);
    if (cmp == 0) {
        arg->node = cf_node(o);
        result = 0; /* stop the walk */
    } else {
        result = 1;
    }

    return result;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
cf_element
cf_elementNew (
    const c_char *tagName)
{
    cf_element el;

    assert(tagName != NULL);

    el = cf_element(os_malloc((os_uint32)C_SIZEOF(cf_element)));
    cf_elementInit(el, tagName);

    return el;
}

void
cf_elementInit (
    cf_element element,
    const c_char *tagName)
{
    assert(element != NULL);
    assert(tagName != NULL);

    cf_nodeInit(cf_node(element), CF_ELEMENT, tagName);

    element->attributes = cf_nodeListNew();
    element->children = cf_nodeListNew();
}

void
cf_elementFree (
    cf_element element)
{
    assert(element != NULL);

    cf_nodeFree(cf_node(element));
}

void
cf_elementDeinit (
    cf_element element)
{
    assert(element != NULL);
    
    if (element->attributes != NULL) {
        cf_nodeListClear(element->attributes);
        cf_nodeListFree(element->attributes);
        element->attributes = NULL;
    }
    if (element->children != NULL) {
        cf_nodeListClear(element->children);
        cf_nodeListFree(element->children);
        element->children = NULL;
    }

    cf_nodeDeinit(cf_node(element));
}
/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
cf_attribute
cf_elementAddAttribute (
    cf_element element,
    cf_attribute attribute)
{
    cf_attribute attr;

    assert(element != NULL);
    assert(attribute != NULL);

    attr = cf_attribute(cf_nodeListInsert(element->attributes, 
                                          cf_node(attribute)));

    return attr;
}

cf_node
cf_elementAddChild(
    cf_element element,
    cf_node child)
{
    cf_node ch;

    assert(element != NULL);
    assert(child != NULL);

    ch = cf_node(cf_nodeListInsert(element->children, 
                                   cf_node(child)));

    return ch;
}

c_iter
cf_elementGetAttributes(
    cf_element element)
{
    c_iter i = NULL;

    assert(element != NULL);

    cf_nodeListWalk(element->attributes, attributesToIterator, &i);

    return i;
}

c_iter
cf_elementGetChilds(
    cf_element element)
{
    c_iter i = NULL;

    assert(element != NULL);

    cf_nodeListWalk(element->children, childsToIterator, &i);

    return i;
}

cf_attribute
cf_elementAttribute(
    cf_element element,
    const c_char *attributeName)
{
    struct getNodeArg arg;

    assert(element != NULL);
    assert(attributeName != NULL);

    arg.name = attributeName;
    arg.node = NULL;
    cf_nodeListWalk(element->attributes, getNode, &arg);

    return cf_attribute(arg.node);
}

cf_node
cf_elementChild(
    cf_element element,
    const c_char *childName)
{
    struct getNodeArg arg;

    assert(element != NULL);
    assert(childName != NULL);

    arg.name = childName;
    arg.node = NULL;
    cf_nodeListWalk(element->children, getNode, &arg);

    return arg.node;
}
