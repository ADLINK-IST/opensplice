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
#include "sd_xmlNode.h"

#include "os_heap.h"
#include <string.h>


static void
sd_xmlNodeInit (
    sd_xmlNode node,
    sd_xmlNodeKind kind)
{
    assert(node);

    node->kind = kind;
    node->name = NULL;
}

static void
sd_xmlNodeDeinit (
    sd_xmlNode node)
{
    if ( node->name ) {
        os_free(node->name);
    }
}

sd_xmlElement
sd_xmlElementNew (
    void)
{
    sd_xmlElement node;

    node = (sd_xmlElement) os_malloc(C_SIZEOF(sd_xmlElement));

    if ( node ) {
        sd_xmlNodeInit(sd_xmlNode(node), SD_XMLNODE_KIND_ELEMENT);
        node->parent     = NULL;
        node->children   = NULL;
        node->attributes = NULL;
        node->data       = NULL;
    }

    return node;
}

void
sd_xmlElementFree (
    sd_xmlElement node)
{
    sd_xmlNode n;
    
    assert(node);
    
    if ( node->children ) {
        n = sd_xmlNode(sd_listTakeFirst(node->children));
        while ( n ) {
            sd_xmlNodeFree(n);
            n = sd_xmlNode(sd_listTakeFirst(node->children));
        }
        sd_listFree(node->children);
    }

    if ( node->attributes ) {
        n = sd_xmlNode(sd_listTakeFirst(node->attributes));
        while ( n ) {
            sd_xmlNodeFree(n);
            n = sd_xmlNode(sd_listTakeFirst(node->attributes));
        }
        sd_listFree(node->attributes);
    }

    if ( node->data ) {
        sd_xmlNodeFree(node->data);
    }
     
    sd_xmlNodeDeinit(sd_xmlNode(node));

    os_free(node);
}
    
sd_xmlAttribute
sd_xmlAttributeNew (
    void)
{
    sd_xmlAttribute node;

    node = (sd_xmlAttribute) os_malloc(C_SIZEOF(sd_xmlAttribute));

    if ( node ) {
        sd_xmlNodeInit(sd_xmlNode(node), SD_XMLNODE_KIND_ATTRIBUTE);
        node->value     = NULL;
    }

    return node;
}

void
sd_xmlAttributeFree (
    sd_xmlAttribute node)
{
    assert(node);
    
    if ( node->value ) {
        os_free(node->value);
    }

    sd_xmlNodeDeinit(sd_xmlNode(node));

    os_free(node);
}
        
sd_xmlData
sd_xmlDataNew (
    void)
{
    sd_xmlData node;

    node = (sd_xmlData) os_malloc(C_SIZEOF(sd_xmlData));

    if ( node ) {
        sd_xmlNodeInit(sd_xmlNode(node), SD_XMLNODE_KIND_DATA);
        node->data     = NULL;
    }

    return node;
}

void
sd_xmlDataFree (
    sd_xmlData node)
{
    assert(node);
    
    if ( node->data ) {
        os_free(node->data);
    }

    sd_xmlNodeDeinit(sd_xmlNode(node));

    os_free(node);
}



sd_xmlNode
sd_xmlNodeNew (
    sd_xmlNodeKind kind)
{
    sd_xmlNode node = NULL;

    switch ( kind ) {
        case SD_XMLNODE_KIND_ELEMENT:
            node = sd_xmlNode(sd_xmlElementNew());
            break;
        case SD_XMLNODE_KIND_ATTRIBUTE:
            node = sd_xmlNode(sd_xmlAttributeNew());
            break;
        case SD_XMLNODE_KIND_DATA:
            node = sd_xmlNode(sd_xmlDataNew());
            break;
        default:
            assert(0);
            break;
    }

    return node;
}

void
sd_xmlNodeFree (
    sd_xmlNode node)
{
    if ( node ) {
        switch ( node->kind ) {
            case SD_XMLNODE_KIND_ELEMENT:
                sd_xmlElementFree(sd_xmlElement(node));
                break;
            case SD_XMLNODE_KIND_ATTRIBUTE:
                sd_xmlAttributeFree(sd_xmlAttribute(node));
                break;
            case SD_XMLNODE_KIND_DATA:
                sd_xmlDataFree(sd_xmlData(node));
                break;
            default:
                assert(0);
                break;
        }
    }
}

void
sd_xmlElementAdd (
    sd_xmlElement element,
    sd_xmlNode    child)
{
    assert(element);
    assert(child);

    switch ( child->kind ) {
        case SD_XMLNODE_KIND_ELEMENT:
            if ( !element->children ) {
                element->children = sd_listNew();
            }
            if ( element->children ) {
                sd_listAppend(element->children, child);
                sd_xmlElement(child)->parent = element;
            }
            break;
        case SD_XMLNODE_KIND_ATTRIBUTE:
            if ( !element->attributes ) {
                element->attributes = sd_listNew();
            }
            if ( element->attributes ) {
                sd_listAppend(element->attributes, child);
            }
            break;
        case SD_XMLNODE_KIND_DATA:
            assert(!element->data);
            element->data = child;
            break;
        default:
            assert(0);
            break;
    }
}

    
sd_xmlElement
sd_xmlElementGetParent (
    sd_xmlElement node)
{
    assert(node);

    return node->parent;
}

sd_list
sd_xmlElementGetChildren (
    sd_xmlElement node)
{
    assert(node);

    return node->children;
}

sd_list
sd_xmlElementGetAttributes (
    sd_xmlElement node)
{
    assert(node);

    return node->attributes;
}

sd_xmlData
sd_xmlElementGetData (
    sd_xmlElement node)
{
    assert(node);

    return sd_xmlData(node->data);
}

void
sd_xmlElementWalkChildren (
    sd_xmlElement node,
    sd_xmlNodeWalkAction action,
    void *arg)
{
    assert(node);

    if ( node->children ) {
        sd_listWalk(node->children, (sd_listAction)action, arg);
    }
}

void
sd_xmlElementWalkAttributes (
    sd_xmlElement node,
    sd_xmlNodeWalkAction action,
    void *arg)
{
    assert(node);

    if ( node->attributes ) {
        sd_listWalk(node->attributes, (sd_listAction)action, arg);
    }
}

typedef struct {
    const c_char *name;
    sd_xmlNode   node;
} sd_xmlFindNodeArg;


static c_bool
sd_xmlFindNodeAction (
    sd_xmlNode node,
    void *arg)
{
    sd_xmlFindNodeArg *argument = (sd_xmlFindNodeArg *) arg;
    c_bool result = TRUE;

    assert(node->name);
    
    if ( strcmp(node->name, argument->name) == 0 ) {
        argument->node = node;
        result = FALSE;
    }

    return result;
}

sd_xmlAttribute
sd_xmlElementFindAttribute (
    sd_xmlElement node,
    const c_char *name)
{
    sd_xmlFindNodeArg arg;

    arg.name = name;
    arg.node = NULL;

    sd_xmlElementWalkAttributes(node, sd_xmlFindNodeAction, &arg);

    return sd_xmlAttribute(arg.node);
}

c_bool
sd_xmlElementHasChildren (
    sd_xmlElement node)
{
    c_bool result = FALSE;

    if ( node->children ) {
        if ( !sd_listIsEmpty(node->children) ) {
            result = TRUE;
        }
    }
    
    return result;
}

c_bool
sd_xmlElementHasAttributes (
    sd_xmlElement node)
{
    c_bool result = FALSE;

    if ( node->attributes ) {
        if ( !sd_listIsEmpty(node->attributes) ) {
            result = TRUE;
        }
    }
    
    return result;
}

c_ulong
sd_xmlElementNumAttributes (
    sd_xmlElement node)
{
    c_ulong size = 0;
    
    assert(node);

    if ( node->attributes ) {
        size = sd_listSize(node->attributes);
    }
    return size;
}
    
c_bool
sd_xmlNodeEqualName (
    sd_xmlNode n1,
    sd_xmlNode n2)
{
    c_bool result = FALSE;

    if ( n1->name && n2->name ) {
        if ( strcmp(n1->name, n2->name) == 0 ) {
            result = TRUE;
        }
    }
    return result;
}

