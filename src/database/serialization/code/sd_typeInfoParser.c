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
#include "os_heap.h"

#include "sd_misc.h"
#include "sd_xmlParser.h"
#include "sd__resultCodes.h"
#include "sd_typeInfoParser.h"
#include <string.h>

/* for tracing, change to #define TRACE(p) p */
#define TRACE(p)

#define SET_XMLPARSER_ERROR(parser, errorType) \
    sd_xmlParserSetError(parser, SD_ERRNO(errorType), SD_MESSAGE(errorType));


C_CLASS(sd_typeInfo);
C_CLASS(sd_node);

typedef struct {
    sd_typeInfo          typeInfo;
    sd_node              node;
    sd_typeInfoCallback  callback;
    void                *arguments;
    c_bool               result;
} sd_nodeActionArg;

typedef c_bool (*sd_nodeAction)(sd_node node, void *arg);

C_STRUCT(sd_node) {
    sd_typeInfoKind kind;
    sd_nodeAction   initAction;
    sd_nodeAction   action;
};

C_CLASS(sd_element);
C_STRUCT(sd_element) {
    C_EXTENDS(sd_node);
    c_char *name;
};

C_CLASS(sd_module);
C_STRUCT(sd_module) {
    C_EXTENDS(sd_element);
    sd_list children;
};

C_CLASS(sd_enumLabel);
C_STRUCT(sd_enumLabel) {
    C_EXTENDS(sd_element);
    c_bool valueDefined;
    c_long value;
};

C_CLASS(sd_enum);
C_STRUCT(sd_enum) {
    C_EXTENDS(sd_element);
    sd_list elements;
};

C_CLASS(sd_member);
C_STRUCT(sd_member) {
    C_EXTENDS(sd_element);
    sd_node type;
};

C_CLASS(sd_struct);
C_STRUCT(sd_struct) {
    C_EXTENDS(sd_element);
    sd_list members;
};

C_CLASS(sd_template);
C_STRUCT(sd_template) {
    C_EXTENDS(sd_node);
    sd_node type;
    c_long  size;
};

C_CLASS(sd_string);
C_STRUCT(sd_string) {
    C_EXTENDS(sd_node);
    c_long  length;
};

C_CLASS(sd_unionSwitch);
C_STRUCT(sd_unionSwitch) {
    C_EXTENDS(sd_node);
    sd_node type;
};

C_CLASS(sd_unionLabel);
C_STRUCT(sd_unionLabel) {
    C_EXTENDS(sd_node);
    c_bool  isDefault;
    c_char *value;
};

C_CLASS(sd_unionCase);
C_STRUCT(sd_unionCase) {
    C_EXTENDS(sd_element);
    sd_node type;
    sd_list labels;
};

C_CLASS(sd_union);
C_STRUCT(sd_union) {
    C_EXTENDS(sd_element);
    sd_node discriminator;
    sd_list cases;
};

C_CLASS(sd_typeDef);
C_STRUCT(sd_typeDef) {
    C_EXTENDS(sd_element);
    sd_node type;
};

C_CLASS(sd_typeRef);
C_STRUCT(sd_typeRef) {
    C_EXTENDS(sd_element);
};

C_STRUCT(sd_typeInfo) {
    sd_module     root;
    sd_node       current;
    sd_list       stack;
    sd_list       emptyList;
    sd_errorInfo *errorInfo;
};

typedef c_bool (*sd_typeInfoHandler)(sd_typeInfo info, sd_list attributes, c_bool start, sd_xmlParser handle);


#define sd_node(s)              ((sd_node)(s))
#define sd_element(s)           ((sd_element)(s))
#define SD_KIND(s)              (((sd_node)(s))->kind)
#define SD_CURRENT(i)           ((i)->current)
#define SD_CURRENT_KIND(i)      (SD_KIND(SD_CURRENT(i)))
#define SD_CURRENT_IS_KIND(i,k) (SD_CURRENT_KIND(i) == k)

static void
sd_nodeFree (
    sd_node node);

static c_bool
walkModules (
    void *obj,
    void *arg);

static void
sd_nodeInit (
    sd_node         node,
    sd_typeInfoKind kind,
    sd_nodeAction   initAction,
    sd_nodeAction   action)
{
    node->kind       = kind;
    node->initAction = initAction;
    node->action     = action;
}

static void
sd_nodeDeinit (
    sd_node node)
{
    os_free(node);
}

static sd_node
sd_nodeNew (
    sd_typeInfoKind kind,
    sd_nodeAction   initAction,
    sd_nodeAction   action)
{
    sd_node node;

    switch ( kind ) {
        case SD_TYPEINFO_KIND_CHAR:
        case SD_TYPEINFO_KIND_BOOLEAN:
        case SD_TYPEINFO_KIND_OCTET:
        case SD_TYPEINFO_KIND_SHORT:
        case SD_TYPEINFO_KIND_USHORT:
        case SD_TYPEINFO_KIND_LONG:
        case SD_TYPEINFO_KIND_ULONG:
        case SD_TYPEINFO_KIND_LONGLONG:
        case SD_TYPEINFO_KIND_ULONGLONG:
        case SD_TYPEINFO_KIND_FLOAT:
        case SD_TYPEINFO_KIND_DOUBLE:
        case SD_TYPEINFO_KIND_TIME:
            node = os_malloc(C_SIZEOF(sd_node));
            memset(node, 0, C_SIZEOF(sd_node));
            sd_nodeInit(node, kind, initAction, action);
            break;
        default:
            assert(0);
            node = NULL;
            break;
    }

    return node;
}

static void
sd_elementInit (
    sd_element       element,
    sd_typeInfoKind  kind,
    const c_char    *name,
    sd_nodeAction    initAction,
    sd_nodeAction    action)
{
    sd_nodeInit((sd_node)element, kind, initAction, action);
    element->name = sd_stringDup(name);
}



static void
sd_elementDeinit (
    sd_element element)
{
    if ( element->name ) {
        os_free(element->name);
    }
    sd_nodeDeinit((sd_node)element);
}

static c_bool
sd_nodeWalkAction (
    void *obj,
    void *arg)
{
    sd_node           node = obj;
    sd_nodeActionArg *info = arg;

    assert(node);
    assert(node->initAction);
    assert(info);
    assert(info->callback);

    info->result = node->initAction(node, info);

    return info->result;
}

static c_bool
sd_nodeDefaultAction (
    sd_node node,
    void    *arg)
{
    assert(node);
    assert(arg);
    OS_UNUSED_ARG(node);
    OS_UNUSED_ARG(arg);

    return TRUE;
}

static c_bool
sd_nodeDefaultInitAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info = arg;
    sd_list           attributes;
    c_bool            result     = FALSE;

    assert(node);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    attributes = info->typeInfo->emptyList;
    assert(attributes);

    info->node = node;

    if ( info->callback ) {
        result = info->callback(node->kind, NULL, attributes, info->arguments, info);
    }

    return result;
}

static c_bool
sd_elementDefaultAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info       = arg;
    sd_element        element    = (sd_element) node;
    c_bool            result     = FALSE;
    sd_list           attributes;

    assert(node);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    attributes = info->typeInfo->emptyList;
    assert(attributes);

    info->node = node;

    if ( info->callback ) {
        result = info->callback(node->kind, element->name, attributes, info->arguments, info);
    }

    return result;
}

static c_bool
sd_moduleAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info       = arg;
    sd_module         pmodule    = (sd_module) node;

    assert(pmodule);
    assert(node->kind == SD_TYPEINFO_KIND_MODULE);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    sd_listWalk(pmodule->children, sd_nodeWalkAction, info);

    return info->result;
}

static void
sd_moduleFree (
    sd_module pmodule)
{
    if ( pmodule ) {
        if ( pmodule->children ) {
            while ( !sd_listIsEmpty(pmodule->children) ) {
                sd_nodeFree((sd_node)sd_listTakeFirst(pmodule->children));
            }
            sd_listFree(pmodule->children);
        }
        sd_elementDeinit((sd_element)pmodule);
    }
}

static sd_module
sd_moduleNew (
    c_char *name)
{
    sd_module pmodule;
    pmodule = os_malloc(C_SIZEOF(sd_module));
    memset(pmodule, 0, C_SIZEOF(sd_module));
    sd_elementInit((sd_element)pmodule, SD_TYPEINFO_KIND_MODULE, name, sd_elementDefaultAction, sd_moduleAction);
    pmodule->children = sd_listNew();
    return pmodule;
}

static c_bool
sd_moduleAddChild (
    sd_module pmodule,
    sd_node   child)
{
    c_bool result = FALSE;

    assert(pmodule);
    assert(SD_KIND(pmodule) == SD_TYPEINFO_KIND_MODULE);

    switch ( child->kind ) {
        case SD_TYPEINFO_KIND_MODULE:
        case SD_TYPEINFO_KIND_STRUCT:
        case SD_TYPEINFO_KIND_UNION:
        case SD_TYPEINFO_KIND_ENUM:
        case SD_TYPEINFO_KIND_TYPEDEF:
            sd_listAppend(pmodule->children, child);
            result = TRUE;
            break;
        default:
            break;
    }

    return result;
}




static c_bool
sd_structAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info       = arg;
    sd_struct         pstruct    = (sd_struct) node;

    assert(node);
    assert(node->kind == SD_TYPEINFO_KIND_STRUCT);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    sd_listWalk(pstruct->members, sd_nodeWalkAction, info);

    return info->result;
}

static void
sd_structFree (
    sd_struct pstruct)
{
    if ( pstruct ) {
        if ( pstruct->members ) {
            while ( !sd_listIsEmpty(pstruct->members) ) {
                sd_nodeFree((sd_node)sd_listTakeFirst(pstruct->members));
            }
            sd_listFree(pstruct->members);
        }
        sd_elementDeinit((sd_element)pstruct);
    }
}

static sd_struct
sd_structNew (
    c_char *name)
{
    sd_struct pstruct = os_malloc(C_SIZEOF(sd_struct));
    memset(pstruct, 0, C_SIZEOF(sd_struct));
    sd_elementInit((sd_element)pstruct, SD_TYPEINFO_KIND_STRUCT, name, sd_elementDefaultAction, sd_structAction);
    pstruct->members = sd_listNew();
    return pstruct;
}

static c_bool
sd_structAddChild (
    sd_struct pstruct,
    sd_node   child)
{
    c_bool result = FALSE;

    assert(pstruct);
    assert(SD_KIND(pstruct) == SD_TYPEINFO_KIND_STRUCT);

    switch ( child->kind ) {
        case SD_TYPEINFO_KIND_MEMBER:
            sd_listAppend(pstruct->members, child);
            result = TRUE;
            break;
        default:
            break;
    }

    return result;
}

static c_bool
sd_memberAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info       = arg;
    sd_member         pmember    = (sd_member) node;
    c_bool            result     = FALSE;

    assert(node);
    assert(node->kind == SD_TYPEINFO_KIND_MEMBER);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    result = pmember->type->initAction(pmember->type, info);

    return result;
}

static void
sd_memberFree (
    sd_member pmember)
{
    if ( pmember ) {
        if ( pmember->type ) {
            sd_nodeFree(pmember->type);
        }
        sd_elementDeinit((sd_element)pmember);
    }
}

static sd_member
sd_memberNew (
    c_char *name)
{
    sd_member pmember = os_malloc(C_SIZEOF(sd_member));
    memset(pmember, 0, C_SIZEOF(sd_member));
    sd_elementInit((sd_element)pmember, SD_TYPEINFO_KIND_MEMBER, name, sd_elementDefaultAction, sd_memberAction);
    return pmember;
}

static c_bool
sd_memberAddChild (
    sd_member pmember,
    sd_node   child)
{
    c_bool result = FALSE;

    assert(pmember);
    assert(SD_KIND(pmember) == SD_TYPEINFO_KIND_MEMBER);

    switch ( child->kind ) {
        case SD_TYPEINFO_KIND_STRUCT:
        case SD_TYPEINFO_KIND_UNION:
        case SD_TYPEINFO_KIND_ENUM:
        case SD_TYPEINFO_KIND_TYPE:
        case SD_TYPEINFO_KIND_ARRAY:
        case SD_TYPEINFO_KIND_SEQUENCE:
        case SD_TYPEINFO_KIND_STRING:
        case SD_TYPEINFO_KIND_CHAR:
        case SD_TYPEINFO_KIND_BOOLEAN:
        case SD_TYPEINFO_KIND_OCTET:
        case SD_TYPEINFO_KIND_SHORT:
        case SD_TYPEINFO_KIND_USHORT:
        case SD_TYPEINFO_KIND_LONG:
        case SD_TYPEINFO_KIND_ULONG:
        case SD_TYPEINFO_KIND_LONGLONG:
        case SD_TYPEINFO_KIND_ULONGLONG:
        case SD_TYPEINFO_KIND_FLOAT:
        case SD_TYPEINFO_KIND_DOUBLE:
        case SD_TYPEINFO_KIND_TIME:
            pmember->type = child;
            result = TRUE;
            break;
        default:
            break;
    }

    return result;
}

static c_bool
sd_unionAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info       = arg;
    sd_union          punion     = (sd_union) node;

    assert(node);
    assert(node->kind == SD_TYPEINFO_KIND_UNION);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    info->result = punion->discriminator->initAction(punion->discriminator, info);
    if ( info->result ) {
        sd_listWalk(punion->cases, sd_nodeWalkAction, info);
    }

    return info->result;
}

static void
sd_unionFree (
    sd_union punion)
{
    if ( punion ) {
        if ( punion->discriminator ) {
            sd_nodeFree(punion->discriminator);
        }
        if ( punion->cases ) {
            while ( !sd_listIsEmpty(punion->cases) ) {
                sd_nodeFree((sd_node)sd_listTakeFirst(punion->cases));
            }
            sd_listFree(punion->cases);
        }
        sd_elementDeinit((sd_element)punion);
    }
}

static sd_union
sd_unionNew (
    c_char *name)
{
    sd_union punion = os_malloc(C_SIZEOF(sd_union));
    memset(punion, 0, C_SIZEOF(sd_union));
    sd_elementInit((sd_element)punion, SD_TYPEINFO_KIND_UNION, name, sd_elementDefaultAction, sd_unionAction);
    punion->cases = sd_listNew();
    return punion;
}

static c_bool
sd_unionAddChild (
    sd_union punion,
    sd_node  child)
{
    c_bool result = FALSE;

    assert(punion);
    assert(SD_KIND(punion) == SD_TYPEINFO_KIND_UNION);

    switch ( child->kind ) {
        case SD_TYPEINFO_KIND_UNIONSWITCH:
            if ( !punion->discriminator ) {
                punion->discriminator = child;
                result = TRUE;
            }
            break;
        case SD_TYPEINFO_KIND_UNIONCASE:
            if ( punion->discriminator ) {
                sd_listAppend(punion->cases, child);
                result = TRUE;
            }
            break;
        default:
            break;
    }

    return result;
}

static c_bool
sd_unionCaseAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info       = arg;
    sd_unionCase      punionCase = (sd_unionCase) node;

    assert(node);
    assert(node->kind == SD_TYPEINFO_KIND_UNIONCASE);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    info->result = punionCase->type->initAction(punionCase->type, info);
    if ( info->result ) {
        sd_listWalk(punionCase->labels, sd_nodeWalkAction, info);
    }

    return info->result;
}

static void
sd_unionCaseFree (
    sd_unionCase punionCase)
{
    if ( punionCase ) {
        if ( punionCase->type ) {
            sd_nodeFree(punionCase->type);
        }
        if ( punionCase->labels ) {
            while ( !sd_listIsEmpty(punionCase->labels) ) {
                sd_nodeFree((sd_node)sd_listTakeFirst(punionCase->labels));
            }
            sd_listFree(punionCase->labels);
        }
        sd_elementDeinit((sd_element)punionCase);
    }
}

static sd_unionCase
sd_unionCaseNew (
    c_char *name)
{
    sd_unionCase punionCase = os_malloc(C_SIZEOF(sd_unionCase));
    memset(punionCase, 0, C_SIZEOF(sd_unionCase));
    sd_elementInit((sd_element)punionCase, SD_TYPEINFO_KIND_UNIONCASE, name, sd_elementDefaultAction, sd_unionCaseAction);
    punionCase->labels = sd_listNew();
    return punionCase;
}

static c_bool
sd_unionCaseAddChild (
    sd_unionCase punionCase,
    sd_node      child)
{
    c_bool result = FALSE;

    assert(punionCase);
    assert(SD_KIND(punionCase) == SD_TYPEINFO_KIND_UNIONCASE);

    switch ( child->kind ) {
        case SD_TYPEINFO_KIND_STRUCT:
        case SD_TYPEINFO_KIND_UNION:
        case SD_TYPEINFO_KIND_ENUM:
        case SD_TYPEINFO_KIND_TYPE:
        case SD_TYPEINFO_KIND_ARRAY:
        case SD_TYPEINFO_KIND_SEQUENCE:
        case SD_TYPEINFO_KIND_STRING:
        case SD_TYPEINFO_KIND_CHAR:
        case SD_TYPEINFO_KIND_BOOLEAN:
        case SD_TYPEINFO_KIND_OCTET:
        case SD_TYPEINFO_KIND_SHORT:
        case SD_TYPEINFO_KIND_USHORT:
        case SD_TYPEINFO_KIND_LONG:
        case SD_TYPEINFO_KIND_ULONG:
        case SD_TYPEINFO_KIND_LONGLONG:
        case SD_TYPEINFO_KIND_ULONGLONG:
        case SD_TYPEINFO_KIND_FLOAT:
        case SD_TYPEINFO_KIND_DOUBLE:
        case SD_TYPEINFO_KIND_TIME:
            if ( !punionCase->type ) {
                punionCase->type = child;
                result = TRUE;
            }
            break;
        case SD_TYPEINFO_KIND_UNIONLABEL:
            if ( punionCase->type ) {
                sd_listAppend(punionCase->labels, child);
                result = TRUE;
            }
            break;
        default:
            break;
    }

    return result;
}

static c_bool
sd_unionLabelAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg    *info        = arg;
    sd_unionLabel        punionLabel = (sd_unionLabel) node;
    c_bool               result      = FALSE;
    sd_list              attributes;
    sd_typeInfoAttribute attribute;

    assert(node);
    assert(node->kind == SD_TYPEINFO_KIND_UNIONLABEL);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    info->node = node;

    if ( info->callback ) {
        if ( !punionLabel->isDefault ) {
            attributes  = sd_listNew();

            if ( attributes ) {
                attribute.name     = "value";
                attribute.d        = SD_TYPEINFO_ATTR_KIND_STRING;
                attribute.u.svalue = punionLabel->value;
                sd_listAppend(attributes, &attribute);

                result = info->callback(node->kind, NULL, attributes, info->arguments, info);
            }
            sd_listFree(attributes);
        } else {
            attributes = info->typeInfo->emptyList;
            result = info->callback(SD_TYPEINFO_KIND_UNIONLABELDEFAULT, NULL, attributes, info->arguments, info);
         }
    }

    return result;
}

static void
sd_unionLabelFree (
    sd_unionLabel punionLabel)
{
    if ( punionLabel ) {
        if ( punionLabel->value ) {
            os_free(punionLabel->value);
        }
        sd_nodeDeinit((sd_node)punionLabel);
    }
}

static sd_unionLabel
sd_unionLabelNew (
    c_char *value)
{
    sd_unionLabel punionLabel = os_malloc(C_SIZEOF(sd_unionLabel));
    memset(punionLabel, 0, C_SIZEOF(sd_unionLabel));
    sd_nodeInit((sd_node)punionLabel, SD_TYPEINFO_KIND_UNIONLABEL, sd_unionLabelAction, sd_nodeDefaultAction);
    punionLabel->value = sd_stringDup(value);
    return punionLabel;
}

static c_bool
sd_unionSwitchInitAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg    *info        = arg;
    c_bool               result      = FALSE;
    sd_list              attributes;

    assert(node);
    assert(node->kind == SD_TYPEINFO_KIND_UNIONSWITCH);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    info->node = node;
    attributes = info->typeInfo->emptyList;

    if ( info->callback ) {
        result = info->callback(node->kind, NULL, attributes, info->arguments, info);
    }

    return result;
}

static c_bool
sd_unionSwitchAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info         = arg;
    sd_unionSwitch    punionSwitch = (sd_unionSwitch) node;
    c_bool            result       = FALSE;

    assert(node);
    assert(node->kind == SD_TYPEINFO_KIND_UNIONSWITCH);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    result = punionSwitch->type->initAction(punionSwitch->type, info);

    return result;
}

static void
sd_unionSwitchFree (
    sd_unionSwitch punionSwitch)
{
    if ( punionSwitch ) {
        if ( punionSwitch->type ) {
            sd_nodeFree(punionSwitch->type);
        }
        sd_nodeDeinit((sd_node)punionSwitch);
    }
}

static sd_unionSwitch
sd_unionSwitchNew (
    void )
{
    sd_unionSwitch punionSwitch = os_malloc(C_SIZEOF(sd_unionSwitch));
    memset(punionSwitch, 0, C_SIZEOF(sd_unionSwitch));
    sd_nodeInit((sd_node)punionSwitch, SD_TYPEINFO_KIND_UNIONSWITCH, sd_unionSwitchInitAction, sd_unionSwitchAction);
    return punionSwitch;
}

static c_bool
sd_unionSwitchAddChild (
    sd_unionSwitch punionSwitch,
    sd_node   child)
{
    c_bool result = FALSE;

    assert(punionSwitch);
    assert(SD_KIND(punionSwitch) == SD_TYPEINFO_KIND_UNIONSWITCH);

    if ( !punionSwitch->type ) {
        switch ( child->kind ) {
            case SD_TYPEINFO_KIND_TYPE:
            case SD_TYPEINFO_KIND_ENUM:
            case SD_TYPEINFO_KIND_CHAR:
            case SD_TYPEINFO_KIND_BOOLEAN:
            case SD_TYPEINFO_KIND_OCTET:
            case SD_TYPEINFO_KIND_SHORT:
            case SD_TYPEINFO_KIND_USHORT:
            case SD_TYPEINFO_KIND_LONG:
            case SD_TYPEINFO_KIND_ULONG:
            case SD_TYPEINFO_KIND_LONGLONG:
            case SD_TYPEINFO_KIND_ULONGLONG:
                punionSwitch->type = child;
                result = TRUE;
                break;
            default:
                break;
        }
    }

    return result;
}


static c_bool
sd_typeDefAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info       = arg;
    sd_typeDef        ptypeDef   = (sd_typeDef) node;
    c_bool            result     = FALSE;

    assert(node);
    assert(node->kind == SD_TYPEINFO_KIND_TYPEDEF);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    result = ptypeDef->type->initAction(ptypeDef->type, info);

    return result;
}

static void
sd_typeDefFree (
    sd_typeDef ptypeDef)
{
    if ( ptypeDef ) {
        if ( ptypeDef->type ) {
            sd_nodeFree(ptypeDef->type);
        }
        sd_elementDeinit((sd_element)ptypeDef);
    }
}

static sd_typeDef
sd_typeDefNew (
    c_char *name)
{
    sd_typeDef ptypeDef = os_malloc(C_SIZEOF(sd_typeDef));
    memset(ptypeDef, 0, C_SIZEOF(sd_typeDef));
    sd_elementInit((sd_element)ptypeDef, SD_TYPEINFO_KIND_TYPEDEF, name, sd_elementDefaultAction, sd_typeDefAction);
    return ptypeDef;
}

static c_bool
sd_typeDefAddChild (
    sd_typeDef ptypeDef,
    sd_node   child)
{
    c_bool result = FALSE;

    assert(ptypeDef);
    assert(SD_KIND(ptypeDef) == SD_TYPEINFO_KIND_TYPEDEF);

    if ( !ptypeDef->type ) {
        switch ( child->kind ) {
            case SD_TYPEINFO_KIND_STRUCT:
            case SD_TYPEINFO_KIND_UNION:
            case SD_TYPEINFO_KIND_ENUM:
            case SD_TYPEINFO_KIND_TYPE:
            case SD_TYPEINFO_KIND_STRING:
            case SD_TYPEINFO_KIND_ARRAY:
            case SD_TYPEINFO_KIND_SEQUENCE:
            case SD_TYPEINFO_KIND_CHAR:
            case SD_TYPEINFO_KIND_BOOLEAN:
            case SD_TYPEINFO_KIND_OCTET:
            case SD_TYPEINFO_KIND_SHORT:
            case SD_TYPEINFO_KIND_USHORT:
            case SD_TYPEINFO_KIND_LONG:
            case SD_TYPEINFO_KIND_ULONG:
            case SD_TYPEINFO_KIND_LONGLONG:
            case SD_TYPEINFO_KIND_ULONGLONG:
            case SD_TYPEINFO_KIND_FLOAT:
            case SD_TYPEINFO_KIND_DOUBLE:
            case SD_TYPEINFO_KIND_TIME:
                ptypeDef->type = child;
                result = TRUE;
                break;
            default:
                break;
        }
    }

    return result;
}

static c_bool
sd_enumAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info       = arg;
    sd_enum           penum      = (sd_enum) node;

    assert(node);
    assert(node->kind == SD_TYPEINFO_KIND_ENUM);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    sd_listWalk(penum->elements, sd_nodeWalkAction, info);

    return info->result;
}

static void
sd_enumFree (
    sd_enum penum)
{
    if ( penum ) {
        if ( penum->elements ) {
            while ( !sd_listIsEmpty(penum->elements) ) {
                sd_nodeFree((sd_node)sd_listTakeFirst(penum->elements));
            }
            sd_listFree(penum->elements);
        }
        sd_elementDeinit((sd_element)penum);
    }
}

static sd_enum
sd_enumNew (
    c_char *name)
{
    sd_enum penum = os_malloc(C_SIZEOF(sd_enum));
    memset(penum, 0, C_SIZEOF(sd_enum));
    sd_elementInit((sd_element)penum, SD_TYPEINFO_KIND_ENUM, name, sd_elementDefaultAction, sd_enumAction);
    penum->elements = sd_listNew();
    return penum;
}

static c_bool
sd_enumAddChild (
    sd_enum penum,
    sd_node child)
{
    c_bool result = FALSE;

    assert(penum);
    assert(SD_KIND(penum) == SD_TYPEINFO_KIND_ENUM);

    switch ( child->kind ) {
        case SD_TYPEINFO_KIND_ENUMLABEL:
            sd_listAppend(penum->elements, child);
            result = TRUE;
            break;
        default:
            break;
    }

    return result;
}

static c_bool
sd_enumLabelAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg    *info        = arg;
    sd_enumLabel         penumLabel  = (sd_enumLabel) node;
    c_bool               result      = FALSE;
    sd_list              attributes  = sd_listNew();
    sd_typeInfoAttribute attribute;

    assert(node);
    assert(node->kind == SD_TYPEINFO_KIND_ENUMLABEL);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    info->node = node;

    if ( info->callback ) {
        if ( attributes ) {
            if ( penumLabel->valueDefined ) {
                attribute.name     = "value";
                attribute.d        = SD_TYPEINFO_ATTR_KIND_NUMBER;
                attribute.u.nvalue = penumLabel->value;
                sd_listAppend(attributes, &attribute);
            }
            result = info->callback(node->kind, sd_element(node)->name, attributes, info->arguments, info);
        }
        sd_listFree(attributes);
    }

    return result;
}

static void
sd_enumLabelFree (
    sd_enumLabel penumLabel)
{
    if ( penumLabel ) {
        sd_elementDeinit((sd_element)penumLabel);
    }
}

static sd_enumLabel
sd_enumLabelNew (
    c_char *name,
    c_bool  valueDefined,
    c_long  value)
{
    sd_enumLabel penumLabel = os_malloc(C_SIZEOF(sd_enumLabel));
    memset(penumLabel, 0, C_SIZEOF(sd_enumLabel));
    sd_elementInit((sd_element)penumLabel, SD_TYPEINFO_KIND_ENUMLABEL, name, sd_enumLabelAction, sd_nodeDefaultAction);
    if ( valueDefined ) {
        penumLabel->valueDefined = valueDefined;
        penumLabel->value        = value;
    }
    return penumLabel;
}

static void
sd_typeRefFree (
    sd_typeRef ptypeRef)
{
    if ( ptypeRef ) {
        sd_elementDeinit((sd_element)ptypeRef);
    }
}

static sd_typeRef
sd_typeRefNew (
    c_char *name)
{
    sd_typeRef ptypeRef = os_malloc(C_SIZEOF(sd_typeRef));
    memset(ptypeRef, 0, C_SIZEOF(sd_typeRef));
    sd_elementInit((sd_element)ptypeRef, SD_TYPEINFO_KIND_TYPE, name, sd_elementDefaultAction, sd_nodeDefaultAction);
    return ptypeRef;
}

static c_bool
sd_templateInitAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg    *info        = arg;
    sd_template          ptemplate   = (sd_template) node;
    c_bool               result      = FALSE;
    sd_list              attributes;
    sd_typeInfoAttribute attribute;

    assert(node);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    info->node = node;

    if ( info->callback ) {
        if ( ptemplate->size > 0 ) {
            attributes = sd_listNew();
            if ( attributes ) {
                attribute.name     = "size";
                attribute.d        = SD_TYPEINFO_ATTR_KIND_NUMBER;
                attribute.u.nvalue = ptemplate->size;
                sd_listAppend(attributes, &attribute);

                result = info->callback(node->kind, NULL, attributes, info->arguments, info);
                sd_listFree(attributes);
            }
        } else {
            attributes = info->typeInfo->emptyList;
            result = info->callback(node->kind, NULL, attributes, info->arguments, info);
        }
    }

    return result;
}

static c_bool
sd_templateAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg *info         = arg;
    sd_template       ptemplate    = (sd_template) node;
    c_bool            result       = FALSE;

    assert(node);
    assert((SD_KIND(ptemplate) == SD_TYPEINFO_KIND_ARRAY) ||
           (SD_KIND(ptemplate) == SD_TYPEINFO_KIND_SEQUENCE));
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    result = ptemplate->type->initAction(ptemplate->type, info);

    return result;
}

static void
sd_templateFree (
    sd_template ptemplate)
{
    if ( ptemplate ) {
        sd_nodeFree(ptemplate->type);
        sd_nodeDeinit((sd_node)ptemplate);
    }
}

static sd_template
sd_templateNew (
    sd_typeInfoKind kind,
    c_long          size)
{
    sd_template ptemplate = os_malloc(C_SIZEOF(sd_template));
    memset(ptemplate, 0, C_SIZEOF(sd_template));
    sd_nodeInit((sd_node)ptemplate, kind, sd_templateInitAction, sd_templateAction);
    ptemplate->size = size;
    return ptemplate;
}

static c_bool
sd_templateAddChild (
    sd_template ptemplate,
    sd_node     child)
{
    c_bool result = FALSE;

    assert(ptemplate);
    assert((SD_KIND(ptemplate) == SD_TYPEINFO_KIND_ARRAY) ||
           (SD_KIND(ptemplate) == SD_TYPEINFO_KIND_SEQUENCE));

    if ( !ptemplate->type ) {
        switch ( child->kind ) {
            case SD_TYPEINFO_KIND_STRUCT:
            case SD_TYPEINFO_KIND_UNION:
            case SD_TYPEINFO_KIND_ENUM:
            case SD_TYPEINFO_KIND_TYPE:
            case SD_TYPEINFO_KIND_STRING:
            case SD_TYPEINFO_KIND_ARRAY:
            case SD_TYPEINFO_KIND_SEQUENCE:
            case SD_TYPEINFO_KIND_CHAR:
            case SD_TYPEINFO_KIND_BOOLEAN:
            case SD_TYPEINFO_KIND_OCTET:
            case SD_TYPEINFO_KIND_SHORT:
            case SD_TYPEINFO_KIND_USHORT:
            case SD_TYPEINFO_KIND_LONG:
            case SD_TYPEINFO_KIND_ULONG:
            case SD_TYPEINFO_KIND_LONGLONG:
            case SD_TYPEINFO_KIND_ULONGLONG:
            case SD_TYPEINFO_KIND_FLOAT:
            case SD_TYPEINFO_KIND_DOUBLE:
            case SD_TYPEINFO_KIND_TIME:
                ptemplate->type = child;
                result = TRUE;
                break;
            default:
                break;
        }
    }

    return result;
}

static c_bool
sd_stringAction (
    sd_node node,
    void    *arg)
{
    sd_nodeActionArg    *info        = arg;
    sd_string            pstring     = (sd_string) node;
    c_bool               result      = FALSE;
    sd_list              attributes;
    sd_typeInfoAttribute attribute;

    assert(node);
    assert(info);
    assert(info->callback);
    assert(info->typeInfo);

    info->node = node;

    if ( info->callback ) {
        if ( pstring->length > 0 ) {
            attributes = sd_listNew();
            if ( attributes ) {
                attribute.name     = "length";
                attribute.d        = SD_TYPEINFO_ATTR_KIND_NUMBER;
                attribute.u.nvalue = pstring->length;
                sd_listAppend(attributes, &attribute);

                result = info->callback(node->kind, NULL, attributes, info->arguments, info);
                sd_listFree(attributes);
            }
        } else {
            attributes = info->typeInfo->emptyList;
            result = info->callback(node->kind, NULL, attributes, info->arguments, info);
        }
    }

    return result;
}

static void
sd_stringFree (
    sd_string pstring)
{
    if ( pstring ) {
        sd_nodeDeinit((sd_node)pstring);
    }
}

static sd_string
sd_stringNew (
    c_long length)
{
    sd_string pstring;
    pstring = os_malloc(C_SIZEOF(sd_string));
    memset(pstring, 0, C_SIZEOF(sd_string));
    sd_nodeInit((sd_node)pstring, SD_TYPEINFO_KIND_STRING, sd_stringAction, sd_nodeDefaultAction);
    pstring->length = length;
    return pstring;
}


static void
sd_nodeFree (
    sd_node node)
{
    if ( node ) {
        switch ( node->kind ) {
            case SD_TYPEINFO_KIND_MODULE:
                sd_moduleFree((sd_module)node);
                break;
            case SD_TYPEINFO_KIND_STRUCT:
                sd_structFree((sd_struct)node);
                break;
            case SD_TYPEINFO_KIND_MEMBER:
                sd_memberFree((sd_member)node);
                break;
            case SD_TYPEINFO_KIND_UNION:
                sd_unionFree((sd_union)node);
                break;
            case SD_TYPEINFO_KIND_UNIONCASE:
                sd_unionCaseFree((sd_unionCase)node);
                break;
            case SD_TYPEINFO_KIND_UNIONSWITCH:
                sd_unionSwitchFree((sd_unionSwitch)node);
                break;
            case SD_TYPEINFO_KIND_UNIONLABEL:
                sd_unionLabelFree((sd_unionLabel)node);
                break;
            case SD_TYPEINFO_KIND_TYPEDEF:
                sd_typeDefFree((sd_typeDef)node);
                break;
            case SD_TYPEINFO_KIND_ENUM:
                sd_enumFree((sd_enum)node);
                break;
            case SD_TYPEINFO_KIND_ENUMLABEL:
                sd_enumLabelFree((sd_enumLabel)node);
                break;
            case SD_TYPEINFO_KIND_TYPE:
                sd_typeRefFree((sd_typeRef)node);
                break;
            case SD_TYPEINFO_KIND_ARRAY:
            case SD_TYPEINFO_KIND_SEQUENCE:
                sd_templateFree((sd_template)node);
                break;
            case SD_TYPEINFO_KIND_STRING:
                sd_stringFree((sd_string)node);
                break;
            case SD_TYPEINFO_KIND_CHAR:
            case SD_TYPEINFO_KIND_BOOLEAN:
            case SD_TYPEINFO_KIND_OCTET:
            case SD_TYPEINFO_KIND_SHORT:
            case SD_TYPEINFO_KIND_USHORT:
            case SD_TYPEINFO_KIND_LONG:
            case SD_TYPEINFO_KIND_ULONG:
            case SD_TYPEINFO_KIND_LONGLONG:
            case SD_TYPEINFO_KIND_ULONGLONG:
            case SD_TYPEINFO_KIND_FLOAT:
            case SD_TYPEINFO_KIND_DOUBLE:
            case SD_TYPEINFO_KIND_TIME:
                sd_nodeDeinit(node);
                break;
            default:
                assert(0);
                break;
        }
    }
}


static void
sd_typeInfoFree (
    sd_typeInfo info)
{
    assert(info);

    if ( info->stack ) {
        sd_listFree(info->stack);
    }

    if ( info->emptyList ) {
        sd_listFree(info->emptyList);
    }

    if ( info->root ) {
        sd_moduleFree(info->root);
    }

    os_free(info);
}

static void
sd_typeInfoPush (
    sd_typeInfo typeInfo,
    sd_node     node)
{
    sd_listInsert(typeInfo->stack, node);
    typeInfo->current = node;
}

static sd_node
sd_typeInfoPop (
    sd_typeInfo typeInfo)
{
    sd_node node = NULL;

    assert(typeInfo);

    if ( !sd_listIsEmpty(typeInfo->stack) ) {
        assert(typeInfo->current);
        node = sd_listTakeFirst(typeInfo->stack);
        assert(typeInfo->current == node);
        typeInfo->current = sd_listReadFirst(typeInfo->stack);
    }

    return node;
}


static sd_typeInfo
sd_typeInfoNew (
    void)
{
    sd_typeInfo info = os_malloc(C_SIZEOF(sd_typeInfo));
    info->root      = sd_moduleNew(NULL);
    info->current   = NULL;
    info->stack     = sd_listNew();
    info->emptyList = sd_listNew();
    info->errorInfo = NULL;
    if ( info->stack && info->emptyList && info->root ) {
        sd_typeInfoPush(info, (sd_node)info->root);
    } else {
        sd_typeInfoFree(info);
        info = NULL;
    }
    return info;
}


static c_bool
sd_typeInfoAddChild (
    sd_typeInfo typeInfo,
    sd_node     node)
{
    c_bool       result  = FALSE;

    assert(typeInfo);
    assert(node);

    if ( SD_CURRENT(typeInfo) ) {
        switch ( SD_CURRENT_KIND(typeInfo)) {
            case SD_TYPEINFO_KIND_MODULE:
                result = sd_moduleAddChild((sd_module)typeInfo->current, node);
                break;
            case SD_TYPEINFO_KIND_STRUCT:
                result = sd_structAddChild((sd_struct)typeInfo->current, node);
                break;
            case SD_TYPEINFO_KIND_MEMBER:
                result = sd_memberAddChild((sd_member)typeInfo->current, node);
                break;
            case SD_TYPEINFO_KIND_UNION:
                result = sd_unionAddChild((sd_union)typeInfo->current, node);
                break;
            case SD_TYPEINFO_KIND_UNIONCASE:
                result = sd_unionCaseAddChild((sd_unionCase)typeInfo->current, node);
                break;
            case SD_TYPEINFO_KIND_UNIONSWITCH:
                result = sd_unionSwitchAddChild((sd_unionSwitch)typeInfo->current, node);
                break;
            case SD_TYPEINFO_KIND_TYPEDEF:
                result = sd_typeDefAddChild((sd_typeDef)typeInfo->current, node);
                break;
            case SD_TYPEINFO_KIND_ENUM:
                result = sd_enumAddChild((sd_enum)typeInfo->current, node);
                break;
            case SD_TYPEINFO_KIND_ARRAY:
            case SD_TYPEINFO_KIND_SEQUENCE:
                result = sd_templateAddChild((sd_template)typeInfo->current, node);
                break;
            default:
                assert(0);
                break;
        }
    } else if ( node->kind == SD_TYPEINFO_KIND_MODULE ) {
        assert(0);
        result = FALSE;
    } else {
        assert(0);
    }

    return result;
}

static c_bool
attributeCompare (
    void *obj,
    void *arg)
{
    c_bool                 equal     = FALSE;
    sd_xmlParserAttribute  attribute = obj;
    const c_char          *name      = arg;

    if ( attribute && name && (strcmp(attribute->name, name) == 0) ) {
        equal = TRUE;
    }
    return equal;
}

static sd_xmlParserAttribute
findAttribute (
    sd_list list,
    const c_char *name)
{
    sd_xmlParserAttribute attribute;

    attribute = sd_listFind(list, attributeCompare, (void *)name);

    return attribute;
}

static c_char *
getElementName (
    sd_list list)
{
    sd_xmlParserAttribute  attribute;
    c_char                *name     = NULL;

    if ( list ) {
        attribute = sd_listFind(list, attributeCompare, "name");
        if ( attribute ) {
            name = attribute->value;
        }
    }

    return name;
}

static c_bool
getAttributeNumber (
    sd_list       list,
    const c_char *name,
    c_long       *value)
{
    c_bool                 result     = FALSE;
    sd_xmlParserAttribute  attribute;

    if ( list ) {
        attribute = sd_listFind(list, attributeCompare, (void *)name);
        if ( attribute ) {
            result = sd_stringToLong(attribute->value, value);
        }
    }

    return result;
}

static c_bool
handleMetaData (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    OS_UNUSED_ARG(info);
    OS_UNUSED_ARG(attributes);
    OS_UNUSED_ARG(start);
    OS_UNUSED_ARG(handle);
    return TRUE;
}

static c_bool
handleModule (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_module  pmodule;
    c_char    *name;
    c_bool     result = FALSE;

    TRACE(printf("handleModule called\n"));

    if ( start ) {
        name = getElementName(attributes);

        if ( name ) {
            pmodule = sd_moduleNew(name);
            if ( sd_typeInfoAddChild(info, (sd_node)pmodule) ) {
                sd_typeInfoPush(info, (sd_node)pmodule);
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)pmodule);
            }
        } else {
            SET_XMLPARSER_ERROR(handle, MODULE_SPEC_INVALID);
        }
    } else {
        sd_typeInfoPop(info);
        result = TRUE;
    }

    TRACE(printf("handleModule exit\n"));

    return result;
}

static c_bool
handleStruct (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_struct  pstruct;
    c_char    *name;
    c_bool     result = FALSE;

    TRACE(printf("handleStruct called\n"));

    if ( start ) {
        name = getElementName(attributes);

        if ( name ) {
            pstruct = sd_structNew(name);
            if ( sd_typeInfoAddChild(info, (sd_node)pstruct) ) {
                sd_typeInfoPush(info, (sd_node)pstruct);
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)pstruct);
            }
        } else {
            SET_XMLPARSER_ERROR(handle, STRUCT_SPEC_INVALID);
        }
    } else {
        sd_typeInfoPop(info);
        result = TRUE;
    }

    TRACE(printf("handleStruct exit\n"));

    return result;
}


static c_bool
handleMember (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_member  member;
    c_char    *name;
    c_bool     result = FALSE;

    TRACE(printf("handleMember called\n"));

    if ( start ) {
        name = getElementName(attributes);

        if ( name ) {
            member = sd_memberNew(name);
            if ( sd_typeInfoAddChild(info, (sd_node)member) ) {
                sd_typeInfoPush(info, (sd_node)member);
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)member);
            }
        } else {
            SET_XMLPARSER_ERROR(handle, MEMBER_SPEC_INVALID);
        }
    } else {
        sd_typeInfoPop(info);
        result = TRUE;
    }

    TRACE(printf("handleMember exit\n"));

    return result;
}

static c_bool
handleUnion (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_union  punion;
    c_char   *name;
    c_bool    result = FALSE;

    TRACE(printf("handleUnion called\n"));

    if ( start ) {
        name = getElementName(attributes);

        if ( name ) {
            punion = sd_unionNew(name);
            if ( sd_typeInfoAddChild(info, (sd_node)punion) ) {
                sd_typeInfoPush(info, (sd_node)punion);
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)punion);
            }
        } else {
            SET_XMLPARSER_ERROR(handle, UNION_SPEC_INVALID);
        }
    } else {
        sd_typeInfoPop(info);
        result = TRUE;
    }

    TRACE(printf("handleUnion exit\n"));

    return result;
}

static c_bool
handleUnionCase (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_unionCase  pcase;
    c_char       *name;
    c_bool        result = FALSE;

    TRACE(printf("handleUnionCase called\n"));

    if ( start ) {
        name = getElementName(attributes);

        if ( name ) {
            pcase = sd_unionCaseNew(name);
            if ( sd_typeInfoAddChild(info, (sd_node)pcase) ) {
                sd_typeInfoPush(info, (sd_node)pcase);
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)pcase);
            }
        } else {
            SET_XMLPARSER_ERROR(handle, UNIONCASE_SPEC_INVALID);
        }
    } else {
        sd_typeInfoPop(info);
        result = TRUE;
    }

    TRACE(printf("handleUnionCase exit\n"));

    return result;
}

static c_bool
handleUnionLabel (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_unionLabel         plabel;
    c_bool                result    = FALSE;
    sd_xmlParserAttribute attribute = NULL;

    TRACE(printf("handleUnionLabel called\n"));

    if ( start ) {
        if ( sd_listSize(attributes) == 1 ) {
            attribute = findAttribute(attributes, "value");
            if ( !attribute ) {
                SET_XMLPARSER_ERROR(handle, LABEL_SPEC_INVALID);
            }
        }

        if ( attribute ) {
            plabel = sd_unionLabelNew(attribute->value);
            if ( sd_typeInfoAddChild(info, (sd_node)plabel) ) {
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)plabel);
            }
        }
    } else {
        result = TRUE;
    }

    TRACE(printf("handleUnionLabel exit\n"));

    return result;
}

static c_bool
handleDefaultLabel (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_unionLabel plabel;
    sd_unionCase  punionCase;
    c_bool        result    = FALSE;
    c_bool        noError   = TRUE;

    OS_UNUSED_ARG(attributes);

    if ( start ) {
        if ( SD_CURRENT(info) ) {
            switch ( SD_CURRENT_KIND(info) ) {
                case SD_TYPEINFO_KIND_UNIONCASE:
                    punionCase = (sd_unionCase)info->current;
                    break;
                default:
                    noError = FALSE;
                    SET_XMLPARSER_ERROR(handle, UNION_SPEC_INVALID);
                    punionCase = NULL;
                    break;
            }
        } else {
            noError = FALSE;
            SET_XMLPARSER_ERROR(handle, UNION_SPEC_INVALID);
            punionCase = NULL;
        }

        if ( noError ) {
            if ( !sd_listIsEmpty(punionCase->labels) ) {
                noError = FALSE;
                SET_XMLPARSER_ERROR(handle, UNION_SPEC_INVALID);
            }
        }

        if ( noError ) {
            plabel = sd_unionLabelNew(0);
            plabel->isDefault = TRUE;
            if ( sd_typeInfoAddChild(info, (sd_node)plabel) ) {
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)plabel);
            }
        }
    } else {
        result = TRUE;
    }

    return result;
}

static c_bool
handleUnionSwitch (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_unionSwitch  punionSwitch;
    c_bool          result = FALSE;

    OS_UNUSED_ARG(attributes);
    OS_UNUSED_ARG(handle);
    TRACE(printf("handleUnionSwitch called\n"));

    if ( start ) {
        punionSwitch = sd_unionSwitchNew();
        if ( sd_typeInfoAddChild(info, (sd_node)punionSwitch) ) {
            sd_typeInfoPush(info, (sd_node)punionSwitch);
            result = TRUE;
        } else {
            sd_nodeFree((sd_node)punionSwitch);
        }
    } else {
        sd_typeInfoPop(info);
        result = TRUE;
    }

    TRACE(printf("handleUnionSwitch exit\n"));

    return result;
}


static c_bool
handleTypeDef (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_typeDef ptypeDef;
    c_char    *name;
    c_bool     result = FALSE;

    if ( start ) {
        name = getElementName(attributes);

        if ( name ) {
            ptypeDef = sd_typeDefNew(name);
            if ( sd_typeInfoAddChild(info, (sd_node)ptypeDef) ) {
                sd_typeInfoPush(info, (sd_node)ptypeDef);
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)ptypeDef);
            }
        } else {
            SET_XMLPARSER_ERROR(handle, TYPEDEF_SPEC_INVALID);
        }
    } else {
        sd_typeInfoPop(info);
        result = TRUE;
    }

    return result;
}

static c_bool
handleEnum (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_enum  penum;
    c_char  *name;
    c_bool   result = FALSE;

    if ( start ) {
        name = getElementName(attributes);

        if ( name ) {
            penum = sd_enumNew(name);
            if ( sd_typeInfoAddChild(info, (sd_node)penum) ) {
                sd_typeInfoPush(info, (sd_node)penum);
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)penum);
            }
        } else {
            SET_XMLPARSER_ERROR(handle, ENUM_SPEC_INVALID);
        }
    } else {
        sd_typeInfoPop(info);
        result = TRUE;
    }

    return result;
}

static c_bool
handleEnumLabel (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_enumLabel  plabel;
    c_char       *name;
    c_long        value        = 0;
    c_bool        valueDefined = FALSE;
    c_bool        noError      = TRUE;
    c_bool        result       = FALSE;

    if ( start ) {
        name = getElementName(attributes);

        if ( name ) {
            sd_xmlParserAttribute attribute;
            attribute = findAttribute(attributes, "value");
            if ( attribute ) {
                if ( sd_stringToLong(attribute->value, &value) ) {
                    valueDefined = TRUE;
                } else {
                    SET_XMLPARSER_ERROR(handle, ELEMENT_SPEC_INVALID);
                    noError = FALSE;
                }
            }
        }

        if ( noError ) {
            plabel = sd_enumLabelNew(name, valueDefined, value);
            if ( sd_typeInfoAddChild(info, (sd_node)plabel) ) {
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)plabel);
            }
        }
    } else {
        result = TRUE;
    }

    return result;
}


static c_bool
handleTypeRef (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_typeRef  ptypeRef;
    c_char     *name;
    c_bool      result = FALSE;

    if ( start ) {
        name = getElementName(attributes);

        if ( name ) {
            ptypeRef = sd_typeRefNew(name);
            if ( sd_typeInfoAddChild(info, (sd_node)ptypeRef) ) {
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)ptypeRef);
            }
        } else {
            SET_XMLPARSER_ERROR(handle, TYPE_SPEC_INVALID);
        }
    } else {
        result = TRUE;
    }

    return result;
}

static c_bool
handleTemplate (
    sd_typeInfo     info,
    sd_typeInfoKind kind,
    sd_list         attributes,
    c_bool          start,
    sd_xmlParser    handle)
{
    sd_template  ptemplate;
    c_long       size    = 0;
    c_bool       result  = FALSE;
    c_bool       noError = TRUE;

    TRACE(printf("handleTemplate called\n"));

    if ( start ) {
        if ( attributes ) {
            noError = getAttributeNumber(attributes, "size", &size);
        }

        if ( noError ) {
            ptemplate = sd_templateNew(kind, size);
            if ( sd_typeInfoAddChild(info, (sd_node)ptemplate) ) {
                sd_typeInfoPush(info, (sd_node)ptemplate);
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)ptemplate);
            }
        } else {
            if ( kind == SD_TYPEINFO_KIND_ARRAY ) {
                SET_XMLPARSER_ERROR(handle, ARRAY_SPEC_INVALID);
            } else {
                SET_XMLPARSER_ERROR(handle, SEQUENCE_SPEC_INVALID);
            }
        }
    } else {
        sd_typeInfoPop(info);
        result = TRUE;
    }

    TRACE(printf("handleTemplate exit\n"));

    return result;
}

static c_bool
handleArray (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handleTemplate(info, SD_TYPEINFO_KIND_ARRAY, attributes, start, handle);
}

static c_bool
handleSequence (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handleTemplate(info, SD_TYPEINFO_KIND_SEQUENCE, attributes, start, handle);
}

static c_bool
handleString (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    sd_string    pstring;
    c_long       length  = 0;
    c_bool       result  = FALSE;
    c_bool       noError = TRUE;

    if ( start ) {
        if ( attributes ) {
            noError = getAttributeNumber(attributes, "length", &length);
        }

        if ( noError ) {
            pstring = sd_stringNew(length);
            if ( sd_typeInfoAddChild(info, (sd_node)pstring) ) {
                result = TRUE;
            } else {
                sd_nodeFree((sd_node)pstring);
            }
        } else {
            SET_XMLPARSER_ERROR(handle, STRING_SPEC_INVALID);
        }
    } else {
        result = TRUE;
    }

    return result;
}

static c_bool
handlePrimitive (
    sd_typeInfo     info,
    sd_typeInfoKind kind,
    sd_list         attributes,
    c_bool          start,
    sd_xmlParser    handle)
{
    c_bool   result = FALSE;
    sd_node  pnode;

    OS_UNUSED_ARG(attributes);
    TRACE(printf("handlePrimitive called\n"));

    if ( start ) {
        pnode = sd_nodeNew(kind, sd_nodeDefaultInitAction, sd_nodeDefaultAction);
        if ( pnode ) {
            if ( sd_typeInfoAddChild(info, pnode) ) {
                result = TRUE;
            } else {
                SET_XMLPARSER_ERROR(handle, MODULE_SPEC_INVALID);
                sd_nodeFree(pnode);
            }
        }
    } else {
        result = TRUE;
    }

    TRACE(printf("handlePrimitive exit\n"));

    return result;
}

static c_bool
handleChar (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_CHAR, attributes, start, handle);
}

static c_bool
handleBoolean (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_BOOLEAN, attributes, start, handle);
}
static c_bool
handleOctet (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_OCTET, attributes, start, handle);
}

static c_bool
handleShort (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_SHORT, attributes, start, handle);
}

static c_bool
handleUShort (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_USHORT, attributes, start, handle);
}

static c_bool
handleLong (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_LONG, attributes, start, handle);
}

static c_bool
handleULong (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_ULONG, attributes, start, handle);
}

static c_bool
handleLongLong (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_LONGLONG, attributes, start, handle);
}

static c_bool
handleULongLong (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_ULONGLONG, attributes, start, handle);
}

static c_bool
handleFloat (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_FLOAT, attributes, start, handle);
}

static c_bool
handleDouble (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_DOUBLE, attributes, start, handle);
}

static c_bool
handleTime (
    sd_typeInfo  info,
    sd_list      attributes,
    c_bool       start,
    sd_xmlParser handle)
{
    return handlePrimitive(info, SD_TYPEINFO_KIND_TIME, attributes, start, handle);
}

typedef struct sd_elementTypeEntry_s {
    const c_char      *name;
    sd_typeInfoHandler typeInfoHandler;
} sd_elementTypeEntry;

static const sd_elementTypeEntry elementTypeMap[] = {
    { "MetaData",   handleMetaData     },
    { "Module",     handleModule       },
    { "Struct",     handleStruct       },
    { "Member",     handleMember       },
    { "Union",      handleUnion        },
    { "Case",       handleUnionCase    },
    { "SwitchType", handleUnionSwitch  },
    { "Label",      handleUnionLabel   },
    { "TypeDef",    handleTypeDef      },
    { "Enum",       handleEnum         },
    { "Element",    handleEnumLabel    },
    { "Type",       handleTypeRef      },
    { "Array",      handleArray        },
    { "Sequence",   handleSequence     },
    { "String",     handleString       },
    { "Char",       handleChar         },
    { "Boolean",    handleBoolean      },
    { "Octet",      handleOctet        },
    { "Short",      handleShort        },
    { "UShort",     handleUShort       },
    { "Long",       handleLong         },
    { "ULong",      handleULong        },
    { "LongLong",   handleLongLong     },
    { "ULongLong",  handleULongLong    },
    { "Float",      handleFloat        },
    { "Double",     handleDouble       },
    { "Time",       handleTime         },
    { "Default",    handleDefaultLabel }
};
static const c_long elementTypeMapSize = sizeof(elementTypeMap)/sizeof(sd_elementTypeEntry);

static sd_typeInfoHandler
findTypeInfoHandler (
    const c_char *name)
{
    sd_typeInfoHandler handler = NULL;
    c_long             i;

    assert(name);

    for ( i = 0; !handler && (i < elementTypeMapSize); i++ ) {
        if ( name && (strcmp(name, elementTypeMap[i].name) == 0) ) {
            handler = elementTypeMap[i].typeInfoHandler;
        }
    }

    return handler;
}

static c_bool
handleXmlElement (
    sd_xmlParserKind     kind,
    sd_xmlParserElement  element,
    void                *argument,
    sd_xmlParser         handle)
{
    c_bool             result   = FALSE;
    sd_typeInfo        info     = argument;
    sd_typeInfoHandler handler;

    switch ( kind ) {
        case SD_XML_PARSER_KIND_ELEMENT_START:
            handler = findTypeInfoHandler(element->name);
            if ( handler ) {
                result = handler(info, element->attributes, TRUE, handle);
            } else {
                SET_XMLPARSER_ERROR(handle, METADATA_SPEC_INVALID);
            }
            break;
        case SD_XML_PARSER_KIND_ELEMENT_END:
            handler = findTypeInfoHandler(element->name);
            if ( handler ) {
                result = handler(info, element->attributes, FALSE, handle);
            } else {
                SET_XMLPARSER_ERROR(handle, METADATA_SPEC_INVALID);
            }
            break;
        default:
            SET_XMLPARSER_ERROR(handle, METADATA_SPEC_INVALID);
            break;
    }

    return result;
}

static c_bool
walkModules (
    void *obj,
    void *arg)
{
    sd_node           node      = obj;
    sd_nodeActionArg *actionArg = arg;
    actionArg->result = node->initAction(node, actionArg);
    return actionArg->result;
}

c_bool
sd_typeInfoParserParse (
    const c_char        *description,
    sd_typeInfoCallback  callback,
    void                *argument,
    sd_errorReport      *errorInfo)
{
    c_bool           result = FALSE;
    sd_typeInfo      info;
    sd_nodeActionArg actionArg;

    if ( callback ) {
        info = sd_typeInfoNew();
        result = sd_xmlParserParse(description, handleXmlElement, info, errorInfo);
        if ( result ) {
            actionArg.typeInfo  = info;
            actionArg.node      = NULL;
            actionArg.callback  = callback;
            actionArg.arguments = argument;
            actionArg.result    = FALSE;

            assert(info->root);

            sd_listWalk(info->root->children, walkModules, &actionArg);
            result = actionArg.result;
        }
        sd_typeInfoFree(info);
    }

    return result;
}

c_bool
sd_typeInfoParserNext (
    sd_typeInfoHandle    handle,
    sd_typeInfoCallback  callback,
    void                *argument)
{
    c_bool            result    = FALSE;
    sd_nodeActionArg *prevInfo  = (sd_nodeActionArg *)handle;
    sd_nodeActionArg  nextInfo;

    assert(prevInfo);

    nextInfo = *prevInfo;
    if ( callback ) {
        nextInfo.callback  = callback;
        nextInfo.arguments = argument;
    }

    if ( nextInfo.node ) {
        result = nextInfo.node->action(nextInfo.node, &nextInfo);
    }

    return result;
}

static c_bool
findTypeInfoAttribute (
    void *object,
    void *argument)
{
    sd_typeInfoAttribute *attribute = object;
    const c_char         *name      = argument;
    c_bool                found     = FALSE;

    assert(attribute);
    assert(attribute->name);
    assert(name);

    if ( strcmp(attribute->name, name) == 0 ) {
        found = TRUE;
    }

    return found;
}


c_long
sd_findAttributeNumber (
    sd_list       attributes,
    const c_char *name)
{
    sd_typeInfoAttribute *attribute;
    c_long                result = -1;

    assert(attributes);
    assert(name);

    attribute = sd_listFind(attributes, findTypeInfoAttribute, (void *)name);
    if ( attribute ) {
        if ( attribute->d == SD_TYPEINFO_ATTR_KIND_NUMBER ) {
            result = attribute->u.nvalue;
        }
    }

    return result;
}

const c_char *
sd_findAttributeValue (
    sd_list       attributes,
    const c_char *name)
{
    sd_typeInfoAttribute *attribute;
    const c_char         *value     = NULL;

    assert(attributes);
    assert(name);

    attribute = sd_listFind(attributes, findTypeInfoAttribute, (void *) name);
    if ( attribute ) {
        if ( attribute->d == SD_TYPEINFO_ATTR_KIND_STRING ) {
            value = attribute->u.svalue;
        }
    }

    return value;

}

