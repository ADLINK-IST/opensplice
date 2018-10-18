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
#include "sd__contextItem.h"

#include "os_heap.h"

static void
sd_contextItemInit (
    sd_contextItem item,
    sd_contextItemKind kind)
{
    assert(item);

    item->kind         = kind;
    item->name         = NULL;
    item->self         = NULL;
    item->scope        = NULL;
    item->parent       = NULL;
    item->children     = NULL;
    item->refcount     = 1;

}

static void
sd_contextItemScopeInit (
    sd_contextItemScope item,
    sd_contextItemKind  kind)
{
    assert(item);

    sd_contextItemInit(sd_contextItem(item), kind);

}

sd_contextItem
sd_contextItemKeep (
    sd_contextItem item)
{
    item->refcount++;
    return item;
}

static sd_contextItem
sd_contextItemScopeNew (
    void)
{
    sd_contextItemScope item = os_malloc(C_SIZEOF(sd_contextItemScope));
    sd_contextItemScopeInit(item, SD_CONTEXT_ITEM_SCOPE);
    return sd_contextItem(item);
}


static sd_contextItem
sd_contextItemModuleNew (
    void)
{
    sd_contextItemModule item = os_malloc(C_SIZEOF(sd_contextItemModule));
    sd_contextItemScopeInit(sd_contextItemScope(item), SD_CONTEXT_ITEM_MODULE);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemStructureNew (
    void)
{
    sd_contextItemStructure item = os_malloc(C_SIZEOF(sd_contextItemStructure));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_STRUCTURE);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemMemberNew (
    void)
{
    sd_contextItemMember item = os_malloc(C_SIZEOF(sd_contextItemMember));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_MEMBER);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemTypeNew (
    void)
{
    sd_contextItemType item = os_malloc(C_SIZEOF(sd_contextItemType));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_TYPE);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemTypedefNew (
    void)
{
    sd_contextItemTypedef item = os_malloc(C_SIZEOF(sd_contextItemTypedef));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_TYPEDEF);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemPrimitiveNew (
    void)
{
    sd_contextItemPrimitive item = os_malloc(C_SIZEOF(sd_contextItemPrimitive));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_PRIMITIVE);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemTimeNew (
    void)
{
    sd_contextItemTime item = os_malloc(C_SIZEOF(sd_contextItemTime));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_TIME);
    return sd_contextItem(item);
}

static void
sd_contextItemCollectionInit (
    sd_contextItemCollection item,
    sd_contextItemKind kind)
{
    sd_contextItemInit(sd_contextItem(item), kind);
    item->kind = OSPL_C_UNDEFINED;
    item->maxSize = 0;
}

static sd_contextItem
sd_contextItemCollectionNew (
    void)
{
    sd_contextItemCollection item = os_malloc(C_SIZEOF(sd_contextItemCollection));
    sd_contextItemCollectionInit(item, SD_CONTEXT_ITEM_COLLECTION);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemStringNew (
    void)
{
    sd_contextItemString item = os_malloc(C_SIZEOF(sd_contextItemString));
    sd_contextItemCollection coll = sd_contextItemCollection(item);
    sd_contextItemCollectionInit(coll, SD_CONTEXT_ITEM_STRING);
    coll->kind = OSPL_C_STRING;
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemArrayNew (
    void)
{
    sd_contextItemArray item = os_malloc(C_SIZEOF(sd_contextItemArray));
    sd_contextItemCollection coll = sd_contextItemCollection(item);
    sd_contextItemCollectionInit(coll, SD_CONTEXT_ITEM_ARRAY);
    coll->kind = OSPL_C_ARRAY;
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemSequenceNew (
    void)
{
    sd_contextItemSequence item = os_malloc(C_SIZEOF(sd_contextItemSequence));
    sd_contextItemCollection coll = sd_contextItemCollection(item);
    sd_contextItemCollectionInit(coll, SD_CONTEXT_ITEM_SEQUENCE);
    coll->kind = OSPL_C_SEQUENCE;
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemEnumerationNew (
    void)
{
    sd_contextItemEnumeration item = os_malloc(C_SIZEOF(sd_contextItemEnumeration));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_ENUMERATION);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemUnionSwitchNew (
    void)
{
    sd_contextItemUnionSwitch item = os_malloc(C_SIZEOF(sd_contextItemUnionSwitch));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_UNIONSWITCH);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemUnionNew (
    void)
{
    sd_contextItemUnion item = os_malloc(C_SIZEOF(sd_contextItemUnion));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_UNION);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemUnionCaseNew (
    void)
{
    sd_contextItemUnionCase item = os_malloc(C_SIZEOF(sd_contextItemUnionCase));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_UNIONCASE);
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemLiteralNew (
    void)
{
    sd_contextItemLiteral item = os_malloc(C_SIZEOF(sd_contextItemLiteral));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_LITERAL);
    item->type = NULL;
    item->value = c_undefinedValue();
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemLabelNew (
    void)
{
    sd_contextItemLabel item = os_malloc(C_SIZEOF(sd_contextItemLabel));
    sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_LABEL);
    return sd_contextItem(item);
}


sd_contextItem
sd_contextItemNew (
    sd_contextItemKind kind)
{

    switch ( kind ) {
        case SD_CONTEXT_ITEM_SCOPE:
            return sd_contextItemScopeNew();
        case SD_CONTEXT_ITEM_MODULE:
            return sd_contextItemModuleNew();
        case SD_CONTEXT_ITEM_STRUCTURE:
            return sd_contextItemStructureNew();
        case SD_CONTEXT_ITEM_MEMBER:
            return sd_contextItemMemberNew();
        case SD_CONTEXT_ITEM_TYPE:
            return sd_contextItemTypeNew();
        case SD_CONTEXT_ITEM_TYPEDEF:
            return sd_contextItemTypedefNew();
        case SD_CONTEXT_ITEM_PRIMITIVE:
            return sd_contextItemPrimitiveNew();
        case SD_CONTEXT_ITEM_STRING:
            return sd_contextItemStringNew();
        case SD_CONTEXT_ITEM_TIME:
            return sd_contextItemTimeNew();
        case SD_CONTEXT_ITEM_COLLECTION:
            return sd_contextItemCollectionNew();
        case SD_CONTEXT_ITEM_ARRAY:
            return sd_contextItemArrayNew();
        case SD_CONTEXT_ITEM_SEQUENCE:
            return sd_contextItemSequenceNew();
        case SD_CONTEXT_ITEM_ENUMERATION:
            return sd_contextItemEnumerationNew();
        case SD_CONTEXT_ITEM_UNION:
            return sd_contextItemUnionNew();
        case SD_CONTEXT_ITEM_UNIONCASE:
            return sd_contextItemUnionCaseNew();
        case SD_CONTEXT_ITEM_UNIONSWITCH:
            return sd_contextItemUnionSwitchNew();
        case SD_CONTEXT_ITEM_LITERAL:
            return sd_contextItemLiteralNew();
        case SD_CONTEXT_ITEM_LABEL:
            return sd_contextItemLabelNew();
        default:
            return NULL;
    }
}

void
sd_contextItemFree (
    sd_contextItem item)
{
    sd_contextItem child;

    item->refcount--;

    if ( item->refcount == 0 ) {
        if ( item->children ) {
            child = sd_contextItem(sd_listTakeFirst(item->children));
            while ( child ) {
                sd_contextItemFree(child);
                child = sd_contextItem(sd_listTakeFirst(item->children));
            }
            sd_listFree(item->children);
        }

        os_free(item);
    }
}

void
sd_contextItemAddChild (
    sd_contextItem item,
    sd_contextItem child)
{
    assert(item);
    assert(child);

    if ( item->children == NULL ) {
        item->children = sd_listNew();
    }

    assert(item->children);
    sd_listAppend(item->children, sd_contextItemKeep(child));
    child->parent = item;
}

void
sd_contextItemInsertChild (
    sd_contextItem item,
    sd_contextItem child)
{
    assert(item);
    assert(child);

    if ( item->children == NULL ) {
        item->children = sd_listNew();
    }

    assert(item->children);
    sd_listInsert(item->children, sd_contextItemKeep(child));
    child->parent = item;
}

void
sd_contextItemInsertChildAfter(
    sd_contextItem item,
    sd_contextItem child,
    sd_contextItem after)
{
    assert(item);
    assert(item->children);
    assert(child);
    assert(after);


    assert(item->children);
    sd_listInsertAt(item->children, sd_contextItemKeep(child), sd_listIndexOf(item->children, after)+1);
    child->parent = item;
}

void
sd_contextItemInsertChildBefore(
    sd_contextItem item,
    sd_contextItem child,
    sd_contextItem before)
{
    assert(item);
    assert(item->children);
    assert(child);
    assert(before);

    sd_listInsertBefore(item->children, sd_contextItemKeep(child), before);
    child->parent = item;
}

void
sd_contextItemRemoveChild(
    sd_contextItem item,
    sd_contextItem child)
{
    assert(item);
    assert(child);

    if(item->children){
        sd_listRemove(item->children, child);
        sd_contextItemFree(child);
    }
}

void
sd_contextItemReplace (
    sd_contextItem orig,
    sd_contextItem with)
{
    sd_contextItem parent;
    sd_contextItem removed;

    parent = orig->parent;

    assert(parent);
    assert(parent->children);

    removed = (sd_contextItem)sd_listTakeLast(parent->children);

    /* only the last child can be replaced */
    assert(removed == orig);

    sd_contextItemFree(removed);

    with->parent = parent;
    with->name   = orig->name;
    with->scope  = orig->scope;
    sd_listAppend(parent->children, with);
}

sd_contextItem
sd_contextItemGetParent (
    sd_contextItem item)
{
    assert(item);

    return item->parent;
}

typedef struct {
    sd_contextItemKind kind;
    sd_contextItem     item;
} sd_contextItemFindArg;

static c_bool
sd_contextItemCompare (
    void *obj,
    void *arg)
{
    c_bool result = TRUE;
    sd_contextItem item = obj;
    sd_contextItemFindArg *findArg = arg;

    if ( item->kind == findArg->kind ) {
        findArg->item = item;
        result = FALSE;
    }

    return result;
}


sd_contextItem
sd_contextItemFindChild (
    sd_contextItem item,
    sd_contextItemKind kind)
{
    sd_contextItem child = NULL;
    sd_contextItemFindArg findArg;

    assert(item);

    if ( item->children ) {
        findArg.kind = kind;
        findArg.item = NULL;

        sd_listWalk(item->children, sd_contextItemCompare, &findArg);
        child = findArg.item;
    }

    return child;
}

typedef struct {
    sd_contextItemAction action;
    c_bool proceed;
    void *arg;
} sd_contextItemWalkArg;


static c_bool
sd_contextItemWalkAction (
    void *obj,
    void *arg)
{
    sd_contextItem item = obj;
    sd_contextItemWalkArg *walkArg = arg;

    return walkArg->action(item, walkArg->arg);
}

static c_bool
sd_contextItemDeepWalkAction (
    void *obj,
    void *arg)
{
    sd_contextItem item = obj;
    sd_contextItemWalkArg *walkArg = arg;

    walkArg->proceed = walkArg->action(item, walkArg->arg);

    if ( walkArg->proceed && item->children ) {
        sd_listWalk(item->children, sd_contextItemDeepWalkAction, walkArg);
    }

    return walkArg->proceed;
}


void
sd_contextItemWalkChildren (
    sd_contextItem item,
    sd_contextItemAction action,
    void *arg)
{
    sd_contextItemWalkArg walkArg;

    assert(item);
    assert(action);

    if ( item->children ) {
        walkArg.action = action;
        walkArg.arg = arg;

        sd_listWalk(item->children, sd_contextItemWalkAction, &walkArg);
    }
}


void
sd_contextItemDeepWalkChildren (
    sd_contextItem item,
    sd_contextItemAction action,
    void *arg)
{
    sd_contextItemWalkArg walkArg;

    assert(item);
    assert(action);

    if ( item->children ) {
        walkArg.action = action;
        walkArg.proceed = TRUE;
        walkArg.arg = arg;

        sd_listWalk(item->children, sd_contextItemDeepWalkAction, &walkArg);
    }

}

c_bool
sd_contextItemScopeEqual (
    sd_contextItem item,
    c_metaObject scope)
{
    c_bool equal = FALSE;

    if ( scope == item->scope ) {
        equal = TRUE;
    }

    return equal;
}

void
sd_contextItemAddDependency (
    sd_contextItem item,
    sd_contextItem parent)
{
    assert(item);
    assert(parent);

    if ( !item->dependencies ) {
        item->dependencies = sd_listNew();
    }

    assert(item->dependencies);

    if ( !sd_listFindObject(item->dependencies, parent) ) {
        sd_listAppend(item->dependencies, parent);
    }
}


void
sd_contextItemRemoveDependency (
    sd_contextItem item,
    sd_contextItem parent)
{
    assert(item);
    assert(parent);

    if ( item->dependencies ) {
        if ( sd_listRemove(item->dependencies, parent) ) {
            if ( sd_listIsEmpty(item->dependencies) ) {
                sd_listFree(item->dependencies);
                item->dependencies = NULL;
            }
        }
    }
}

c_bool
sd_contextItemHasDependencies (
    sd_contextItem item)
{
    c_bool hasDependencies = FALSE;

    if ( item->dependencies ) {
        hasDependencies = TRUE;
    }

    return hasDependencies;
}

typedef struct sd_findItemByObject_s {
    c_metaObject   object;
    sd_contextItem result;
} sd_findItemByObject;

static c_bool
findItemByObject (
    sd_contextItem item,
    void           *arg)
{
    sd_findItemByObject *info = arg;
    c_bool proceed = TRUE;

    assert(info);
    assert(info->object);

    if ( item->self == info->object ) {
        info->result = item;
        proceed = FALSE;
    }

    return proceed;
}

sd_contextItem
sd_contextItemFindObject (
    sd_contextItem item,
    c_metaObject   object)
{
    sd_findItemByObject info;

    info.object = object;
    info.result = NULL;

    sd_contextItemDeepWalkChildren(item, findItemByObject, &info);

    return info.result;
}

c_bool
sd_contextItemIsAncestor (
    sd_contextItem item,
    sd_contextItem ancestor)
{
    c_bool isAncestor = FALSE;

    while ( !isAncestor && item ) {
        if ( item->parent == ancestor ) {
            isAncestor = TRUE;
        } else {
            item = item->parent;
        }
    }

    return isAncestor;
}


sd_contextItem
sd_contextItemFindAncestor (
    sd_contextItem item1,
    sd_contextItem item2)
{
    sd_contextItem ancestor = NULL;
    sd_contextItem parent    = item1->parent;

    while ( !ancestor && parent ) {
        if ( sd_contextItemIsAncestor(item2, parent) ) {
            ancestor = parent;
        } else {
            parent = parent->parent;
        }
    }

    return ancestor;
}
