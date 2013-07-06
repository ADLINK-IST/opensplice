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
#ifndef SD__CONTEXTITEM_H
#define SD__CONTEXTITEM_H

#include "c_base.h"
#include "sd_list.h"

#define sd_contextItem(i)           ((sd_contextItem)i)
#define sd_contextItemScope(i)      ((sd_contextItemScope)i)
#define sd_contextItemCollection(i) ((sd_contextItemCollection)i)
#define sd_contextItemType(i)       ((sd_contextItemType)i)

typedef enum {
    SD_CONTEXT_ITEM_UNKNOWN,
    SD_CONTEXT_ITEM_SCOPE,
    SD_CONTEXT_ITEM_MODULE,
    SD_CONTEXT_ITEM_STRUCTURE,
    SD_CONTEXT_ITEM_MEMBER,
    SD_CONTEXT_ITEM_TYPE,
    SD_CONTEXT_ITEM_TYPEDEF,
    SD_CONTEXT_ITEM_PRIMITIVE,
    SD_CONTEXT_ITEM_COLLECTION,
    SD_CONTEXT_ITEM_STRING,
    SD_CONTEXT_ITEM_TIME,
    SD_CONTEXT_ITEM_ARRAY,
    SD_CONTEXT_ITEM_SEQUENCE,
    SD_CONTEXT_ITEM_ENUMERATION,
    SD_CONTEXT_ITEM_UNIONSWITCH,
    SD_CONTEXT_ITEM_UNIONCASE,
    SD_CONTEXT_ITEM_UNION,
    SD_CONTEXT_ITEM_LITERAL,
    SD_CONTEXT_ITEM_LABEL
} sd_contextItemKind;

C_CLASS(sd_contextItem);
C_STRUCT(sd_contextItem) {
    sd_contextItemKind kind;
    c_ulong            refcount;
    sd_contextItem     parent;
    c_char            *name;
    c_metaObject       self;
    c_metaObject       scope;
    sd_list            children;
    sd_list            dependencies;
};

C_CLASS(sd_contextItemScope);
C_STRUCT(sd_contextItemScope) {
    C_EXTENDS(sd_contextItem);
};


C_CLASS(sd_contextItemModule);
C_STRUCT(sd_contextItemModule) {
    C_EXTENDS(sd_contextItemScope);
};

C_CLASS(sd_contextItemStructure);
C_STRUCT(sd_contextItemStructure) {
    C_EXTENDS(sd_contextItem);
};

C_CLASS(sd_contextItemMember);
C_STRUCT(sd_contextItemMember) {
    C_EXTENDS(sd_contextItem);
};

C_CLASS(sd_contextItemType);
C_STRUCT(sd_contextItemType) {
    C_EXTENDS(sd_contextItem);
    c_type type;
};

C_CLASS(sd_contextItemTypedef);
C_STRUCT(sd_contextItemTypedef) {
    C_EXTENDS(sd_contextItem);
};

C_CLASS(sd_contextItemPrimitive);
C_STRUCT(sd_contextItemPrimitive) {
    C_EXTENDS(sd_contextItemType);
    c_primKind kind;
};

C_CLASS(sd_contextItemTime);
C_STRUCT(sd_contextItemTime) {
    C_EXTENDS(sd_contextItem);
};

C_CLASS(sd_contextItemCollection);
C_STRUCT(sd_contextItemCollection) {
    C_EXTENDS(sd_contextItem);
    c_collKind kind;
    c_long     maxSize;
};

C_CLASS(sd_contextItemString);
C_STRUCT(sd_contextItemString) {
    C_EXTENDS(sd_contextItemCollection);
};

C_CLASS(sd_contextItemArray);
C_STRUCT(sd_contextItemArray) {
    C_EXTENDS(sd_contextItemCollection);
};

C_CLASS(sd_contextItemSequence);
C_STRUCT(sd_contextItemSequence) {
    C_EXTENDS(sd_contextItemCollection);
};


C_CLASS(sd_contextItemEnumeration);
C_STRUCT(sd_contextItemEnumeration) {
    C_EXTENDS(sd_contextItem);
    c_array elements;
};

C_CLASS(sd_contextItemUnionSwitch);
C_STRUCT(sd_contextItemUnionSwitch) {
    C_EXTENDS(sd_contextItem);
};

C_CLASS(sd_contextItemUnion);
C_STRUCT(sd_contextItemUnion) {
    C_EXTENDS(sd_contextItem);
};

C_CLASS(sd_contextItemUnionCase);
C_STRUCT(sd_contextItemUnionCase) {
    C_EXTENDS(sd_contextItem);
};

C_CLASS(sd_contextItemLiteral);
C_STRUCT(sd_contextItemLiteral) {
    C_EXTENDS(sd_contextItem);
    c_type type;
    c_value value;
};

C_CLASS(sd_contextItemLabel);
C_STRUCT(sd_contextItemLabel) {
    C_EXTENDS(sd_contextItem);
};

sd_contextItem
sd_contextItemNew (
    sd_contextItemKind kind);

void
sd_contextItemFree (
    sd_contextItem item);

sd_contextItem
sd_contextItemKeep (
    sd_contextItem item);


void
sd_contextItemAddChild (
    sd_contextItem parent,
    sd_contextItem child);

void
sd_contextItemInsertChild (
    sd_contextItem item,
    sd_contextItem child);

void
sd_contextItemInsertChildAfter(
    sd_contextItem item,
    sd_contextItem child,
    sd_contextItem after);

void
sd_contextItemInsertChildBefore(
    sd_contextItem item,
    sd_contextItem child,
    sd_contextItem before);

void
sd_contextItemRemoveChild(
    sd_contextItem item,
    sd_contextItem child);

void 
sd_contextItemReplace (
    sd_contextItem orig,
    sd_contextItem with);

sd_contextItem
sd_contextItemGetParent (
    sd_contextItem item);

sd_contextItem
sd_contextItemFindChild (
    sd_contextItem item,
    sd_contextItemKind kind);


typedef c_bool (*sd_contextItemAction)(sd_contextItem item, void *arg);

void
sd_contextItemWalkChildren (
    sd_contextItem item,
    sd_contextItemAction action,
    void *arg);

void
sd_contextItemDeepWalkChildren (
    sd_contextItem item,
    sd_contextItemAction action,
    void *arg);

c_bool
sd_contextItemScopeEqual (
    sd_contextItem item,
    c_metaObject scope);

void
sd_contextItemAddDependency (
    sd_contextItem item,
    sd_contextItem parent);

void
sd_contextItemRemoveDependency (
    sd_contextItem item,
    sd_contextItem parent);

c_bool
sd_contextItemHasDependencies (
    sd_contextItem item);

sd_contextItem
sd_contextItemFindObject (
    sd_contextItem item,
    c_metaObject   object);

c_bool
sd_contextItemIsAncestor (
    sd_contextItem item,
    sd_contextItem ancestor);

sd_contextItem
sd_contextItemFindAncestor (
    sd_contextItem item1,
    sd_contextItem item2);

#endif /* SD__CONTEXTITEM_H */
