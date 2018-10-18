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
#include "v__nameSpace.h"
#include "v_cfAttribute.h"
#include "v_cfData.h"
#include "v_cfElement.h"
#include "v_cfNode.h"
#include "os_heap.h"

#define V_MAX_STRLEN_NAMESPACE (101) /* same as D_MAX_STRLEN_NAMESPACE */

/* NOTE: function is base on d_nameSpaceAddElement */
static struct v_nameSpacePartitionTopic *
formatPartitionTopic (
    const char * partitionTopic,
    const char * topicGiven )
{
    struct v_nameSpacePartitionTopic *result = NULL;
    char *  partition;
    char *  topic;
    os_uint32 strlenPartitionTopic;
    os_uint32 strlenTopicGiven;

    /* QAC EXPECT 1253; */
    strlenPartitionTopic = (os_uint32) (strlen(partitionTopic) + 1);
    /* if strlen exceeds max strlen NULL is returned and assumed that
     * the durability service reports an error */
    if (strlenPartitionTopic < V_MAX_STRLEN_NAMESPACE) {
        if (topicGiven) {
            /* QAC EXPECT 1253; */
            strlenTopicGiven = (os_uint32) (strlen(topicGiven) + 1);
            /* QAC EXPECT 1253; */
            /* if strlen exceeds max strlen NULL is returned and assumed that
             * the durability service reports an error */
            if (strlenTopicGiven < V_MAX_STRLEN_NAMESPACE) {
                result = os_malloc(sizeof(struct v_nameSpacePartitionTopic));
                result->partition = os_strdup(partitionTopic);
                result->topic = os_strdup(topicGiven);
            }
        } else {
            partition = os_malloc(strlenPartitionTopic);

            os_strncpy(partition, partitionTopic, strlenPartitionTopic);
            /* Make topic point to last character in partition string.
            * partition points to first character and strlenPartitionTopic
            * includes '\0', so subtract 2 to point to last character.
            */
            topic = partition + (strlenPartitionTopic-2);

            /* QAC EXPECT 2106,3123; */
            while ((*topic != '.') && (topic != partition)) {
                /* QAC EXPECT 0489; */
                topic--;
            }
            /* QAC EXPECT 2106,3123; */
            if (*topic == '.') {
                *topic = 0;
                /* QAC EXPECT 0489; */
                topic++;
                /* QAC EXPECT 2106; */
                if (*topic != 0) {
                    result = os_malloc(sizeof(struct v_nameSpacePartitionTopic));
                    result->partition = os_strdup(partition);
                    result->topic = os_strdup(topic);
                }
            } else {
                /* Though <PartitionTopic> was used in the definition of a namespace, only
                * a partition is provided. */
                result = os_malloc(sizeof(struct v_nameSpacePartitionTopic));
                result->partition = os_strdup(partition);
                result->topic = os_strdup("*");
            }
            os_free(partition);
        }
    }

    return result;
}

static c_iter
configurationCollectEnabledServices (
    v_configuration config)
{
    c_iter result = NULL;

    c_iter services;
    v_cfNode tmp;

    v_cfAttribute attribute;
    c_value value;

    services = v_cfElementXPath(config->root, "Domain/Service");

    while ((tmp = v_cfNode(c_iterTakeFirst(services))) != NULL) {
        if (v_cfNodeKind(tmp) == V_CFELEMENT) {
            c_bool enabled = TRUE;

            attribute = v_cfElementAttribute(v_cfElement(tmp), "enabled");
            if (attribute != NULL) {
                enabled = FALSE;
                value = v_cfAttributeValue(attribute);
                if (value.kind == V_STRING) {
                    if (strcmp(value.is.String, "true") == 0) {
                        enabled = TRUE;
                    }
                }
            }

            if (enabled) {
                attribute = v_cfElementAttribute(v_cfElement(tmp), "name");
                if (attribute != NULL) {
                    value = v_cfAttributeValue(attribute);
                    if (value.kind == V_STRING) {
                        result = c_iterAppend(result, os_strdup(value.is.String));
                    }
                }
            }
        }
    }
    c_iterFree(services);
    return result;
}

static c_iter /* v_cfElement */
configurationCollectDurabilityServices (
    v_configuration config)
{
    c_iter result = NULL;
    c_iter enabledServices;
    c_iter durabilityServices;
    v_cfNode tmp;
    v_cfAttribute attribute;
    c_value value;
    char *name;

    assert(config);
    assert(C_TYPECHECK(config,v_configuration));

    enabledServices = configurationCollectEnabledServices(config);
    if (c_iterLength(enabledServices) > 0) {
        durabilityServices = v_cfElementXPath(config->root, "DurabilityService");
        while ((tmp = v_cfNode(c_iterTakeFirst(durabilityServices))) != NULL) {
            if (v_cfNodeKind(tmp) == V_CFELEMENT) {
                attribute = v_cfElementAttribute(v_cfElement(tmp), "name");
                if (attribute != NULL) {
                    value = v_cfAttributeValue(attribute);
                    if (value.kind == V_STRING) {
                        c_iterIter it = c_iterIterGet(enabledServices);

                        while ((name = c_iterNext(&it)) != NULL) {
                            if (strcmp(name, value.is.String) == 0) {
                                result = c_iterAppend(result, tmp);
                                break;
                            }
                        }
                    }
                }
            }
        }
        c_iterFree(durabilityServices);
    }

    while ((name = c_iterTakeFirst(enabledServices)) != NULL) {
        os_free(name);
    }
    c_iterFree(enabledServices);

    return result;
}

static c_iter
elementResolvePartition (
    v_cfElement element)
{
    c_iter result = NULL;
    c_iter nodes, nodes2;
    v_cfNode tmp, tmp2;

    nodes = v_cfElementXPath(element, "Partition");
    while ((tmp = v_cfNode(c_iterTakeFirst(nodes))) != NULL) {
        if (v_cfNodeKind(tmp) == V_CFELEMENT) {
            nodes2 = v_cfElementGetChildren(v_cfElement(tmp));
            if (c_iterLength(nodes2) > 0) {
                while ((tmp2 = v_cfNode(c_iterTakeFirst(nodes2))) != NULL) {
                    if(v_cfNodeKind(tmp2) == V_CFDATA) {
                        c_value value;
                        value = v_cfDataValue(v_cfData(tmp2));
                        if (value.kind == V_STRING) {
                            struct v_nameSpacePartitionTopic * pt = formatPartitionTopic(value.is.String, "*");
                            result = c_iterAppend(result, pt);
                        }
                    }
                }
            } else {
                struct v_nameSpacePartitionTopic * pt = formatPartitionTopic("", "*");
                result = c_iterAppend(result, pt);
            }
            c_iterFree(nodes2);
        }
    }
    c_iterFree(nodes);
    return result;
}

static c_iter
elementResolvePartitionTopic (
    v_cfElement element)
{
    c_iter result = NULL;
    c_iter nodes, nodes2;
    v_cfNode tmp, tmp2;

    nodes = v_cfElementXPath(element, "PartitionTopic");
    while ((tmp = v_cfNode(c_iterTakeFirst(nodes))) != NULL) {
        if (v_cfNodeKind(tmp) == V_CFELEMENT) {
            nodes2 = v_cfElementGetChildren(v_cfElement(tmp));
            if (c_iterLength(nodes2) > 0) {
                while ((tmp2 = v_cfNode(c_iterTakeFirst(nodes2))) != NULL) {
                    if(v_cfNodeKind(tmp2) == V_CFDATA) {
                        c_value value;
                        value = v_cfDataValue(v_cfData(tmp2));
                        if (value.kind == V_STRING) {
                            struct v_nameSpacePartitionTopic * pt = formatPartitionTopic(value.is.String, NULL);
                            result = c_iterAppend(result, pt);
                        }
                    }
                }
            } else {
                struct v_nameSpacePartitionTopic * pt = formatPartitionTopic("*.*", NULL);
                result = c_iterAppend(result, pt);
            }
            c_iterFree(nodes2);
        }
    }
    c_iterFree(nodes);
    return result;
}

c_iter /* v_nameSpace */
v__nameSpaceCollect (
    v_kernel kernel)
{
    c_iter result = NULL;

    v_configuration config;

    c_iter elements;
    v_cfElement e;
    c_iter nodes;
    v_cfNode n;
    v_cfAttribute attribute;
    c_value value;

    assert(kernel);
    assert(C_TYPECHECK(kernel,v_kernel));

    config = v_getConfiguration(kernel);
    if (config) {
        elements = configurationCollectDurabilityServices(config);
        while ((e = v_cfElement(c_iterTakeFirst(elements))) != NULL) {
            assert(v_cfNodeKind(v_cfNode(e)) == V_CFELEMENT);

            nodes = v_cfElementXPath(e, "NameSpaces/NameSpace");
            while ((n = v_cfNode(c_iterTakeFirst(nodes))) != NULL) {
                if (v_cfNodeKind(n) == V_CFELEMENT) {
                    attribute = v_cfElementAttribute(v_cfElement(n), "name");

                    if (attribute != NULL) {
                        value = v_cfAttributeValue(attribute);
                        if (value.kind == V_STRING) {
                            c_iter list = NULL;
                            list = elementResolvePartition(v_cfElement(n));
                            list = c_iterConcat(list, elementResolvePartitionTopic(v_cfElement(n)));

                            if (c_iterLength(list) > 0) {
                                struct v_nameSpace *ns;

                                ns = os_malloc(sizeof(struct v_nameSpace));
                                ns->name = os_strdup(value.is.String);
                                ns->partitionTopics = list;
                                result = c_iterAppend(result, ns);
                            }
                        }
                    }
                }
            }
            c_iterFree(nodes);
        }
        c_iterFree(elements);
    }

    return result;
}

static c_bool
patternMatch(
    char *pattern,
    char *str)
{
    c_value p,n,r;

    p.kind = n.kind = V_STRING;
    p.is.String = (char *)pattern;
    n.is.String = (char *)str;
    r = c_valueStringMatch(p,n);
    return r.is.Boolean;
}

c_bool
v__nameSpaceIsIn (
    struct v_nameSpace *ns,
    os_char *partition,
    os_char *topic)
{
    c_bool result = FALSE;
    c_iterIter iter;
    struct v_nameSpacePartitionTopic *pt;

    iter = c_iterIterGet(ns->partitionTopics);
    while (((pt = c_iterNext(&iter)) != NULL) && (result == FALSE)) {
        if ((patternMatch(pt->partition, partition)) &&
            (patternMatch(pt->topic, topic))) {
            result = TRUE;
        }
    }

    return result;
}

void
v__nameSpaceFree (
    struct v_nameSpace *ns)
{
    struct v_nameSpacePartitionTopic *pt;

    os_free(ns->name);
    while ((pt = c_iterTakeFirst(ns->partitionTopics)) != NULL) {
        os_free(pt->partition);
        os_free(pt->topic);
        os_free(pt);
    }
    os_free(ns);
}
