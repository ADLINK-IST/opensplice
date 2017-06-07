/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#include "os_abstract.h"
#include "cfg_metaConfig.h"

#define cfg_nodeKind(_this)          (cfg_node(_this)->kind)

#define CFG_INT_MIN     INT32_MIN
#define CFG_INT_MAX     INT32_MAX
#define CFG_LONG_MIN    INT64_MIN
#define CFG_LONG_MAX    INT64_MAX
#define CFG_SIZE_MIN    0
#define CFG_SIZE_MAX    UINT64_MAX
#define CFG_FLOAT_MIN   1.1754943508222875E-38
#define CFG_FLOAT_MAX   3.4028234663852886E+38
#define CFG_DOUBLE_MIN  2.2250738585072014E-308
#define CFG_DOUBLE_MAX  1.7976931348623157E+308



typedef enum {
    CFG_NODE_KIND_ELEMENT,
    CFG_NODE_KIND_ATTRIBUTE
} cfg_nodeKind_t;


typedef enum {
    CFG_ELEMENT_KIND_ELEMENT,
    CFG_ELEMENT_KIND_EMPTY,
    CFG_ELEMENT_KIND_BOOLEAN,
    CFG_ELEMENT_KIND_INT,
    CFG_ELEMENT_KIND_LONG,
    CFG_ELEMENT_KIND_SIZE,
    CFG_ELEMENT_KIND_FLOAT,
    CFG_ELEMENT_KIND_DOUBLE,
    CFG_ELEMENT_KIND_ENUM,
    CFG_ELEMENT_KIND_STRING,
    CFG_ELEMENT_KIND_SERVICE_MAPPING
} cfg_elementKind_t;

typedef enum {
    CFG_ATTRIBUTE_KIND_BOOLEAN,
    CFG_ATTRIBUTE_KIND_INT,
    CFG_ATTRIBUTE_KIND_LONG,
    CFG_ATTRIBUTE_KIND_SIZE,
    CFG_ATTRIBUTE_KIND_FLOAT,
    CFG_ATTRIBUTE_KIND_DOUBLE,
    CFG_ATTRIBUTE_KIND_ENUM,
    CFG_ATTRIBUTE_KIND_STRING,
} cfg_attributeKind_t;





typedef void (*cfg_nodeDestructorFunc)(cfg_node node);

C_STRUCT(cfg_node) {
    cfg_nodeKind_t kind;
    cfg_nodeDestructorFunc destructor;
    cfg_node parent;
    char *name;
    os_uint32 occurrences;
};

C_STRUCT(cfg_attribute) {
    C_EXTENDS(cfg_node);
    cfg_attributeKind_t attributeKind;
    os_boolean required;
};


C_STRUCT(cfg_element) {
    C_EXTENDS(cfg_node);
    cfg_elementKind_t elementKind;
    c_iter attributes;
    c_iter children;
    os_uint32 minOccurrences;
    os_uint32 maxOccurrences;
};

C_STRUCT(cfg_serviceMapping) {
    C_EXTENDS(cfg_element);
    char *command;
};

C_STRUCT(cfg_booleanAttribute) {
    C_EXTENDS(cfg_attribute);
};

C_STRUCT(cfg_intAttribute) {
    C_EXTENDS(cfg_attribute);
    os_int32 min;
    os_int32 max;
};

C_STRUCT(cfg_longAttribute) {
    C_EXTENDS(cfg_attribute);
    os_int64 min;
    os_int64 max;
};

C_STRUCT(cfg_sizeAttribute) {
    C_EXTENDS(cfg_attribute);
    os_uint64 min;
    os_uint64 max;
};

C_STRUCT(cfg_floatAttribute) {
    C_EXTENDS(cfg_attribute);
    os_float min;
    os_float max;
};

C_STRUCT(cfg_doubleAttribute) {
    C_EXTENDS(cfg_attribute);
    os_double min;
    os_double max;
};

C_STRUCT(cfg_enumAttribute) {
    C_EXTENDS(cfg_attribute);
    c_iter labels;
};

C_STRUCT(cfg_stringAttribute) {
    C_EXTENDS(cfg_attribute);
    os_uint32 maxSize;
    char *min;
    char *max;
};

C_STRUCT(cfg_emptyElement) {
    C_EXTENDS(cfg_element);
};

C_STRUCT(cfg_booleanElement) {
    C_EXTENDS(cfg_element);
};

C_STRUCT(cfg_intElement) {
    C_EXTENDS(cfg_element);
    os_int32 min;
    os_int32 max;
};

C_STRUCT(cfg_longElement) {
    C_EXTENDS(cfg_element);
    os_int64 min;
    os_int64 max;
};

C_STRUCT(cfg_sizeElement) {
    C_EXTENDS(cfg_element);
    os_uint64 min;
    os_uint64 max;
};

C_STRUCT(cfg_floatElement) {
    C_EXTENDS(cfg_element);
    os_float min;
    os_float max;
};

C_STRUCT(cfg_doubleElement) {
    C_EXTENDS(cfg_element);
    os_double min;
    os_double max;
};

C_STRUCT(cfg_enumElement) {
    C_EXTENDS(cfg_element);
    c_iter labels;
};

C_STRUCT(cfg_stringElement) {
    C_EXTENDS(cfg_element);
    os_uint32 maxSize;
    char *min;
    char *max;
};


/************************************/

static os_boolean
stringToSizeValue(
    const char *image,
    os_uint64  *value) __nonnull_all__;


static os_boolean
stringToIntValue(
    const char *image,
    os_int32   *value) __nonnull_all__;


static os_boolean
stringToUIntValue(
    const char *image,
    os_uint32  *value) __nonnull_all__;


static os_boolean
stringToLongValue(
    const char *image,
    os_int64   *value) __nonnull_all__;


static os_boolean
stringToFloatValue(
    const char *image,
    os_float   *value) __nonnull_all__;


static os_boolean
stringToDoubleValue(
    const char *image,
    os_double  *value) __nonnull_all__;


static os_boolean
stringToBooleanValue(
    const char *image,
    os_boolean *value) __nonnull_all__;


/************************************/


static c_equality
cfg_nodeCompare(
    void *o,
    c_iterResolveCompareArg arg)
{
    cfg_node node = o;
    const char *name = arg;

    assert(node);
    assert(node->name);
    assert(name);

    if (strcmp(node->name, name) == 0) {
        return C_EQ;
    }
    return C_NE;
}


static void
cfg_nodeInit(
    cfg_node _this,
    cfg_nodeKind_t kind,
    cfg_nodeDestructorFunc destructor)
{
    _this->kind = kind;
    _this->destructor = destructor;
    _this->name = NULL;
    _this->parent = NULL;
    _this->occurrences = 0;
}

static void
cfg_nodeDeinit(
    cfg_node _this)
{
    assert(_this);

    os_free(_this->name);
}


void
cfg_nodeFree(
    cfg_node _this)
{
    if (_this) {
        if (_this->destructor) {
            _this->destructor(_this);
        }
        os_free(_this);
    }
}

void
cfg_nodeSetName(
    cfg_node _this,
    const char *name)
{
    _this->name = os_strdup(name);
}

const char *
cfg_nodeGetName(
    cfg_node _this)
{
    return _this->name;
}

cfg_node
cfg_nodeGetParent(
    cfg_node _this)
{
    if (_this) {
        return _this->parent;
    }
    return NULL;
}

os_uint32
cfg_nodeGetOccurrences(
    cfg_node _this)
{
    assert(_this);

    return _this->occurrences;
}

void
cfg_nodeIncOccurrences(
    cfg_node _this)
{
    assert(_this);

    _this->occurrences++;
}

void
cfg_nodeResetOccurrences(
    cfg_node _this)
{
    assert(_this);

    _this->occurrences = 0;
}

const char *
cfg_nodeGetFullName(
    cfg_node _this,
    char *buffer,
    c_ulong buflen)
{
    c_ulong len = 0;
    c_iter iter = NULL;
    cfg_node node;

    assert(_this);
    assert(buffer);
    assert(buflen > 0);

    iter = c_iterNew(NULL);
    node = _this;
    while (node) {
        iter = c_iterInsert(iter, node);
        node = node->parent;
    }

    node = c_iterTakeFirst(iter);
    while (node && len < buflen -1) {
        if (len == 0) {
            len += (c_ulong)snprintf(&buffer[len], buflen-len, "%s", node->name);
        } else {
            len += (c_ulong)snprintf(&buffer[len], buflen-len, "/%s", node->name);
        }
        node = c_iterTakeFirst(iter);
    }

    buffer[len] = '\0';

    c_iterFree(iter);

    return buffer;
}


os_boolean
cfg_nodeIsElement(
    cfg_node node)
{
    return (node->kind == CFG_NODE_KIND_ELEMENT) ? OS_TRUE : OS_FALSE;
}

static void
cfg_elementInit(
    cfg_element _this,
    cfg_elementKind_t kind,
    cfg_nodeDestructorFunc destructor)
{
    cfg_nodeInit(cfg_node(_this), CFG_NODE_KIND_ELEMENT, destructor);
    _this->elementKind = kind;
    _this->attributes = NULL;
    _this->children = NULL;
    _this->minOccurrences = 0;
    _this->maxOccurrences = UINT32_MAX;
}

static void
cfg_elementDeinit(
    cfg_node e)
{
    cfg_element _this = cfg_element(e);
    cfg_node node;

    assert(_this);

    node = cfg_node(c_iterTakeFirst(_this->children));
    while (node) {
        cfg_nodeFree(node);
        node = cfg_node(c_iterTakeFirst(_this->children));
    }
    c_iterFree(_this->children);

    node = cfg_node(c_iterTakeFirst(_this->attributes));
    while (node) {
        cfg_nodeFree(node);
        node = cfg_node(c_iterTakeFirst(_this->attributes));
    }
    c_iterFree(_this->attributes);
    cfg_nodeDeinit(e);
}

cfg_element
cfg_elementNew(void)
{
    cfg_element element = os_malloc(C_SIZEOF(cfg_element));

    cfg_elementInit(element, CFG_ELEMENT_KIND_ELEMENT, cfg_elementDeinit);

    return element;
}

cfg_element
cfg_elementFindChild(
    cfg_element _this,
    const char *name)
{
    cfg_element child = NULL;

    assert(_this);

    child = cfg_element(c_iterResolve(_this->children, cfg_nodeCompare, (void *)name));

    return child;
}



cfg_attribute
cfg_elementFindAttribute(
    cfg_element _this,
    const char *name)
{
    cfg_attribute attribute = NULL;

    assert(_this);

    attribute = cfg_attribute(c_iterResolve(_this->attributes, cfg_nodeCompare, (void *)name));

    return attribute;
}

void
cfg_elementAddAttribute(
    cfg_element _this,
    cfg_attribute attribute)
{
    assert(_this);
    assert(attribute);

    _this->attributes = c_iterAppend(_this->attributes, attribute);
    cfg_node(attribute)->parent = cfg_node(_this);
}

void
cfg_elementAddChild(
    cfg_element _this,
    cfg_element element)
{
    assert(_this);
    assert(element);

    _this->children = c_iterAppend(_this->children, element);
    cfg_node(element)->parent = cfg_node(_this);
}

static void
cfg_elementSetMinOccurrences(
    cfg_element _this,
    os_uint32 minOccurrences)
{
    _this->minOccurrences = minOccurrences;
}

static void
cfg_elementSetMaxOccurrences(
    cfg_element _this,
    os_uint32 maxOccurrences)
{
    if (maxOccurrences > 0) {
        _this->maxOccurrences = maxOccurrences;
    }
}

c_iter
cfg_elementGetAttributes(
   _In_ cfg_element _this)
{
    assert(_this);
    assert(cfg_node(_this)->kind == CFG_NODE_KIND_ELEMENT);

    return _this->attributes;
}

c_iter
cfg_elementGetChildren(
   _In_ cfg_element _this)
{
    assert(_this);
    assert(cfg_node(_this)->kind == CFG_NODE_KIND_ELEMENT);

    return _this->children;
}

static void
cfg_serviceMappingDeinit(
    cfg_node node)
{
    cfg_serviceMapping _this = cfg_serviceMapping(node);

    assert(node);

    os_free(_this->command);

    cfg_elementDeinit(node);
}

cfg_serviceMapping
cfg_serviceMappingNew(void)
{
    cfg_serviceMapping sm = os_malloc(C_SIZEOF(cfg_serviceMapping));

    cfg_elementInit(cfg_element(sm), CFG_ELEMENT_KIND_SERVICE_MAPPING, cfg_serviceMappingDeinit);
    sm->command = NULL;

    return sm;
}

void
cfg_serviceMappingSetCommand(
    cfg_serviceMapping _this,
    const char *command)
{
    assert(_this);
    assert(command);

    _this->command = os_strdup(command);
}

const char *
cfg_serviceMappingGetCommand(
    cfg_serviceMapping _this)
{
    assert(_this);

    return _this->command;
}

os_boolean
cfg_nodeIsServiceMapping(
   _In_ cfg_node _this)
{
    if (cfg_nodeIsElement(_this)) {
        return (cfg_element(_this)->elementKind == CFG_ELEMENT_KIND_SERVICE_MAPPING ? OS_TRUE : OS_FALSE);
    }

    return OS_FALSE;
}

cfg_emptyElement
cfg_emptyElementNew(void)
{
    cfg_emptyElement _this = os_malloc(C_SIZEOF(cfg_emptyElement));

    cfg_elementInit(cfg_element(_this), CFG_ELEMENT_KIND_EMPTY, cfg_elementDeinit);

    return _this;
}

cfg_booleanElement
cfg_booleanElementNew(void)
{
    cfg_booleanElement _this = os_malloc(C_SIZEOF(cfg_booleanElement));

    cfg_elementInit(cfg_element(_this), CFG_ELEMENT_KIND_BOOLEAN, cfg_elementDeinit);

    return _this;
}

static os_boolean
cfg_booleanElementCheckValue(
    cfg_booleanElement _this,
    const char *str)
{
    os_boolean valid;
    os_boolean value;

    OS_UNUSED_ARG(_this);

    valid = stringToBooleanValue(str, &value);

    return valid;
}

cfg_intElement
cfg_intElementNew(void)
{
    cfg_intElement _this = os_malloc(C_SIZEOF(cfg_intElement));

    cfg_elementInit(cfg_element(_this), CFG_ELEMENT_KIND_INT, cfg_elementDeinit);
    _this->min = CFG_INT_MIN;
    _this->max = CFG_INT_MAX;

    return _this;
}


static os_boolean
cfg_intElementSetMinValue(
    cfg_intElement _this,
    const char *str)
{
    os_boolean valid;
    os_int32 value;

    valid = stringToIntValue(str, &value);
    if (valid) {
        _this->min = value;
    }
    return valid;
}

static os_boolean
cfg_intElementSetMaxValue(
    cfg_intElement _this,
    const char *str)
{
    os_boolean valid;
    os_int32 value;

    valid = stringToIntValue(str, &value);
    if (valid) {
        _this->max = value;
    }
    return valid;
}

static os_boolean
cfg_intElementCheckValue(
    cfg_intElement _this,
    const char *str)
{
    os_boolean valid;
    os_int32 value;

    valid = stringToIntValue(str, &value);
    if (valid) {
        if ((value < _this->min) || (value > _this->max)) {
            valid = OS_FALSE;
        }
    }
    return valid;
}

cfg_longElement
cfg_longElementNew(void)
{
    cfg_longElement _this = os_malloc(C_SIZEOF(cfg_longElement));

    cfg_elementInit(cfg_element(_this), CFG_ELEMENT_KIND_LONG, cfg_elementDeinit);
    _this->min = CFG_LONG_MIN;
    _this->max = CFG_LONG_MAX;

    return _this;
}

static os_boolean
cfg_longElementSetMinValue(
    cfg_longElement _this,
    const char *str)
{
    os_boolean valid;
    os_int64 value;

    valid = stringToLongValue(str, &value);
    if (valid) {
        _this->min = value;
    }
    return valid;
}

static os_boolean
cfg_longElementSetMaxValue(
    cfg_longElement _this,
    const char *str)
{
    os_boolean valid;
    os_int64 value;

    valid = stringToLongValue(str, &value);
    if (valid) {
        _this->max = value;
    }
    return valid;
}

static os_boolean
cfg_longElementCheckValue(
    cfg_longElement _this,
    const char *str)
{
    os_boolean valid;
    os_int64 value;

    valid = stringToLongValue(str, &value);
    if (valid) {
        if ((value < _this->min) || (value > _this->max)) {
            valid = OS_FALSE;
        }
    }
    return valid;
}

cfg_sizeElement
cfg_sizeElementNew(void)
{
    cfg_sizeElement _this = os_malloc(C_SIZEOF(cfg_sizeElement));

    cfg_elementInit(cfg_element(_this), CFG_ELEMENT_KIND_SIZE, cfg_elementDeinit);
    _this->min = CFG_SIZE_MIN;
    _this->max = CFG_SIZE_MAX;

    return _this;
}

static os_boolean
cfg_sizeElementSetMinValue(
    cfg_sizeElement _this,
    const char *str)
{
    os_boolean valid;
    os_uint64 value;

    valid = stringToSizeValue(str, &value);
    if (valid) {
        _this->min = value;
    }
    return valid;
}

static os_boolean
cfg_sizeElementSetMaxValue(
    cfg_sizeElement _this,
    const char *str)
{
    os_boolean valid;
    os_uint64 value;

    valid = stringToSizeValue(str, &value);
    if (valid) {
        _this->max = value;
    }
    return valid;
}

static os_boolean
cfg_sizeElementCheckValue(
    cfg_sizeElement _this,
    const char *str)
{
    os_boolean valid;
    os_uint64 value;

    valid = stringToSizeValue(str, &value);
    if (valid) {
        if ((value < _this->min) || (value > _this->max)) {
            valid = OS_FALSE;
        }
    }
    return valid;
}


cfg_floatElement
cfg_floatElementNew(void)
{
    cfg_floatElement _this = os_malloc(C_SIZEOF(cfg_floatElement));

    cfg_elementInit(cfg_element(_this), CFG_ELEMENT_KIND_FLOAT, cfg_elementDeinit);
    _this->min = CFG_FLOAT_MIN;
    _this->max = CFG_FLOAT_MAX;

    return _this;
}

static os_boolean
cfg_floatElementSetMinValue(
    cfg_floatElement _this,
    const char *str)
{
    os_boolean valid;
    os_float value;

    valid = stringToFloatValue(str, &value);
    if (valid) {
        _this->min = value;
    }
    return valid;
}

static os_boolean
cfg_floatElementSetMaxValue(
    cfg_floatElement _this,
    const char *str)
{
    os_boolean valid;
    os_float value;

    valid = stringToFloatValue(str, &value);
    if (valid) {
        _this->max = value;
    }
    return valid;
}

static os_boolean
cfg_floatElementCheckValue(
    cfg_floatElement _this,
    const char *str)
{
    os_boolean valid;
    os_float value;

    valid = stringToFloatValue(str, &value);
    if (valid) {
        if ((value < _this->min) || (value > _this->max)) {
            valid = OS_FALSE;
        }
    }
    return valid;
}

cfg_doubleElement
cfg_doubleElementNew(void)
{
    cfg_doubleElement _this = os_malloc(C_SIZEOF(cfg_doubleElement));

    cfg_elementInit(cfg_element(_this), CFG_ELEMENT_KIND_DOUBLE, cfg_elementDeinit);
    _this->min = CFG_DOUBLE_MIN;
    _this->max = CFG_DOUBLE_MAX;

    return _this;
}

static os_boolean
cfg_doubleElementSetMinValue(
    cfg_doubleElement _this,
    const char *str)
{
    os_boolean valid;
    os_double value;

    valid = stringToDoubleValue(str, &value);
    if (valid) {
        _this->min = value;
    }
    return valid;
}

static os_boolean
cfg_doubleElementSetMaxValue(
    cfg_doubleElement _this,
    const char *str)
{
    os_boolean valid;
    os_double value;

    valid = stringToDoubleValue(str, &value);
    if (valid) {
        _this->max = value;
    }
    return valid;
}

static os_boolean
cfg_doubleElementCheckValue(
    cfg_doubleElement _this,
    const char *str)
{
    os_boolean valid;
    os_double value;

    valid = stringToDoubleValue(str, &value);
    if (valid) {
        if ((value < _this->min) || (value > _this->max)) {
            valid = OS_FALSE;
        }
    }
    return valid;
}

static void
cfg_enumElementDeinit(
    cfg_node node)
{
    cfg_enumElement _this = cfg_enumElement(node);
    char *label;

    assert(node);

    label = (char *)c_iterTakeFirst(_this->labels);
    while (label) {
        os_free(label);
        label = (char *)c_iterTakeFirst(_this->labels);
    }
    c_iterFree(_this->labels);

    cfg_elementDeinit(node);
}


cfg_enumElement
cfg_enumElementNew(void)
{
    cfg_enumElement _this = os_malloc(C_SIZEOF(cfg_enumElement));

    cfg_elementInit(cfg_element(_this), CFG_ELEMENT_KIND_ENUM, cfg_enumElementDeinit);
    _this->labels = NULL;

    return _this;
}

static void
cfg_enumElementAddLabel(
    cfg_enumElement _this,
    const char *label)
{
    _this->labels = c_iterAppend(_this->labels, os_strdup(label));
}

struct CompareEnumLabelArg {
    const char *value;
    os_boolean found;
};


static os_boolean
compareEnumLabel(
    void *o,
    c_iterActionArg arg)
{
    char *label = o;
    struct CompareEnumLabelArg *info = arg;

    assert(label);
    assert(info);

    if (os_strcasecmp(label, info->value) == 0) {
        info->found = OS_TRUE;
        return OS_FALSE;
    }

    return OS_TRUE;
}


static os_boolean
cfg_enumLabelExist(
    c_iter labels,
    const char *lbl)
{
    os_boolean valid = OS_FALSE;
    struct CompareEnumLabelArg arg;

    if (lbl != NULL) {
        arg.found = OS_FALSE;
        arg.value = lbl;
        (void)c_iterWalkUntil(labels, (c_iterAction)compareEnumLabel, (c_iterActionArg)&arg);
        valid = arg.found;
    }

    return valid;
}

static os_boolean
cfg_enumElementCheckValue(
    cfg_enumElement _this,
    const char *str)
{
    os_boolean valid;

    valid = cfg_enumLabelExist(_this->labels, str);

    return valid;
}

static void
cfg_stringElementDeinit(
    cfg_node node)
{
    cfg_stringElement _this = cfg_stringElement(node);

    assert(_this);

    os_free(_this->min);
    os_free(_this->max);

    cfg_elementDeinit(node);
}

cfg_stringElement
cfg_stringElementNew(void)
{
    cfg_stringElement _this = os_malloc(C_SIZEOF(cfg_stringElement));

    cfg_elementInit(cfg_element(_this), CFG_ELEMENT_KIND_STRING, cfg_stringElementDeinit);
    _this->maxSize = 0;
    _this->min = NULL;
    _this->max = NULL;

    return _this;
}

static os_boolean
cfg_stringElementSetMaxSize(
    cfg_stringElement _this,
    const char *str)
{
    os_boolean valid;
    os_uint32 value;

    valid = stringToUIntValue(str, &value);
    if (valid) {
        _this->maxSize = value;
    }
    return valid;
}

static os_boolean
cfg_stringElementSetMinValue(
    cfg_stringElement _this,
    const char *min)
{
    _this->min = os_strdup(min);

    return OS_TRUE;
}

static os_boolean
cfg_stringElementSetMaxValue(
    cfg_stringElement _this,
    const char *max)
{
    assert(_this);
    assert(max);

    _this->max = os_strdup(max);

    return OS_TRUE;
}

static os_boolean
cfg_stringElementCheckValue(
    cfg_stringElement _this,
    const char *str)
{
    os_boolean valid = OS_TRUE;
    os_uint32 len;

    if (str) {
        if (_this->maxSize > 0) {
            len = (os_uint32) strlen(str);
            if (len > _this->maxSize) {
                valid = OS_FALSE;
            }
        }
    } else {
        valid = OS_FALSE;
    }

    return valid;
}

/************************************************************/

static void
cfg_attributeInit(
    cfg_attribute _this,
    cfg_attributeKind_t attributeKind,
    cfg_nodeDestructorFunc destructor)
{
    cfg_nodeInit(cfg_node(_this), CFG_NODE_KIND_ATTRIBUTE, destructor);
    _this->attributeKind = attributeKind;
    _this->required = OS_FALSE;
}

static void
cfg_attributeDeinit(
    cfg_node node)
{
    cfg_nodeDeinit(node);
}

os_boolean
cfg_attributeIsRequired(
    cfg_attribute _this)
{
    assert(_this);

    return _this->required;
}

cfg_booleanAttribute
cfg_booleanAttributeNew(void)
{
    cfg_booleanAttribute _this = os_malloc(C_SIZEOF(cfg_booleanAttribute));

    cfg_attributeInit(cfg_attribute(_this), CFG_ATTRIBUTE_KIND_BOOLEAN, cfg_attributeDeinit);

    return _this;
}

static os_boolean
cfg_booleanAttributeCheckValue(
    cfg_booleanAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_boolean value;

    OS_UNUSED_ARG(_this);

    valid = stringToBooleanValue(str, &value);

    return valid;
}

cfg_intAttribute
cfg_intAttributeNew(void)
{
    cfg_intAttribute _this = os_malloc(C_SIZEOF(cfg_intAttribute));

    cfg_attributeInit(cfg_attribute(_this), CFG_ATTRIBUTE_KIND_INT, cfg_attributeDeinit);
    _this->min = CFG_INT_MIN;
    _this->max = CFG_INT_MAX;

    return _this;
}

static os_boolean
cfg_intAttributeSetMinValue(
    cfg_intAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_int32 value;

    valid = stringToIntValue(str, &value);
    if (valid) {
        _this->min = value;
    }
    return valid;
}

static os_boolean
cfg_intAttributeSetMaxValue(
    cfg_intAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_int32 value;

    valid = stringToIntValue(str, &value);
    if (valid) {
        _this->max = value;
    }
    return valid;
}

static os_boolean
cfg_intAttributeCheckValue(
    cfg_intAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_int32 value;

    valid = stringToIntValue(str, &value);
    if (valid) {
        if ((value < _this->min) || (value > _this->max)) {
            valid = OS_FALSE;
        }
    }
    return valid;
}

cfg_longAttribute
cfg_longAttributeNew(void)
{
    cfg_longAttribute _this = os_malloc(C_SIZEOF(cfg_longAttribute));

    cfg_attributeInit(cfg_attribute(_this), CFG_ATTRIBUTE_KIND_LONG, cfg_attributeDeinit);
    _this->min = CFG_LONG_MIN;
    _this->max = CFG_LONG_MAX;

    return _this;
}

static os_boolean
cfg_longAttributeSetMinValue(
    cfg_longAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_int64 value;

    valid = stringToLongValue(str, &value);
    if (valid) {
        _this->min = value;
    }
    return valid;
}

static os_boolean
cfg_longAttributeSetMaxValue(
    cfg_longAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_int64 value;

    valid = stringToLongValue(str, &value);
    if (valid) {
        _this->max = value;
    }
    return valid;
}

static os_boolean
cfg_longAttributeCheckValue(
    cfg_longAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_int64 value;

    valid = stringToLongValue(str, &value);
    if (valid) {
        if ((value < _this->min) || (value > _this->max)) {
            valid = OS_FALSE;
        }
    }
    return valid;
}

cfg_sizeAttribute
cfg_sizeAttributeNew(void)
{
    cfg_sizeAttribute _this = os_malloc(C_SIZEOF(cfg_sizeAttribute));

    cfg_attributeInit(cfg_attribute(_this), CFG_ATTRIBUTE_KIND_SIZE, cfg_attributeDeinit);
    _this->min = CFG_SIZE_MIN;
    _this->max = CFG_SIZE_MAX;

    return _this;
}

static os_boolean
cfg_sizeAttributeSetMinValue(
    cfg_sizeAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_uint64 value;

    valid = stringToSizeValue(str, &value);
    if (valid) {
        _this->min = value;
    }
    return valid;
}

static os_boolean
cfg_sizeAttributeSetMaxValue(
    cfg_sizeAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_uint64 value;

    valid = stringToSizeValue(str, &value);
    if (valid) {
        _this->max = value;
    }
    return valid;
}

static os_boolean
cfg_sizeAttributeCheckValue(
    cfg_sizeAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_uint64 value;

    valid = stringToSizeValue(str, &value);
    if (valid) {
        if ((value < _this->min) || (value > _this->max)) {
            valid = OS_FALSE;
        }
    }
    return valid;
}


cfg_floatAttribute
cfg_floatAttributeNew(void)
{
    cfg_floatAttribute _this = os_malloc(C_SIZEOF(cfg_floatAttribute));

    cfg_attributeInit(cfg_attribute(_this), CFG_ATTRIBUTE_KIND_FLOAT, cfg_attributeDeinit);
    _this->min = CFG_FLOAT_MIN;
    _this->max = CFG_FLOAT_MAX;

    return _this;
}

static os_boolean
cfg_floatAttributeSetMinValue(
    cfg_floatAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_float value;

    valid = stringToFloatValue(str, &value);
    if (valid) {
        _this->min = value;
    }
    return valid;
}

static os_boolean
cfg_floatAttributeSetMaxValue(
    cfg_floatAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_float value;

    valid = stringToFloatValue(str, &value);
    if (valid) {
        _this->max = value;
    }
    return valid;
}

static os_boolean
cfg_floatAttributeCheckValue(
    cfg_floatAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_float value;

    valid = stringToFloatValue(str, &value);
    if (valid) {
        if ((value < _this->min) || (value > _this->max)) {
            valid = OS_FALSE;
        }
    }
    return valid;
}

cfg_doubleAttribute
cfg_doubleAttributeNew(void)
{
    cfg_doubleAttribute _this = os_malloc(C_SIZEOF(cfg_doubleAttribute));

    cfg_attributeInit(cfg_attribute(_this), CFG_ATTRIBUTE_KIND_DOUBLE, cfg_attributeDeinit);
    _this->min = CFG_DOUBLE_MIN;
    _this->max = CFG_DOUBLE_MAX;

    return _this;
}

static os_boolean
cfg_doubleAttributeSetMinValue(
    cfg_doubleAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_double value;

    valid = stringToDoubleValue(str, &value);
    if (valid) {
        _this->min = value;
    }
    return valid;
}

static os_boolean
cfg_doubleAttributeSetMaxValue(
    cfg_doubleAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_double value;

    valid = stringToDoubleValue(str, &value);
    if (valid) {
        _this->max = value;
    }
    return valid;
}

static os_boolean
cfg_doubleAttributeCheckValue(
    cfg_doubleAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_double value;

    valid = stringToDoubleValue(str, &value);
    if (valid) {
        if ((value < _this->min) || (value > _this->max)) {
            valid = OS_FALSE;
        }
    }
    return valid;
}

static void
cfg_enumAttributeDeinit(
    cfg_node node)
{
    cfg_enumAttribute _this = cfg_enumAttribute(node);
    char *label;

    assert(_this);

    label = (char *)c_iterTakeFirst(_this->labels);
    while (label) {
        os_free(label);
        label = (char *)c_iterTakeFirst(_this->labels);
    }
    c_iterFree(_this->labels);
    cfg_nodeDeinit(node);
}


cfg_enumAttribute
cfg_enumAttributeNew(void)
{
    cfg_enumAttribute _this = os_malloc(C_SIZEOF(cfg_enumAttribute));

    cfg_attributeInit(cfg_attribute(_this), CFG_ATTRIBUTE_KIND_ENUM, cfg_enumAttributeDeinit);
    _this->labels = NULL;

    return _this;
}

static void
cfg_enumAttributeAddLabel(
    cfg_enumAttribute _this,
    const char *label)
{
    _this->labels = c_iterAppend(_this->labels, os_strdup(label));
}

static os_boolean
cfg_enumAttributeCheckValue(
    cfg_enumAttribute _this,
    const char *str)
{
    os_boolean valid;

    valid = cfg_enumLabelExist(_this->labels, str);

    return valid;
}

static void
cfg_stringAttributeDeinit(
    cfg_node node)
{
    cfg_stringAttribute _this = cfg_stringAttribute(node);

    assert(_this);

    os_free(_this->min);
    os_free(_this->max);

    cfg_nodeDeinit(node);
}

cfg_stringAttribute
cfg_stringAttributeNew(void)
{
    cfg_stringAttribute _this = os_malloc(C_SIZEOF(cfg_stringAttribute));

    cfg_attributeInit(cfg_attribute(_this), CFG_ATTRIBUTE_KIND_STRING, cfg_stringAttributeDeinit);
    _this->maxSize = 0;
    _this->min = NULL;
    _this->max = NULL;

    return _this;
}

static os_boolean
cfg_stringAttributeSetMaxSize(
    cfg_stringAttribute _this,
    const char *str)
{
    os_boolean valid;
    os_uint32 value;

    valid = stringToUIntValue(str, &value);
    if (valid) {
        _this->maxSize = value;
    }
    return valid;
}

static os_boolean
cfg_stringAttributeSetMinValue(
    cfg_stringAttribute _this,
    const char *min)
{
    if (min) {
        _this->min = os_strdup(min);
    }

    return OS_TRUE;
}

static os_boolean
cfg_stringAttributeSetMaxValue(
    cfg_stringAttribute _this,
    const char *max)
{
    if (max) {
        _this->max = os_strdup(max);
    }

    return OS_TRUE;
}

static os_boolean
cfg_stringAttributeCheckValue(
    cfg_stringAttribute _this,
    const char *str)
{
    os_boolean valid = OS_TRUE;
    os_uint32 len;

    if (str) {
        if (_this->maxSize > 0) {
            len = (os_uint32) strlen(str);
            if (len > _this->maxSize) {
                valid = OS_FALSE;
            }
        }
    } else {
        valid = OS_FALSE;
    }

    return valid;
}

os_boolean
cfg_elementCheckValue(
    cfg_element _this,
    const char *str)
{
    os_boolean valid = OS_TRUE;

    if (str == NULL) {
        return OS_FALSE;
    }

    switch (_this->elementKind) {
    case CFG_ELEMENT_KIND_BOOLEAN:
        valid = cfg_booleanElementCheckValue(cfg_booleanElement(_this), str);
        break;
    case CFG_ELEMENT_KIND_INT:
        valid = cfg_intElementCheckValue(cfg_intElement(_this), str);
        break;
    case CFG_ELEMENT_KIND_LONG:
        valid = cfg_longElementCheckValue(cfg_longElement(_this), str);
        break;
    case CFG_ELEMENT_KIND_SIZE:
        valid = cfg_sizeElementCheckValue(cfg_sizeElement(_this), str);
        break;
    case CFG_ELEMENT_KIND_FLOAT:
        valid = cfg_floatElementCheckValue(cfg_floatElement(_this), str);
        break;
    case CFG_ELEMENT_KIND_DOUBLE:
        valid = cfg_doubleElementCheckValue(cfg_doubleElement(_this), str);
        break;
    case CFG_ELEMENT_KIND_ENUM:
        valid = cfg_enumElementCheckValue(cfg_enumElement(_this), str);
        break;
    case CFG_ELEMENT_KIND_STRING:
        valid = cfg_stringElementCheckValue(cfg_stringElement(_this), str);
        break;
    default:
        valid = OS_FALSE;
        break;
    }
    return valid;
}


os_boolean
cfg_attributeCheckValue(
    cfg_attribute _this,
    const char *str)
{
    os_boolean valid = OS_TRUE;

    if (str == NULL) {
        return OS_FALSE;
    }

    switch (_this->attributeKind) {
    case CFG_ATTRIBUTE_KIND_BOOLEAN:
        valid = cfg_booleanAttributeCheckValue(cfg_booleanAttribute(_this), str);
        break;
    case CFG_ATTRIBUTE_KIND_INT:
        valid = cfg_intAttributeCheckValue(cfg_intAttribute(_this), str);
        break;
    case CFG_ATTRIBUTE_KIND_LONG:
        valid = cfg_longAttributeCheckValue(cfg_longAttribute(_this), str);
        break;
    case CFG_ATTRIBUTE_KIND_SIZE:
        valid = cfg_sizeAttributeCheckValue(cfg_sizeAttribute(_this), str);
        break;
    case CFG_ATTRIBUTE_KIND_FLOAT:
        valid = cfg_floatAttributeCheckValue(cfg_floatAttribute(_this), str);
        break;
    case CFG_ATTRIBUTE_KIND_DOUBLE:
        valid = cfg_doubleAttributeCheckValue(cfg_doubleAttribute(_this), str);
        break;
    case CFG_ATTRIBUTE_KIND_ENUM:
        valid = cfg_enumAttributeCheckValue(cfg_enumAttribute(_this), str);
        break;
    case CFG_ATTRIBUTE_KIND_STRING:
        valid = cfg_stringAttributeCheckValue(cfg_stringAttribute(_this), str);
        break;
    default:
        valid = OS_FALSE;
        break;
    }
    return valid;
}

os_boolean
cfg_nodeSetMinOccurrences(
    cfg_node node,
    const char *image)
{
    os_boolean valid = OS_TRUE;
    os_uint32 value;

    valid = stringToUIntValue(image, &value);
    if (valid) {
        if (cfg_nodeIsElement(node)) {
            cfg_elementSetMinOccurrences(cfg_element(node), value);
        } else {
            if (value > 0) {
                cfg_attribute(node)->required = OS_TRUE;
            } else {
                cfg_attribute(node)->required = OS_FALSE;
            }
        }
    }

    return valid;
}

os_uint32
cfg_elementGetMinOccurrences(
    _In_ cfg_element _this)
{
    assert(_this);

    return _this->minOccurrences;
}

os_boolean
cfg_nodeSetMaxOccurrences(
    cfg_node node,
    const char *image)
{
    os_boolean valid = OS_TRUE;
    os_uint32 value;

    if (cfg_nodeIsElement(node)) {
        valid = stringToUIntValue(image, &value);
        if (valid) {
            cfg_elementSetMaxOccurrences(cfg_element(node), value);
        }
    }

    return valid;
}

os_uint32
cfg_elementGetMaxOccurrences(
    _In_ cfg_element _this)
{
    assert(_this);

    return _this->maxOccurrences;
}

os_boolean
cfg_nodeSetMinimum(
    cfg_node _this,
    const char *str)
{
    os_boolean valid = OS_TRUE;

    if (cfg_nodeIsElement(_this)) {
        switch (cfg_element(_this)->elementKind) {
        case CFG_ELEMENT_KIND_INT:
            valid = cfg_intElementSetMinValue(cfg_intElement(_this), str);
            break;
        case CFG_ELEMENT_KIND_LONG:
            valid = cfg_longElementSetMinValue(cfg_longElement(_this), str);
            break;
        case CFG_ELEMENT_KIND_SIZE:
            valid = cfg_sizeElementSetMinValue(cfg_sizeElement(_this), str);
            break;
        case CFG_ELEMENT_KIND_FLOAT:
            valid = cfg_floatElementSetMinValue(cfg_floatElement(_this), str);
            break;
        case CFG_ELEMENT_KIND_DOUBLE:
            valid = cfg_doubleElementSetMinValue(cfg_doubleElement(_this), str);
            break;
        case CFG_ELEMENT_KIND_STRING:
            valid = cfg_stringElementSetMinValue(cfg_stringElement(_this), str);
            break;
        default:
            assert(OS_FALSE);
            valid = OS_FALSE;
            break;
        }
    } else {
        switch (cfg_attribute(_this)->attributeKind) {
        case CFG_ATTRIBUTE_KIND_INT:
            valid = cfg_intAttributeSetMinValue(cfg_intAttribute(_this), str);
            break;
        case CFG_ATTRIBUTE_KIND_LONG:
            valid = cfg_longAttributeSetMinValue(cfg_longAttribute(_this), str);
            break;
        case CFG_ATTRIBUTE_KIND_SIZE:
            valid = cfg_sizeAttributeSetMinValue(cfg_sizeAttribute(_this), str);
            break;
        case CFG_ATTRIBUTE_KIND_FLOAT:
            valid = cfg_floatAttributeSetMinValue(cfg_floatAttribute(_this), str);
            break;
        case CFG_ATTRIBUTE_KIND_DOUBLE:
            valid = cfg_doubleAttributeSetMinValue(cfg_doubleAttribute(_this), str);
            break;
        case CFG_ATTRIBUTE_KIND_STRING:
            valid = cfg_stringAttributeSetMinValue(cfg_stringAttribute(_this), str);
            break;
        default:
            assert(OS_FALSE);
            valid = OS_FALSE;
            break;
        }
    }

    return valid;
}

os_boolean
cfg_nodeSetMaximum(
    cfg_node _this,
    const char *str)
{
    os_boolean valid = OS_TRUE;

    if (cfg_nodeIsElement(_this)) {
        switch (cfg_element(_this)->elementKind) {
        case CFG_ELEMENT_KIND_INT:
            valid = cfg_intElementSetMaxValue(cfg_intElement(_this), str);
            break;
        case CFG_ELEMENT_KIND_LONG:
            valid = cfg_longElementSetMaxValue(cfg_longElement(_this), str);
            break;
        case CFG_ELEMENT_KIND_SIZE:
            valid = cfg_sizeElementSetMaxValue(cfg_sizeElement(_this), str);
            break;
        case CFG_ELEMENT_KIND_FLOAT:
            valid = cfg_floatElementSetMaxValue(cfg_floatElement(_this), str);
            break;
        case CFG_ELEMENT_KIND_DOUBLE:
            valid = cfg_doubleElementSetMaxValue(cfg_doubleElement(_this), str);
            break;
        case CFG_ELEMENT_KIND_STRING:
            valid = cfg_stringElementSetMaxValue(cfg_stringElement(_this), str);
            break;
        default:
            assert(OS_FALSE);
            valid = OS_FALSE;
            break;
        }
    } else {
        switch (cfg_attribute(_this)->attributeKind) {
        case CFG_ATTRIBUTE_KIND_INT:
            valid = cfg_intAttributeSetMaxValue(cfg_intAttribute(_this), str);
            break;
        case CFG_ATTRIBUTE_KIND_LONG:
            valid = cfg_longAttributeSetMaxValue(cfg_longAttribute(_this), str);
            break;
        case CFG_ATTRIBUTE_KIND_SIZE:
            valid = cfg_sizeAttributeSetMaxValue(cfg_sizeAttribute(_this), str);
            break;
        case CFG_ATTRIBUTE_KIND_FLOAT:
            valid = cfg_floatAttributeSetMaxValue(cfg_floatAttribute(_this), str);
            break;
        case CFG_ATTRIBUTE_KIND_DOUBLE:
            valid = cfg_doubleAttributeSetMaxValue(cfg_doubleAttribute(_this), str);
            break;
        case CFG_ATTRIBUTE_KIND_STRING:
            valid = cfg_stringAttributeSetMaxValue(cfg_stringAttribute(_this), str);
            break;
        default:
            assert(OS_FALSE);
            valid = OS_FALSE;
            break;
        }
    }

    return valid;
}

os_boolean
cfg_nodeSetMaxLength(
    cfg_node _this,
    const char *image)
{
    os_boolean valid = OS_FALSE;

    if (cfg_nodeIsElement(_this)) {
        if (cfg_element(_this)->elementKind == CFG_ELEMENT_KIND_STRING) {
            valid = cfg_stringElementSetMaxSize(cfg_stringElement(_this), image);
        } else {
            valid = OS_FALSE;
        }
    } else {
        if (cfg_attribute(_this)->attributeKind == CFG_ATTRIBUTE_KIND_STRING) {
            valid = cfg_stringAttributeSetMaxSize(cfg_stringAttribute(_this), image);
        } else {
            valid = OS_FALSE;
        }
    }

    return valid;
}

os_boolean
cfg_nodeAddLabel(
    cfg_node _this,
    const char *label)
{
    os_boolean valid = OS_TRUE;

    if (cfg_nodeIsElement(_this)) {
        if (cfg_element(_this)->elementKind == CFG_ELEMENT_KIND_ENUM) {
            cfg_enumElementAddLabel(cfg_enumElement(_this), label);
        } else {
            valid = OS_FALSE;
        }
    } else {
        if (cfg_attribute(_this)->attributeKind == CFG_ATTRIBUTE_KIND_ENUM) {
            cfg_enumAttributeAddLabel(cfg_enumAttribute(_this), label);
        } else {
            valid = OS_FALSE;
        }
    }

    return valid;
}

os_boolean
cfg_nodeSetRequired(
    cfg_node node,
    const char *value)
{
    os_boolean valid = OS_TRUE;

    if (!cfg_nodeIsElement(node)) {
        if (os_strcasecmp(value, "true") == 0) {
            cfg_attribute(node)->required = OS_TRUE;
        } else if (os_strcasecmp(value, "false") == 0) {
            cfg_attribute(node)->required = OS_FALSE;
        } else {
            valid = OS_FALSE;
        }
    } else {
        valid = OS_FALSE;
    }

    return valid;
}

static os_boolean
stringToSizeValue(
    const char *image,
    os_uint64  *value)
{
    os_boolean valid = OS_TRUE;
    char *endptr;
    os_size_t len;

    *value = os_strtoull (image, &endptr, 0);
    len = strlen(endptr);
    if (len > 0) {
        switch (*endptr) {
        case 'K':
            (*value) *= 1024ULL;
            break;
        case 'M':
            (*value) *= 1024ULL * 1024UL;
            break;
        case 'G':
            (*value) *= 1024ULL * 1024UL * 1024UL;
            break;
        default:
            valid = OS_FALSE;
            break;
        }
    }

    return valid;
}

static os_boolean
stringToIntValue(
    const char *image,
    os_int32   *value)
{
    os_boolean valid = OS_TRUE;
    char *endptr;

    *value = (os_int32) strtol (image, &endptr, 0);
    if (*endptr != '\0') {
        valid = OS_FALSE;
    }

    return valid;
}

static os_boolean
stringToUIntValue(
    const char *image,
    os_uint32  *value)
{
    os_boolean valid = OS_TRUE;
    char *endptr;

    *value = (os_uint32) strtoul (image, &endptr, 0);
    if (*endptr != '\0') {
        valid = OS_FALSE;
    }

    return valid;
}

static os_boolean
stringToLongValue(
    const char *image,
    os_int64   *value)
{
    os_boolean valid = OS_TRUE;
    char *endptr;

    *value = os_strtoll (image, &endptr, 0);
    if (*endptr != '\0') {
        valid = OS_FALSE;
    }

    return valid;
}

static os_boolean
stringToFloatValue(
    const char *image,
    os_float   *value)
{
    os_boolean valid = OS_TRUE;
    char *endptr;

    *value = os_strtof (image, &endptr);
    if (*endptr != '\0') {
        valid = OS_FALSE;
    }

    return valid;
}

static os_boolean
stringToDoubleValue(
    const char *image,
    os_double  *value)
{
    os_boolean valid = OS_TRUE;
    char *endptr;

    *value = os_strtod (image, &endptr);
    if (*endptr != '\0') {
        valid = OS_FALSE;
    }

    return valid;
}

static os_boolean
stringToBooleanValue(
    const char *image,
    os_boolean *value)
{
    os_boolean valid = OS_TRUE;

    if (os_strncasecmp(image,"TRUE",5) == 0) {
        *value = OS_TRUE;
    } else if (os_strncasecmp(image,"FALSE",6) == 0) {
        *value = OS_FALSE;
    } else {
        valid = OS_FALSE;
    }

    return valid;
}


#ifdef CFG_DEBUG_CONFIG

static void
printIdent(
    os_uint32 level)
{
    printf("%*s", level<<1, "");
}

static void
printNode(
    void *item,
    c_iterActionArg arg)
{
    os_uint32 level = *(os_uint32 *) arg;
    cfg_nodePrint(cfg_node(item), level+1);
}

static void
printLabel(
    void *item,
    c_iterActionArg arg)
{
    char *label = item;
    os_uint32 level = *(os_uint32 *) arg;

    printIdent(level+1);
    printf("<value>%s</value>\n", label);
}


static const char *
elementName(
    cfg_elementKind_t kind)
{
#define _CASE_(k,n) case k: return #n
    switch(kind) {
    _CASE_(CFG_ELEMENT_KIND_ELEMENT,         "element");
    _CASE_(CFG_ELEMENT_KIND_EMPTY,           "leafEmpty");
    _CASE_(CFG_ELEMENT_KIND_BOOLEAN,         "leafBoolean");
    _CASE_(CFG_ELEMENT_KIND_INT,             "leafInt");
    _CASE_(CFG_ELEMENT_KIND_LONG,            "leafLong");
    _CASE_(CFG_ELEMENT_KIND_SIZE,            "leafSize");
    _CASE_(CFG_ELEMENT_KIND_FLOAT,           "leafFloat");
    _CASE_(CFG_ELEMENT_KIND_DOUBLE,          "leafDouble");
    _CASE_(CFG_ELEMENT_KIND_ENUM,            "leafEnum");
    _CASE_(CFG_ELEMENT_KIND_STRING,          "leafString");
    _CASE_(CFG_ELEMENT_KIND_SERVICE_MAPPING, "ServiceMapping");
    default:
        assert(OS_FALSE);
        break;
    }
#undef _CASE_
}

static const char *
attributeName(
    cfg_attributeKind_t kind)
{
#define _CASE_(k,n) case k: return #n
    switch(kind) {
    _CASE_(CFG_ATTRIBUTE_KIND_BOOLEAN,       "attributeBoolean");
    _CASE_(CFG_ATTRIBUTE_KIND_INT,           "attributeInt");
    _CASE_(CFG_ATTRIBUTE_KIND_LONG,          "attributeLong");
    _CASE_(CFG_ATTRIBUTE_KIND_SIZE,          "attributeSize");
    _CASE_(CFG_ATTRIBUTE_KIND_FLOAT,         "attributeFloat");
    _CASE_(CFG_ATTRIBUTE_KIND_DOUBLE,        "attributeDouble");
    _CASE_(CFG_ATTRIBUTE_KIND_ENUM,          "attributeEnum");
    _CASE_(CFG_ATTRIBUTE_KIND_STRING,        "attributeString");
    default:
        assert(OS_FALSE);
        break;
    }
#undef _CASE_
}

static const char *
nodeElementName(
    cfg_node node)
{
    if (node->kind == CFG_NODE_KIND_ELEMENT) {
        return elementName(cfg_element(node)->elementKind);
    } else {
        return attributeName(cfg_attribute(node)->attributeKind);
    }
}

void
cfg_nodePrint(
    cfg_node node,
    os_uint32 level)
{
    const char *elementName = nodeElementName(node);

    if (node->kind == CFG_NODE_KIND_ELEMENT) {
        printIdent(level);
        printf("<%s name=\"%s\" minOccurances=\"%u\" maxOccurences=\"%u\">\n",
                 elementName, node->name, cfg_element(node)->minOccurrences, cfg_element(node)->maxOccurrences);
        c_iterWalk(cfg_element(node)->children, printNode, &level);
        c_iterWalk(cfg_element(node)->attributes, printNode, &level);

        switch (cfg_element(node)->elementKind) {
        case CFG_ELEMENT_KIND_INT:
            printf("<minimum>%d</minimum>\n", cfg_intElement(node)->min);
            printf("<maximum>%d</maximum>\n", cfg_intElement(node)->max);
            break;
        case CFG_ELEMENT_KIND_LONG:
            printf("<minimum>%" PA_PRId64 "</minimum>\n", cfg_longElement(node)->min);
            printf("<maximum>%" PA_PRId64 "</maximum>\n", cfg_longElement(node)->max);
            break;
        case CFG_ELEMENT_KIND_SIZE:
            printf("<minimum>%" PA_PRIu64 "</minimum>\n", cfg_sizeElement(node)->min);
            printf("<maximum>%" PA_PRIu64 "</maximum>\n", cfg_sizeElement(node)->max);
            break;
        case CFG_ELEMENT_KIND_FLOAT:
            printf("<minimum>%g</minimum>\n", cfg_floatElement(node)->min);
            printf("<maximum>%g</maximum>\n", cfg_floatElement(node)->max);
            break;
        case CFG_ELEMENT_KIND_DOUBLE:
            printf("<minimum>%g</minimum>\n", cfg_doubleElement(node)->min);
            printf("<maximum>%g</maximum>\n", cfg_doubleElement(node)->max);
            break;
        case CFG_ELEMENT_KIND_ENUM:
            c_iterWalk(cfg_enumElement(node)->labels, printLabel, &level);
            break;
        case CFG_ELEMENT_KIND_STRING:
            {
                cfg_stringElement e = cfg_stringElement(node);

                printf("<minLength>%u</minLength>\n", e->maxSize);
                if (e->min) {
                    printIdent(level);
                    printf("<minimum>%s</minimum>\n", e->min);
                }

                if (e->max) {
                    printIdent(level);
                    printf("<maximum>%s</maximum>\n", e->max);
                }
            }
            break;
        case CFG_ELEMENT_KIND_SERVICE_MAPPING:
            break;
        default:
            break;
        }

        printIdent(level);
        printf("</%s>\n", elementName);

    } else {
         printIdent(level);
         printf("<%s name=\"%s\" required=\"%s\">\n",
                 elementName, node->name, (cfg_attribute(node)->required ? "true": "false"));

        switch (cfg_attribute(node)->attributeKind) {
        case CFG_ATTRIBUTE_KIND_INT:
            printf("<minimum>%d</minimum>\n", cfg_intAttribute(node)->min);
            printf("<maximum>%d</maximum>\n", cfg_intAttribute(node)->max);
            break;
        case CFG_ATTRIBUTE_KIND_LONG:
            printf("<minimum>%" PA_PRId64 "</minimum>\n", cfg_longAttribute(node)->min);
            printf("<maximum>%" PA_PRId64 "</maximum>\n", cfg_longAttribute(node)->max);
            break;
        case CFG_ATTRIBUTE_KIND_SIZE:
            printf("<minimum>%" PA_PRIu64 "</minimum>\n", cfg_sizeAttribute(node)->min);
            printf("<maximum>%" PA_PRIu64 "</maximum>\n", cfg_sizeAttribute(node)->max);
            break;
        case CFG_ATTRIBUTE_KIND_FLOAT:
            printf("<minimum>%g</minimum>\n", cfg_floatAttribute(node)->min);
            printf("<maximum>%g</maximum>\n", cfg_floatAttribute(node)->max);
            break;
        case CFG_ATTRIBUTE_KIND_DOUBLE:
            printf("<minimum>%g</minimum>\n", cfg_doubleAttribute(node)->min);
            printf("<maximum>%g</maximum>\n", cfg_doubleAttribute(node)->max);
            break;
        case CFG_ATTRIBUTE_KIND_ENUM:
            c_iterWalk(cfg_enumAttribute(node)->labels, printLabel, &level);
            break;
        case CFG_ATTRIBUTE_KIND_STRING:
            {
                cfg_stringAttribute a = cfg_stringAttribute(node);

                printf("<minLength>%u</minLength>\n", a->maxSize);
                if (a->min) {
                    printIdent(level);
                    printf("<minimum>%s</minimum>\n", a->min);
                }

                if (a->max) {
                    printIdent(level);
                    printf("<maximum>%s</maximum>\n", a->max);
                }
            }
            break;
        default:
            assert(OS_FALSE);
            break;
        }

        printIdent(level);
        printf("</%s>\n", elementName);
    }
}
#endif
