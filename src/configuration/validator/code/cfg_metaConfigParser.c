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

#include "vortex_os.h"
#include "os_report.h"
#include "ut_xmlparser.h"
#include "cf_config.h"
#include "cfg_metaConfigParser.h"

typedef enum {
    PARSER_STATE_UNDEFINED,
    PARSER_STATE_ELEMENT,
    PARSER_STATE_ATTRIBUTE,
    PARSER_STATE_MINIMUM,
    PARSER_STATE_MAXIMUM,
    PARSER_STATE_MAX_LENGTH,
    PARSER_STATE_LABEL
} ParserState_t;

/* This structure is used when parsing the meta configuration file.
 * The meta configuration parser builds a tree of elements and
 * attributes corresponding to the information provided by the
 * meta configuration file.
 * The root element corresponds to the OpenSplice entry tag and
 * is the root of the tree that contains the specific configuration
 * elements. The tree that is build under the root element will have
 * a similar structure as a configuration file. For example
 * element name=OpenSplice
 *   element name=Domain
 *     element name=Name
 *     ..
 *
 * The serviceMapping is used to collect the service mappings which
 * relate a particular service with the command used to start the service.
 * Note that the command relates to a particular service.
 * These service mappings are used to relate the Service entries which are
 * listed under the Domain element with the configuration of the services.
 *
 * The collectServices indicates that the current state of the parser is
 * to collect the service mappings.
 *
 * The current member points to the current meta configuration element that
 * is being processed.
 *
 * The state member indicates the kind of the current meta configuration element.
 * For example for an element named leafEnum it will be set to PARSER_STATE_ELEMENT,
 * for an element named attributeBoolean it will be set to PARSER_STATE_ATTRIBUTE,
 * for an element named minimum it will be set to PARSER_STATE_MINIMUM, etc
 */
typedef struct MetaConfigParser_s *MetaConfigParser;
struct MetaConfigParser_s {
    cfg_element root;
    cfg_element serviceMapping;
    os_boolean collectServices;
    cfg_node current;
    ParserState_t state;
};

static void
determineSyntaxParserState(
    MetaConfigParser parser)
{
    if (parser->current) {
        if (cfg_nodeIsElement(parser->current)) {
            parser->state = PARSER_STATE_ELEMENT;
        } else {
            parser->state = PARSER_STATE_ATTRIBUTE;
        }
    } else {
        parser->state = PARSER_STATE_UNDEFINED;
    }
}

/* The function that is called on each open tag in the meta configuration file.
 * Only the following element tags will be handled:
 *
 * Tags that correspond to elements in the configuration file:
 * - rootElement
 * - element
 * - leafEmpty
 * - leafBoolean
 * - leafInt
 * - leafLong
 * - leafSize
 * - leafFloat
 * - leafDouble
 * - leafEnum
 * - leafString
 *
 * Tags that correspond to attributes in the configuration file;
 * - attributeBoolean
 * - attributeInt
 * - attributeLong
 * - attributeSize
 * - attributeFloat
 * - attributeDouble
 * - attributeEnum
 * - attributeString
 *
 * Tags that set the constraints on configuration values;
 * - minimum   : minimum allowed value of elements and attributes
 * - maximum   : maximum allowed value of elements and attributes
 * - value     : an allowed label of an enum element or attribute
 * - maxLength : the maximum string length of an string element or attribute
 *
 * Tags that correspond to the Service mapping
 * - serviceMapping
 */

static int
elementOpenCallback(
    void *varg,
    os_address parentinfo,
    os_address *eleminfo,
    const char *name)
{
    MetaConfigParser parser = varg;
    cfg_node node = NULL;
    int result = 0;

    OS_UNUSED_ARG(parentinfo);

    if (strcmp(name, "rootElement") == 0) {
        node = cfg_node(cfg_elementNew());
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "element") == 0) {
        if (parser->collectServices) {
            node = cfg_node(cfg_serviceMappingNew());
        } else {
            node = cfg_node(cfg_elementNew());
        }
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "leafEmpty") == 0) {
        node = cfg_node(cfg_emptyElementNew());
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "leafBoolean") == 0) {
        node = cfg_node(cfg_booleanElementNew());
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "leafInt") == 0) {
        node = cfg_node(cfg_intElementNew());
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "leafLong") == 0) {
        node = cfg_node(cfg_longElementNew());
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "leafEnum") == 0) {
        node = cfg_node(cfg_enumElementNew());
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "leafFloat") == 0) {
        node = cfg_node(cfg_floatElementNew());
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "leafDouble") == 0) {
        node = cfg_node(cfg_doubleElementNew());
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "leafSize") == 0) {
        node = cfg_node(cfg_sizeElementNew());
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "leafString") == 0) {
        node = cfg_node(cfg_stringElementNew());
        parser->state = PARSER_STATE_ELEMENT;
    } else if (strcmp(name, "attributeBoolean") == 0) {
        node = cfg_node(cfg_booleanAttributeNew());
        parser->state = PARSER_STATE_ATTRIBUTE;
    } else if (strcmp(name, "attributeInt") == 0) {
        node = cfg_node(cfg_intAttributeNew());
        parser->state = PARSER_STATE_ATTRIBUTE;
    } else if (strcmp(name, "attributeLong") == 0) {
        node = cfg_node(cfg_longAttributeNew());
        parser->state = PARSER_STATE_ATTRIBUTE;
    } else if (strcmp(name, "attributeEnum") == 0) {
        node = cfg_node(cfg_enumAttributeNew());
        parser->state = PARSER_STATE_ATTRIBUTE;
    } else if (strcmp(name, "attributeFloat") == 0) {
        node = cfg_node(cfg_floatAttributeNew());
        parser->state = PARSER_STATE_ATTRIBUTE;
    } else if (strcmp(name, "attributeDouble") == 0) {
        node = cfg_node(cfg_doubleAttributeNew());
        parser->state = PARSER_STATE_ATTRIBUTE;
    } else if (strcmp(name, "attributeSize") == 0) {
        node = cfg_node(cfg_sizeAttributeNew());
        parser->state = PARSER_STATE_ATTRIBUTE;
    } else if (strcmp(name, "attributeString") == 0) {
        node = cfg_node(cfg_stringAttributeNew());
        parser->state = PARSER_STATE_ATTRIBUTE;
    } else if (strcmp(name, "minimum") == 0) {
        parser->state = PARSER_STATE_MINIMUM;
    } else if (strcmp(name, "maximum") == 0) {
        parser->state = PARSER_STATE_MAXIMUM;
    } else if (strcmp(name, "maxLength") == 0) {
        parser->state = PARSER_STATE_MAX_LENGTH;
    } else if (strcmp(name, "value") == 0) {
        parser->state = PARSER_STATE_LABEL;
    } else if (strcmp(name, "serviceMapping") == 0) {
        parser->serviceMapping = cfg_elementNew();
        node = cfg_node(parser->serviceMapping);
        cfg_nodeSetName(node, name);
        parser->state = PARSER_STATE_ELEMENT;
        parser->collectServices = OS_TRUE;
    }

    if (node) {
        if (parser->current) {
            if (cfg_nodeIsElement(parser->current)) {
                if (parser->state == PARSER_STATE_ELEMENT) {
                    cfg_elementAddChild(cfg_element(parser->current), cfg_element(node));
                } else if (parser->state == PARSER_STATE_ATTRIBUTE) {
                    cfg_elementAddAttribute(cfg_element(parser->current), cfg_attribute(node));
                } else {
                    result = -1;
                }
            } else {
                result = -1;
            }
        } else if (parser->collectServices) {
            /* continue */
        } else if (parser->root) {
            cfg_elementAddChild(parser->root, cfg_element(node));
        } else {
            parser->root = cfg_element(node);
        }

        parser->current = node;
    }

    *eleminfo = (os_address)node;

    return result;
}

/* The function that is called on each attribute in the meta configuration file.
 * Only the following attributes will be handled:
 * - name : the name of an element or attribute
 * - minOccurrences : the minimum number a configuration element should occur
 * - maxOccurrences : the maximum number a configuration element should occur
 * - required : indicates if attribute should be present
 * - command : the command related to a service mapping
 *
 */
static int
elementAttributeCallback(
    void *varg,
    os_address eleminfo,
    const char *name,
    const char *value)
{
    MetaConfigParser parser = varg;
    int result = 0;

    if (eleminfo) {
        if (strcmp(name, "name") == 0) {
            cfg_nodeSetName(parser->current, value);
        } else if (strcmp(name, "minOccurrences") == 0) {
            result = cfg_nodeSetMinOccurrences(parser->current, value);
        } else if (strcmp(name, "maxOccurrences") == 0) {
            result = cfg_nodeSetMaxOccurrences(parser->current, value);
        } else if (strcmp(name, "required") == 0) {
              result = cfg_nodeSetRequired(parser->current, value);
        } else if (strcmp(name, "command") == 0) {
            if (parser->collectServices) {
                if (cfg_nodeIsServiceMapping(parser->current)) {
                    cfg_serviceMappingSetCommand(cfg_serviceMapping(parser->current), value);
                } else {
                    result = -1;
                }
            } else {
                result = -1;
            }
        }
    }

    return result;
}

/* The function that is called on each data item in the meta configuration file.
 * Depending on the parser state the value is interpreted as:
 * - minimum
 * - maximum
 * - maxLength
 * - label
 */
static int
elementDataCallback(
    void *varg,
    os_address eleminfo,
    const char *data)
{
    MetaConfigParser parser = varg;
    int result = 0;

    OS_UNUSED_ARG(eleminfo);

    if (parser->current) {
        switch (parser->state) {
        case PARSER_STATE_MINIMUM:
            result = cfg_nodeSetMinimum(parser->current, data);
            break;
        case PARSER_STATE_MAXIMUM:
            result = cfg_nodeSetMaximum(parser->current, data);
            break;
        case PARSER_STATE_MAX_LENGTH:
            result = cfg_nodeSetMaxLength(parser->current, data);
            break;
        case PARSER_STATE_LABEL:
            result = cfg_nodeAddLabel(parser->current, data);
            break;
        default:
            break;
        }
    }

    return result;
}

static int
elementCloseCallback(
    void *varg,
    os_address eleminfo)
{
    MetaConfigParser parser = varg;

    if (eleminfo) {
        if (parser->collectServices && !cfg_nodeIsServiceMapping(parser->current)) {
            parser->collectServices = OS_FALSE;
        }
        parser->current = cfg_nodeGetParent(parser->current);
    }

    determineSyntaxParserState(parser);

    return 0;
}

static void
errorCallback(
    void *varg,
    const char *msg,
    int line)
{
    OS_UNUSED_ARG(varg);

    OS_REPORT(OS_ERROR, "configuration validator", 0,
              "Failed to parse configuration file: error %d - %s", line, msg);
}

cfgprs_status
cfg_parseMetaConfig(
    FILE *fp,
    cfg_element *root,
    cfg_element *serviceMapping)
{
    C_STRUCT(MetaConfigParser) parser = {0};
    struct ut_xmlpCallbacks cb;
    struct ut_xmlpState *st;
    cfgprs_status status;

    cb.elem_open = elementOpenCallback;
    cb.elem_data = elementDataCallback;
    cb.elem_close = elementCloseCallback;
    cb.attr = elementAttributeCallback;
    cb.error = errorCallback;

    st = ut_xmlpNewFile(fp, &parser, &cb);

    if (ut_xmlpParse(st) == 0) {
#ifdef CFG_DEBUG_CONFIG
        printf("\n\n");
        cfg_nodePrint(parser.root, 0);

        printf("\n\n");
        cfg_nodePrint(cfg_node(parser.serviceMapping), 0);
#endif
        *root = parser.root;
        *serviceMapping = parser.serviceMapping;
        status = CFGPRS_OK;
    } else {
        status = CFGPRS_ERROR;
    }

    ut_xmlpFree(st);

    return status;
}
