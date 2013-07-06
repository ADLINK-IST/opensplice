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
#ifndef SD_XMLNODE_H
#define SD_XMLNODE_H

#include "c_base.h"
#include "sd_list.h"

#define sd_xmlNode(n)      ((sd_xmlNode)n)
#define sd_xmlElement(n)   ((sd_xmlElement)n)
#define sd_xmlAttribute(n) ((sd_xmlAttribute)n)
#define sd_xmlData(n)      ((sd_xmlData)n)

typedef enum {
    SD_XMLNODE_KIND_ELEMENT,
    SD_XMLNODE_KIND_DATA,
    SD_XMLNODE_KIND_ATTRIBUTE
} sd_xmlNodeKind;

C_CLASS(sd_xmlNode);
C_STRUCT(sd_xmlNode) {
    sd_xmlNodeKind  kind;
    c_char         *name;
};

C_CLASS(sd_xmlElement);
C_STRUCT(sd_xmlElement) {
    C_EXTENDS(sd_xmlNode);
    sd_xmlElement parent;
    sd_list       children;
    sd_list       attributes;
    sd_xmlNode    data;
};

C_CLASS(sd_xmlData);
C_STRUCT(sd_xmlData) {
    C_EXTENDS(sd_xmlNode);
    c_char *data;
};

C_CLASS(sd_xmlAttribute);
C_STRUCT(sd_xmlAttribute) {
    C_EXTENDS(sd_xmlNode);
    c_char *value;
};
 
sd_xmlElement
sd_xmlElementNew (
    void);

void
sd_xmlElementFree (
    sd_xmlElement node);

sd_xmlAttribute
sd_xmlAttributeNew (
    void);

void
sd_xmlAttributeFree (
    sd_xmlAttribute node);

sd_xmlData
sd_xmlDataNew (
    void);

void
sd_xmlDataFree (
    sd_xmlData node);

sd_xmlNode
sd_xmlNodeNew (
    sd_xmlNodeKind kind);

void
sd_xmlNodeFree (
    sd_xmlNode node);
 
void
sd_xmlElementAdd (
    sd_xmlElement element,
    sd_xmlNode    child);

sd_xmlElement
sd_xmlElementGetParent (
    sd_xmlElement node);

sd_list
sd_xmlElementGetChildren (
    sd_xmlElement node);

sd_list
sd_xmlElementGetAttributes (
    sd_xmlElement node);

sd_xmlData
sd_xmlElementGetData (
    sd_xmlElement node);

c_bool
sd_xmlElementHasChildren (
    sd_xmlElement node);

typedef c_bool (*sd_xmlNodeWalkAction)(sd_xmlNode node, void *arg);

void
sd_xmlElementWalkChildren (
    sd_xmlElement node,
    sd_xmlNodeWalkAction action,
    void *arg);

void
sd_xmlElementWalkAttributes (
    sd_xmlElement node,
    sd_xmlNodeWalkAction action,
    void *arg);

sd_xmlAttribute
sd_xmlElementFindAttribute (
    sd_xmlElement node,
    const c_char *name);

c_ulong
sd_xmlElementNumAttributes (
    sd_xmlElement node);

c_bool
sd_xmlElementHasAttributes (
    sd_xmlElement node);

c_bool
sd_xmlNodeEqualName (
    sd_xmlNode n1,
    sd_xmlNode n2);

#endif /* SD_XMLNODE_H */
