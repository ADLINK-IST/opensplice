/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "in__config.h"
#include "os_heap.h"
#include "u_cfElement.h"
#include "u_cfNode.h"
#include "u_cfData.h"
#include "u_participant.h"
#include "in__configDataChannel.h"
#include "in__configChannel.h"
#include "in__configNetworkPartition.h"
#include "in__configPartitionMapping.h"

static void
in_configTraverseConfiguration(
    u_cfElement element);

static void
in_configTraverseDdsiServiceElement(
    u_cfElement element);

static void
in_configTraverseGeneralElement(
    u_cfElement element,
    in_configDdsiService ddsiService);

static void
in_configTraverseChannelsElement(
    u_cfElement element,
    in_configDdsiService ddsiService);

static void
in_configTraverseDiscoveryChannelElement(
    u_cfElement element,
    in_configDdsiService ddsiService);

static void
in_configTraverseChannelElement(
    u_cfElement element,
    in_configDdsiService ddsiService);

static void
in_configTraverseGroupQueueElement(
    u_cfElement element,
    in_configDataChannel dataChannel);

static void
in_configTraverseFragmentSizeElement(
    u_cfElement element,
    in_configDataChannel dataChannel);

static void
in_configTraversePortNrElement(
    u_cfElement element,
    in_configChannel channel);

static void
in_configTraverseInterfaceElement(
    u_cfElement element,
    in_configDdsiService ddsiService);

void
in_configTraversePartitionMappingElement(
    u_cfElement element,
    in_configPartitioning partitioning);

void
in_configTraverseNetworkPartitionElement(
    u_cfElement element,
    in_configPartitioning partitioning);

void
in_configTraverseGlobalPartitionElement(
    u_cfElement element,
    in_configPartitioning partitioning);

void
in_configTraversePartitionMappingsElement(
    u_cfElement element,
    in_configPartitioning partitioning);

void
in_configTraverseNetworkPartitionsElement(
    u_cfElement element,
    in_configPartitioning partitioning);

void
in_configTraversePartitioningElement(
    u_cfElement element,
    in_configDdsiService ddsiService);

#define INCF_ELEM_DdsiService           "DDSIService"
#define INCF_ATTRIB_DdsiService_name    "name"
#define INCF_ELEM_General               "General"
#define INCF_ELEM_Interface             "NetworkInterfaceAddress"
#define INCF_ATTRIB_Interface_value_DEF INCF_DEF_INTERFACE
#define INCF_ELEM_Channels              "Channels"
#define INCF_ELEM_Channel               "Channel"
#define INCF_ATTRIB_channel_isEnabled   "enabled"
#define INCF_ATTRIB_channel_priority    "priority"
#define INCF_ATTRIB_channel_isDefault   "default"
#define INCF_ATTRIB_channel_isEnabled_DEF TRUE
#define INCF_ATTRIB_channel_isDefault_DEF FALSE
#define INCF_ATTRIB_channel_priority_DEF 0
#define INCF_ATTRIB_channel_priority_MIN 0
#define INCF_ATTRIB_channel_priority_MAX 1000
#define INCF_ATTRIB_channel_name        "name"
#define INCF_ELEM_DiscoveryChannel      "Discovery"
#define INCF_ELEM_PortNr                "PortNr"
#define INCF_ATTRIB_PortNr_value_DEF 3131
#define INCF_ATTRIB_PortNr_value_MIN 0
#define INCF_ATTRIB_PortNr_value_MAX 32000
#define INCF_ELEM_GroupQueueSize        "GroupQueueSize"
#define INCF_ATTRIB_GroupQueueSize_value_DEF 2000
#define INCF_ATTRIB_GroupQueueSize_value_MIN 500
#define INCF_ATTRIB_GroupQueueSize_value_MAX 10000
#define INCF_ELEM_FragmentSize          "FragmentSize"
#define INCF_ATTRIB_FragmentSize_value_DEF 1200
#define INCF_ATTRIB_FragmentSize_value_MIN 100
#define INCF_ATTRIB_FragmentSize_value_MAX 10000
#define INCF_ELEM_Partitioning          "Partitioning"
#define INCF_ELEM_GlobalPartition       "GlobalPartition"
#define INCF_ATTRIB_GlobalPartition_address "Address"
#define INCF_ATTRIB_GlobalPartition_address_DEF INCF_DEF_GLOBAL_PARTITON
#define INCF_ELEM_NetworkPartitions     "NetworkPartitions"
#define INCF_ELEM_NetworkPartition      "NetworkPartition"
#define INCF_ATTRIB_NetworkPartitions_name      "Name"
#define INCF_ATTRIB_NetworkPartitions_address   "Address"
#define INCF_ATTRIB_NetworkPartitions_connected "Connected"
#define INCF_ELEM_PartitionMappings     "PartitionMappings"
#define INCF_ELEM_PartitionMapping      "PartitionMapping"
#define INCF_ATTRIB_PartitionMapping_networkPartition "NetworkPartition"
#define INCF_ATTRIB_PartitionMapping_topicPartitionCombo "DCPSPartitionTopic"
#define INCF_ELEM_Tracing               "Tracing"
#define INCF_ELEM_OutputFile            "OutputFile"
#define INCF_ELEM_Timestamps            "Timestamps"
#define INCF_ELEM_Categories            "Categories"
#define INCF_ELEM_Default               "Default"
#define INCF_ELEM_Configuration         "Configuration"
#define INCF_ELEM_Construction          "Construction"
#define INCF_ELEM_Destruction           "Destruction"
#define INCF_ELEM_Mainloop              "Mainloop"
#define INCF_ELEM_Groups                "Groups"
#define INCF_ELEM_Send                  "Send"
#define INCF_ELEM_Receive               "Receive"
#define INCF_ELEM_Test                  "Test"
#define INCF_ELEM_DiscoveryTracing      "Discovery"
#define INCF_ELEM_Reporting             "Reporting"
#define INCF_ELEM_Debugging             "Debugging"
#define INCF_ELEM_WaitForDebugger       "WaitForDebugger"

OS_STRUCT(in_config)
{
    os_char* pathName;
    Coll_List ddsiServices;
};

static in_config _this = NULL;

in_config
in_configGetInstance(
    )
{
    if(!_this)
    {
        _this = os_malloc(sizeof(OS_STRUCT(in_config)));
        if(_this)
        {
            _this->pathName = NULL;
            Coll_List_init(&(_this->ddsiServices));
        }
    }
    return _this;
}

os_char*
in_configGetPathName(
    in_config _this)
{
    assert(_this);

    return _this->pathName;
}

Coll_List*
in_configGetDdsiServices(
    in_config _this)
{
    assert(_this);

    return &(_this->ddsiServices);
}

in_configDdsiService
in_configGetDdsiServiceByName(
    in_config _this,
    const os_char* name)
{
    in_configDdsiService ddsiService;
    in_configDdsiService foundService = NULL;
    Coll_Iter* iterator;
    os_char* serviceName;

    assert(_this);
    assert(name);

    /* iterate over all elements, if there are a lot of ddsi service configs
     * then this way of iterating is not that efficient, but it is unlikely
     * that there are more then a handful of ddsi services configured at
     * any given time
     */
    iterator = Coll_List_getFirstElement(&(_this->ddsiServices));
    while(iterator)
    {
        ddsiService = in_configDdsiService(Coll_Iter_getObject(iterator));
        serviceName = in_configDdsiServiceGetName(ddsiService);
        if(0 == strcmp(serviceName, name))
        {
            foundService = ddsiService;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    return foundService;
}


in_result
in_configConvertDomTree(
    in_config _this,
    const os_char* pathName,
    u_service service)
{
    in_result result = IN_RESULT_OK;
    u_cfElement element;

    assert(_this);
    assert(pathName);
    assert(service);

    /* store pathName */
    _this->pathName = os_strdup(pathName);
    if(!_this->pathName)
    {
        result = IN_RESULT_OUT_OF_MEMORY;
    }
    if(result == IN_RESULT_OK)
    {
        element = u_participantGetConfiguration(u_participant(service));
        if(element)
        {
            in_configTraverseConfiguration(element);
        }
    }

    return result;
}

void
in_configTraverseConfiguration(
    u_cfElement element)
{
    c_iter children;
    u_cfNode childNode;
    os_char* name;
    v_cfKind nodeKind;

    assert(element);

    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_DdsiService))
        {
            in_configTraverseDdsiServiceElement(u_cfElement(childNode));
        } /* else ignore the element */
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
}

void
in_configTraverseDdsiServiceElement(
    u_cfElement element)
{
    in_configDdsiService ddsiService;
    c_iter children;
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    os_boolean success;
    os_char* serviceName = NULL;
    u_cfNode childNode;
    v_cfKind nodeKind;

    assert(element);

    /* Step 1: read attributes */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        if(0 == strcmp(name, INCF_ATTRIB_DdsiService_name))
        {
            success = u_cfAttributeStringValue(attribute, &serviceName);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else
        {
            /* TODO ignore and report this */
        }
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    /*step 2: verify required attributes are found, then instantiate the class*/
    if(!serviceName)
    {
        /* TODO report warning, revert to default */
    }
    ddsiService = in_configDdsiServiceNew(serviceName);
    os_free(serviceName);
    if(!ddsiService)
    {
        /* TODO report out of memory error */
    }
    /* Step 3: traverse child elements */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {

        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_General))
        {
            in_configTraverseGeneralElement(u_cfElement(childNode), ddsiService);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Channels))
        {
            in_configTraverseChannelsElement(u_cfElement(childNode), ddsiService);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_DiscoveryChannel))
        {
            in_configTraverseDiscoveryChannelElement(u_cfElement(childNode), ddsiService);
        } if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Partitioning))
        {
            in_configTraversePartitioningElement(u_cfElement(childNode), ddsiService);
        } else
        {
            /* TODO ignore and report this */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);

    Coll_List_pushBack(&(in_configGetInstance()->ddsiServices),  ddsiService);
}

void
in_configTraversePartitioningElement(
    u_cfElement element,
    in_configDdsiService ddsiService)
{
    in_configPartitioning partitioning;
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        /* TODO report warning and ignore this attribute */
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    partitioning = in_configPartitioningNew();

    /* Step 2: traverse child elements */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_GlobalPartition))
        {
            in_configTraverseGlobalPartitionElement(u_cfElement(childNode), partitioning);
        } if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_NetworkPartitions))
        {
            in_configTraverseNetworkPartitionsElement(u_cfElement(childNode), partitioning);
        } if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_PartitionMappings))
        {
            in_configTraversePartitionMappingsElement(u_cfElement(childNode), partitioning);
        } else
        {
            /* TODO ignore and report this */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);

    in_configDdsiServiceSetPartitioning(ddsiService, partitioning);
}

void
in_configTraverseNetworkPartitionsElement(
    u_cfElement element,
    in_configPartitioning partitioning)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        /* TODO report warning and ignore this attribute */
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    /* Step 2: traverse child elements */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_NetworkPartition))
        {
            in_configTraverseNetworkPartitionElement(u_cfElement(childNode), partitioning);
        } else
        {
            /* TODO ignore and report this */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
}

void
in_configTraversePartitionMappingsElement(
    u_cfElement element,
    in_configPartitioning partitioning)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        /* TODO report warning and ignore this attribute */
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    /* Step 2: traverse child elements */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_PartitionMapping))
        {
            in_configTraversePartitionMappingElement(u_cfElement(childNode), partitioning);
        } else
        {
            /* TODO ignore and report this */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
}

void
in_configTraverseGlobalPartitionElement(
    u_cfElement element,
    in_configPartitioning partitioning)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* addy;
    os_char* name;
    os_boolean success;
    c_iter children;
    u_cfNode childNode;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        if(0 == strcmp(name, INCF_ATTRIB_GlobalPartition_address))
        {
            success = u_cfAttributeStringValue(attribute, &addy);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else
        {
            /* TODO report warning and ignore this attribute */
        }
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    in_configPartitioningSetGlobalPartitionAddress(partitioning, addy);
    os_free(addy);

    /* Step 2: traverse child elements, shouldnt be any, report warning if
     * any were found anyway.
     */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        /* TODO ignore and report this */
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
}

void
in_configTraverseNetworkPartitionElement(
    u_cfElement element,
    in_configPartitioning partitioning)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* addy;
    os_char* name;
    os_char* nwName;
    os_boolean connected;
    os_boolean success;
    c_iter children;
    u_cfNode childNode;
    in_configNetworkPartition nwPartition;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        if(0 == strcmp(name, INCF_ATTRIB_NetworkPartitions_name))
        {
            success = u_cfAttributeStringValue(attribute, &nwName);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_NetworkPartitions_address))
        {
            success = u_cfAttributeStringValue(attribute, &addy);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_NetworkPartitions_connected))
        {
            success = u_cfAttributeBoolValue(attribute, (c_bool*)&connected);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else
        {
            /* TODO report warning and ignore this attribute */
        }
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    nwPartition = in_configNetworkPartitionNew(nwName, addy, connected);
    if(!nwPartition)
    {
        /* TODO report out of memory error */
    } else
    {
        in_configPartitioningAddNetworkPartition(partitioning, nwPartition);
    }
    os_free(nwName);
    os_free(addy);

    /* Step 2: traverse child elements, shouldnt be any, report warning if
     * any were found anyway.
     */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        /* TODO ignore and report this */
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
}

void
in_configTraversePartitionMappingElement(
    u_cfElement element,
    in_configPartitioning partitioning)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* partitionTopicCombo;
    os_char* name;
    os_char* nwPartition;
    os_boolean success;
    c_iter children;
    u_cfNode childNode;
    in_configPartitionMapping partitionMapping;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        if(0 == strcmp(name, INCF_ATTRIB_PartitionMapping_networkPartition))
        {
            success = u_cfAttributeStringValue(attribute, &nwPartition);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_PartitionMapping_topicPartitionCombo))
        {
            success = u_cfAttributeStringValue(attribute, &partitionTopicCombo);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else
        {
            /* TODO report warning and ignore this attribute */
        }
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    partitionMapping = in_configPartitionMappingNew(
        nwPartition,
        partitionTopicCombo);
    if(!partitionMapping)
    {
        /* TODO report out of memory error */
    } else
    {
        in_configPartitioningAddPartitionMapping(partitioning, partitionMapping);
    }
    os_free(nwPartition);
    os_free(partitionTopicCombo);

    /* Step 2: traverse child elements, shouldnt be any, report warning if
     * any were found anyway.
     */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        /* TODO ignore and report this */
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
}

void
in_configTraverseGeneralElement(
    u_cfElement element,
    in_configDdsiService ddsiService)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;

    assert(element);
    assert(ddsiService);

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        /* TODO report warning and ignore this attribute */
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    /* Step 2: traverse child elements */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Interface))
        {
            in_configTraverseInterfaceElement(u_cfElement(childNode), ddsiService);
        } else
        {
            /* TODO ignore and report a warning */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
}

void
in_configTraverseChannelsElement(
    u_cfElement element,
    in_configDdsiService ddsiService)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;

    assert(element);
    assert(ddsiService);

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        /* TODO report warning and ignore this attribute */
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    /* Step 2: traverse child elements */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Channel))
        {
            in_configTraverseChannelElement(u_cfElement(childNode), ddsiService);
        } else
        {
            /* TODO ignore and report a warning */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
}

void
in_configTraverseDiscoveryChannelElement(
    u_cfElement element,
    in_configDdsiService ddsiService)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    os_boolean success;
    v_cfKind nodeKind;
    os_boolean isEnabled = INCF_ATTRIB_channel_isEnabled_DEF;
    in_configDiscoveryChannel discoveryChannel;

    assert(element);
    assert(ddsiService);

    /* Step 1: read attributes, ignore unsupported or superfluous elements
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        if(0 == strcmp(name, INCF_ATTRIB_channel_isEnabled))
        {
            success = u_cfAttributeBoolValue(attribute, (c_bool*)&isEnabled);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else
        {
            /* TODO report warning/error */
        }
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    discoveryChannel = in_configDiscoveryChannelNew(isEnabled, ddsiService);
    if(!discoveryChannel)
    {
        /* TODO report error */
    }
    /* Step 2: traverse child elements */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_PortNr))
        {
            in_configTraversePortNrElement(u_cfElement(childNode), in_configChannel(discoveryChannel));
        } else
        {
            /* TODO ignore and report a warning */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    in_configDdsiServiceSetDiscoveryChannelConfig(ddsiService, discoveryChannel);

}

void
in_configTraverseChannelElement(
    u_cfElement element,
    in_configDdsiService ddsiService)
{
    c_iter attributes;
    u_cfAttribute attribute;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_char* name = NULL;
    os_uint32 priority = INCF_ATTRIB_channel_priority_DEF;
    os_boolean isDefault = INCF_ATTRIB_channel_isDefault_DEF;
    static os_boolean defaultDefined = FALSE; /* TODO FIXME, static prevents re-configuration */
    os_boolean isEnabled = INCF_ATTRIB_channel_isEnabled_DEF;
    in_configDataChannel dataChannel;
    os_boolean success;
    os_char* channelName = "defaultName";

    assert(element);
    assert(ddsiService);

    /* Step 1: read attributes, ignore unsupported or superfluous elements
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        if(0 == strcmp(name, INCF_ATTRIB_channel_name))
        {
            success = u_cfAttributeStringValue(attribute, &channelName);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_channel_priority))
        {
            success = u_cfAttributeULongValue(attribute, &priority);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_channel_isDefault))
        {
            success = u_cfAttributeBoolValue(attribute, (c_bool*)&isDefault);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_channel_isEnabled))
        {
            success = u_cfAttributeBoolValue(attribute, (c_bool*)&isEnabled);
            if(!success)
            {
                /* TODO report warning/error */
            }
        } else
        {
            /* TODO report warning/error */
        }
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);
    if(priority <= INCF_ATTRIB_channel_priority_MIN)
    {
        priority = INCF_ATTRIB_channel_priority_MIN;
    } else if(priority > INCF_ATTRIB_channel_priority_MAX)
    {
        priority = INCF_ATTRIB_channel_priority_MAX;
    }
    if(isDefault)
    {
        if(defaultDefined)
        {
            isDefault = FALSE;
        } else
        {
            defaultDefined = TRUE;
        }
    }
    dataChannel = in_configDataChannelNew(channelName, priority, isDefault, isEnabled, ddsiService);
    os_free(channelName);
    if(!dataChannel)
    {
        /* TODO report error */
    }
    /* Step 2: traverse child elements */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_GroupQueueSize))
        {
            in_configTraverseGroupQueueElement(u_cfElement(childNode), dataChannel);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_FragmentSize))
        {
            in_configTraverseFragmentSizeElement(u_cfElement(childNode), dataChannel);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_PortNr))
        {
            in_configTraversePortNrElement(u_cfElement(childNode), in_configChannel(dataChannel));
        } else
        {
            /* TODO ignore and report a warning */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    in_configDdsiServiceAddDataChannelConfig(ddsiService, dataChannel);

}

void
in_configTraverseGroupQueueElement(
    u_cfElement element,
    in_configDataChannel dataChannel)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 queueSize = INCF_ATTRIB_GroupQueueSize_value_DEF;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        /* TODO report warning and ignore this attribute */
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    /* Step 2: traverse child element, ignore any child elements found and
     * provide a warning for them
     */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFDATA)
        {
            success = u_cfDataULongValue(u_cfData(childNode), &queueSize);
            if(!success)
            {
                /* TODO ignore and report a warning */
            }
        } else
        {
            /* TODO ignore and report a warning */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(queueSize < INCF_ATTRIB_GroupQueueSize_value_MIN)
    {
        queueSize = INCF_ATTRIB_GroupQueueSize_value_MIN;
    } else if(queueSize > INCF_ATTRIB_GroupQueueSize_value_MAX)
    {
        queueSize = INCF_ATTRIB_GroupQueueSize_value_MAX;
    }
    in_configDataChannelSetGroupQueueSize(dataChannel, queueSize);
}

void
in_configTraverseFragmentSizeElement(
    u_cfElement element,
    in_configDataChannel dataChannel)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 fragmentSize = INCF_ATTRIB_FragmentSize_value_DEF;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        /* TODO report warning and ignore this attribute */
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    /* Step 2: traverse child element, ignore any child elements found and
     * provide a warning for them
     */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFDATA)
        {
            success = u_cfDataULongValue(u_cfData(childNode), &fragmentSize);
            if(!success)
            {
                /* TODO ignore and report a warning */
            }
        } else
        {
            /* TODO ignore and report a warning */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(fragmentSize < INCF_ATTRIB_FragmentSize_value_MIN)
    {
        fragmentSize = INCF_ATTRIB_FragmentSize_value_MIN;
    } else if(fragmentSize > INCF_ATTRIB_FragmentSize_value_MAX)
    {
        fragmentSize = INCF_ATTRIB_FragmentSize_value_MAX;
    }
    in_configChannelSetFragmentSize(in_configChannel(dataChannel), fragmentSize);
}

void
in_configTraversePortNrElement(
    u_cfElement element,
    in_configChannel channel)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 portNr = INCF_ATTRIB_PortNr_value_DEF;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        /* TODO report warning and ignore this attribute */
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    /* Step 2: traverse child element, ignore any child elements found and
     * provide a warning for them
     */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFDATA)
        {
            success = u_cfDataULongValue(u_cfData(childNode), &portNr);
            if(!success)
            {
                /* TODO ignore and report a warning */
            }
        } else
        {
            /* TODO ignore and report a warning */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(portNr <= INCF_ATTRIB_PortNr_value_MIN)
    {
        portNr = INCF_ATTRIB_PortNr_value_MIN;
    } else if(portNr > INCF_ATTRIB_PortNr_value_MAX)
    {
        portNr = INCF_ATTRIB_PortNr_value_MAX;
    }
    in_configChannelSetPortNr(channel, portNr);
}

void
in_configTraverseInterfaceElement(
    u_cfElement element,
    in_configDdsiService ddsiService)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_char* networkId = INCF_ATTRIB_Interface_value_DEF;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        /* TODO report warning and ignore this attribute */
        os_free(name);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    /* Step 2: traverse child element, ignore any child elements found and
     * provide a warning for them
     */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        if(nodeKind == V_CFDATA && !networkId)
        {
            success = u_cfDataStringValue(u_cfData(childNode), &networkId);
            if(!success)
            {
                /* TODO ignore and report a warning */
            }
        } else
        {
            /* TODO ignore and report a warning */
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    in_configDdsiServiceSetNetworkId(ddsiService, networkId);
}
