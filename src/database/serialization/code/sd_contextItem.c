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
    sd_contextItemScope item;

    item = (sd_contextItemScope)os_malloc(C_SIZEOF(sd_contextItemScope));
    if ( item ) {
        sd_contextItemScopeInit(item, SD_CONTEXT_ITEM_SCOPE);
    }
    return sd_contextItem(item);
}


static sd_contextItem
sd_contextItemModuleNew (
    void)
{
    sd_contextItemModule item;

    item = (sd_contextItemModule)os_malloc(C_SIZEOF(sd_contextItemModule));
    if ( item ) {
        sd_contextItemScopeInit(sd_contextItemScope(item), SD_CONTEXT_ITEM_MODULE);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemStructureNew (
    void)
{
    sd_contextItemStructure item;

    item = (sd_contextItemStructure)os_malloc(C_SIZEOF(sd_contextItemStructure));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_STRUCTURE);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemMemberNew (
    void)
{
    sd_contextItemMember item;

    item = (sd_contextItemMember)os_malloc(C_SIZEOF(sd_contextItemMember));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_MEMBER);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemTypeNew (
    void)
{
    sd_contextItemType item;

    item = (sd_contextItemType)os_malloc(C_SIZEOF(sd_contextItemType));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_TYPE);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemTypedefNew (
    void)
{
    sd_contextItemTypedef item;

    item = (sd_contextItemTypedef)os_malloc(C_SIZEOF(sd_contextItemTypedef));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_TYPEDEF);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemPrimitiveNew (
    void)
{
    sd_contextItemPrimitive item;

    item = (sd_contextItemPrimitive)os_malloc(C_SIZEOF(sd_contextItemPrimitive));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_PRIMITIVE);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemTimeNew (
    void)
{
    sd_contextItemTime item;

    item = (sd_contextItemTime)os_malloc(C_SIZEOF(sd_contextItemTime));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_TIME);
    }
    return sd_contextItem(item);
}

static void
sd_contextItemCollectionInit (
    sd_contextItemCollection item,
    sd_contextItemKind kind)
{
    sd_contextItemInit(sd_contextItem(item), kind);
    item->kind = C_UNDEFINED;
    item->maxSize = 0;
}

static sd_contextItem
sd_contextItemCollectionNew (
    void)
{
    sd_contextItemCollection item;

    item = (sd_contextItemCollection)os_malloc(C_SIZEOF(sd_contextItemCollection));
    if ( item ) {
        sd_contextItemCollectionInit(item, SD_CONTEXT_ITEM_COLLECTION);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemStringNew (
    void)
{
    sd_contextItemString item;
    sd_contextItemCollection coll;

    item = (sd_contextItemString)os_malloc(C_SIZEOF(sd_contextItemString));
    if ( item ) {
        coll = sd_contextItemCollection(item);
        sd_contextItemCollectionInit(coll, SD_CONTEXT_ITEM_STRING);
        coll->kind = C_STRING;
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemArrayNew (
    void)
{
    sd_contextItemArray item;
    sd_contextItemCollection coll;

    item = (sd_contextItemArray)os_malloc(C_SIZEOF(sd_contextItemArray));
    if ( item ) {
        coll = sd_contextItemCollection(item);
        sd_contextItemCollectionInit(coll, SD_CONTEXT_ITEM_ARRAY);
        coll->kind = C_ARRAY;
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemSequenceNew (
    void)
{
    sd_contextItemSequence item;
    sd_contextItemCollection coll;

    item = (sd_contextItemSequence)os_malloc(C_SIZEOF(sd_contextItemSequence));
    if ( item ) {
        coll = sd_contextItemCollection(item);
        sd_contextItemCollectionInit(coll, SD_CONTEXT_ITEM_SEQUENCE);
        coll->kind = C_SEQUENCE;
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemEnumerationNew (
    void)
{
    sd_contextItemEnumeration item;

    item = (sd_contextItemEnumeration)os_malloc(C_SIZEOF(sd_contextItemEnumeration));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_ENUMERATION);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemUnionSwitchNew (
    void)
{
    sd_contextItemUnionSwitch item;

    item = (sd_contextItemUnionSwitch)os_malloc(C_SIZEOF(sd_contextItemUnionSwitch));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_UNIONSWITCH);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemUnionNew (
    void)
{
    sd_contextItemUnion item;

    item = (sd_contextItemUnion)os_malloc(C_SIZEOF(sd_contextItemUnion));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_UNION);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemUnionCaseNew (
    void)
{
    sd_contextItemUnionCase item;

    item = (sd_contextItemUnionCase)os_malloc(C_SIZEOF(sd_contextItemUnionCase));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_UNIONCASE);
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemLiteralNew (
    void)
{
    sd_contextItemLiteral item;

    item = (sd_contextItemLiteral)os_malloc(C_SIZEOF(sd_contextItemLiteral));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_LITERAL);
        item->type = NULL;
        item->value = c_undefinedValue();
    }
    return sd_contextItem(item);
}

static sd_contextItem
sd_contextItemLabelNew (
    void)
{
    sd_contextItemLabel item;

    item = (sd_contextItemLabel)os_malloc(C_SIZEOF(sd_contextItemLabel));
    if ( item ) {
        sd_contextItemInit(sd_contextItem(item), SD_CONTEXT_ITEM_LABEL);
    }
    return sd_contextItem(item);
}


sd_contextItem
sd_contextItemNew (
    sd_contextItemKind kind)
{
    sd_contextItem item = NULL;

    switch ( kind ) {
        case SD_CONTEXT_ITEM_SCOPE:
            item = sd_contextItemScopeNew();
            break;
        case SD_CONTEXT_ITEM_MODULE:
            item = sd_contextItemModuleNew();
            break;
        case SD_CONTEXT_ITEM_STRUCTURE:
            item = sd_contextItemStructureNew();
            break;
        case SD_CONTEXT_ITEM_MEMBER:
            item = sd_contextItemMemberNew();
            break;
        case SD_CONTEXT_ITEM_TYPE:
            item = sd_contextItemTypeNew();
            break;
        case SD_CONTEXT_ITEM_TYPEDEF:
            item = sd_contextItemTypedefNew();
            break;
        case SD_CONTEXT_ITEM_PRIMITIVE:
            item = sd_contextItemPrimitiveNew();
            break;
        case SD_CONTEXT_ITEM_STRING:
            item = sd_contextItemStringNew();
            break;
        case SD_CONTEXT_ITEM_TIME:
            item = sd_contextItemTimeNew();
            break;
        case SD_CONTEXT_ITEM_COLLECTION:
            item = sd_contextItemCollectionNew();
            break;
        case SD_CONTEXT_ITEM_ARRAY:
            item = sd_contextItemArrayNew();
            break;
        case SD_CONTEXT_ITEM_SEQUENCE:
            item = sd_contextItemSequenceNew();
            break;
        case SD_CONTEXT_ITEM_ENUMERATION:
            item = sd_contextItemEnumerationNew();
            break;
        case SD_CONTEXT_ITEM_UNION:
            item = sd_contextItemUnionNew();
            break;
        case SD_CONTEXT_ITEM_UNIONCASE:
            item = sd_contextItemUnionCaseNew();
            break;
        case SD_CONTEXT_ITEM_UNIONSWITCH:
            item = sd_contextItemUnionSwitchNew();
            break;
        case SD_CONTEXT_ITEM_LITERAL:
            item = sd_contextItemLiteralNew();
            break;
        case SD_CONTEXT_ITEM_LABEL:
            item = sd_contextItemLabelNew();
            break;
        default:
            item = NULL;
            break;
    }

    return item;
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
    sd_contextItem item = (sd_contextItem) obj;
    sd_contextItemFindArg *findArg = (sd_contextItemFindArg*) arg;

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
    sd_contextItem item = (sd_contextItem) obj;
    sd_contextItemWalkArg *walkArg = (sd_contextItemWalkArg*) arg;

    return walkArg->action(item, walkArg->arg);
}

static c_bool
sd_contextItemDeepWalkAction (
    void *obj,
    void *arg)
{
    sd_contextItem item = (sd_contextItem) obj;
    sd_contextItemWalkArg *walkArg = (sd_contextItemWalkArg*) arg;

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
    sd_findItemByObject *info = (sd_findItemByObject *) arg;
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
