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
#ifndef CFG_SYNTAX_H
#define CFG_SYNTAX_H

#include "vortex_os.h"
#include "c_typebase.h"
#include "c_iterator.h"

//#define CFG_DEBUG_CONFIG 1


#define cfg_node(_this)              ((cfg_node)(_this))
#define cfg_element(_this)           ((cfg_element)(_this))
#define cfg_emptyElement(_this)      ((cfg_emptyElement)(_this))
#define cfg_booleanElement(_this)    ((cfg_booleanElement)(_this))
#define cfg_intElement(_this)        ((cfg_intElement)(_this))
#define cfg_longElement(_this)       ((cfg_longElement)(_this))
#define cfg_sizeElement(_this)       ((cfg_sizeElement)(_this))
#define cfg_floatElement(_this)      ((cfg_floatElement)(_this))
#define cfg_doubleElement(_this)     ((cfg_doubleElement)(_this))
#define cfg_enumElement(_this)       ((cfg_enumElement)(_this))
#define cfg_stringElement(_this)     ((cfg_stringElement)(_this))

#define cfg_attribute(_this)         ((cfg_attribute)(_this))
#define cfg_booleanAttribute(_this)  ((cfg_booleanAttribute)(_this))
#define cfg_intAttribute(_this)      ((cfg_intAttribute)(_this))
#define cfg_longAttribute(_this)     ((cfg_longAttribute)(_this))
#define cfg_sizeAttribute(_this)     ((cfg_sizeAttribute)(_this))
#define cfg_floatAttribute(_this)    ((cfg_floatAttribute)(_this))
#define cfg_doubleAttribute(_this)   ((cfg_doubleAttribute)(_this))
#define cfg_enumAttribute(_this)     ((cfg_enumAttribute)(_this))
#define cfg_stringAttribute(_this)   ((cfg_stringAttribute)(_this))

#define cfg_serviceMapping(_this)    ((cfg_serviceMapping)(_this))

#define cfg_stringValue(c)           (assert((c).kind == V_STRING), (c).is.String)

C_CLASS(cfg_node);
C_CLASS(cfg_element);
C_CLASS(cfg_emptyElement);
C_CLASS(cfg_booleanElement);
C_CLASS(cfg_intElement);
C_CLASS(cfg_longElement);
C_CLASS(cfg_sizeElement);
C_CLASS(cfg_floatElement);
C_CLASS(cfg_doubleElement);
C_CLASS(cfg_enumElement);
C_CLASS(cfg_stringElement);
C_CLASS(cfg_serviceMapping);

C_CLASS(cfg_attribute);
C_CLASS(cfg_booleanAttribute);
C_CLASS(cfg_intAttribute);
C_CLASS(cfg_longAttribute);
C_CLASS(cfg_sizeAttribute);
C_CLASS(cfg_floatAttribute);
C_CLASS(cfg_doubleAttribute);
C_CLASS(cfg_enumAttribute);
C_CLASS(cfg_stringAttribute);

void
cfg_nodeFree(
    _In_ cfg_node _this);

void
cfg_nodeSetName(
    _In_ cfg_node _this,
    _In_ const char *name) __nonnull_all__;

const char *
cfg_nodeGetName(
    _In_ cfg_node _this) __nonnull_all__;

cfg_node
cfg_nodeGetParent(
    cfg_node _this);

os_uint32
cfg_nodeGetOccurrences(
    cfg_node _this) __nonnull_all__;

void
cfg_nodeIncOccurrences(
    cfg_node _this) __nonnull_all__;

void
cfg_nodeResetOccurrences(
    cfg_node _this) __nonnull_all__;

os_boolean
cfg_nodeIsElement(
    _In_ cfg_node _this) __nonnull_all__;

cfg_element
cfg_elementNew(void);

void
cfg_elementAddChild(
    _In_ cfg_element _this,
    _In_ cfg_element element) __nonnull_all__;

void
cfg_elementAddAttribute(
    _In_ cfg_element _this,
    _In_ cfg_attribute attribute) __nonnull_all__;

cfg_element
cfg_elementFindChild(
    _In_ cfg_element _this,
    _In_ const char *name) __nonnull_all__;

cfg_attribute
cfg_elementFindAttribute(
    _In_ cfg_element _this,
    _In_ const char *name) __nonnull_all__;

os_boolean
cfg_elementCheckValue(
    _In_ cfg_element _this,
    _In_ const char *value) __nonnull_all__;

c_iter
cfg_elementGetAttributes(
   _In_ cfg_element element) __nonnull_all__;

c_iter
cfg_elementGetChildren(
   _In_ cfg_element element) __nonnull_all__;

cfg_emptyElement
cfg_emptyElementNew(void);

cfg_booleanElement
cfg_booleanElementNew(void);

cfg_intElement
cfg_intElementNew(void);

cfg_longElement
cfg_longElementNew(void);

cfg_sizeElement
cfg_sizeElementNew(void);

cfg_floatElement
cfg_floatElementNew(void);

cfg_doubleElement
cfg_doubleElementNew(void);

cfg_enumElement
cfg_enumElementNew(void);

cfg_stringElement
cfg_stringElementNew(void);

cfg_serviceMapping
cfg_serviceMappingNew(void);

void
cfg_serviceMappingSetCommand(
    _In_ cfg_serviceMapping _this,
    _In_ const char *command) __nonnull_all__;

const char *
cfg_serviceMappingGetCommand(
    cfg_serviceMapping _this) __nonnull_all__;

os_boolean
cfg_nodeIsServiceMapping(
   _In_ cfg_node _this) __nonnull_all__;

cfg_booleanAttribute
cfg_booleanAttributeNew(void);

cfg_intAttribute
cfg_intAttributeNew(void);

cfg_longAttribute
cfg_longAttributeNew(void);

cfg_sizeAttribute
cfg_sizeAttributeNew(void);

cfg_floatAttribute
cfg_floatAttributeNew(void);

cfg_doubleAttribute
cfg_doubleAttributeNew(void);

cfg_enumAttribute
cfg_enumAttributeNew(void);

cfg_stringAttribute
cfg_stringAttributeNew(void);

os_boolean
cfg_attributeIsRequired(
    cfg_attribute _this) __nonnull_all__;

os_boolean
cfg_attributeCheckValue(
    _In_ cfg_attribute attribute,
    _In_ const char *value) __nonnull_all__;

os_boolean
cfg_nodeSetMinOccurrences(
    _In_ cfg_node _this,
    _In_ const char *image) __nonnull_all__;

os_uint32
cfg_elementGetMinOccurrences(
    _In_ cfg_element _this) __nonnull_all__;

os_boolean
cfg_nodeSetMaxOccurrences(
    _In_ cfg_node _this,
    _In_ const char *image) __nonnull_all__;

os_uint32
cfg_elementGetMaxOccurrences(
    _In_ cfg_element _this) __nonnull_all__;

os_boolean
cfg_nodeSetMinimum(
    _In_ cfg_node _this,
    _In_ const char *image) __nonnull_all__;

os_boolean
cfg_nodeSetMaximum(
    _In_ cfg_node _this,
    _In_ const char *image) __nonnull_all__;

os_boolean
cfg_nodeSetMaxLength(
    _In_ cfg_node _this,
    _In_ const char *image) __nonnull_all__;

os_boolean
cfg_nodeAddLabel(
    _In_ cfg_node _this,
    _In_ const char *image) __nonnull_all__;

os_boolean
cfg_nodeSetRequired(
    _In_ cfg_node _this,
    _In_ const char *image) __nonnull_all__;

const char *
cfg_nodeGetFullName(
    _In_ cfg_node _this,
    _In_ char *buffer,
    _In_ os_uint32 buflen) __nonnull((1,2));

#ifdef CFG_DEBUG_CONFIG
void
cfg_nodePrint(
    _In_ cfg_node _this,
    _In_ os_uint32 level) __nonnull((1));
#endif


#endif /* CFG_SYNTAX_H */
