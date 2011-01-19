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
#include "in__configTracing.h"
#include "in_report.h"
#include "in_align.h"

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
    in_configChannel channel);

static void
in_configTraversePortNrElement(
    u_cfElement element,
    in_configChannel channel);

static void
in_configTraverseInterfaceElement(
    u_cfElement element,
    in_configDdsiService ddsiService);

static void
in_configTraversePartitionMappingElement(
    u_cfElement element,
    in_configPartitioning partitioning);

static void
in_configTraverseNetworkPartitionElement(
    u_cfElement element,
    in_configPartitioning partitioning);

static void
in_configTraverseGlobalPartitionElement(
    u_cfElement element,
    in_configPartitioning partitioning);

static void
in_configTraversePartitionMappingsElement(
    u_cfElement element,
    in_configPartitioning partitioning);

static void
in_configTraverseNetworkPartitionsElement(
    u_cfElement element,
    in_configPartitioning partitioning);

static void
in_configTraversePartitioningElement(
    u_cfElement element,
    in_configDdsiService ddsiService);

static void
in_configTraverseTracingElement(
    u_cfElement element,
    in_configDdsiService ddsiService);

static void
in_configTraverseCategoriesElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseDefaultElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseConfigurationElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseConstructionElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseDestructionElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseMainloopElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseGroupsElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseSendElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseReceiveElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseTestElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseDiscoveryTracingElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configTraverseOutputFileElement(
    u_cfElement element,
    in_configTracing tracing);

static void
in_configFinalizeDdsiService(
    in_configDdsiService _this);

static void
in_configTraverseDebuggingElement(
    u_cfElement element,
    in_configDdsiService ddsiService);

static void
in_configFinalizeTracing(
    in_configTracing tracing);

static void
in_configFinalizePartitioning(
    in_configPartitioning partitioning);

#define INCF_ELEM_DdsiService                   "DDSIService"
#define INCF_ATTRIB_DdsiService_name            "name"
#define INCF_ELEM_General                       "General"
#define INCF_ELEM_Interface                     "NetworkInterfaceAddress"
#define INCF_ATTRIB_Interface_value_DEF         INCF_DEF_INTERFACE
#define INCF_ELEM_Channels                      "Channels"
#define INCF_ELEM_Channel                       "Channel"
#define INCF_ATTRIB_channel_isEnabled           "enabled"
#define INCF_ATTRIB_channel_priority            "priority"
#define INCF_ATTRIB_channel_isDefault           "default"
#define INCF_ATTRIB_channel_isEnabled_DEF       OS_TRUE
#define INCF_ATTRIB_channel_isDefault_DEF       OS_FALSE
#define INCF_ATTRIB_channel_priority_DEF        0
#define INCF_ATTRIB_channel_priority_MIN        0
#define INCF_ATTRIB_channel_priority_MAX        1000
#define INCF_ATTRIB_channel_name                "name"
#define INCF_ELEM_DiscoveryChannel              "Discovery"
#define INCF_ELEM_PortNr                        "PortNr"
#define INCF_ATTRIB_Discovery_PortNr_value_DEF  INCF_DEF_DISCOVERY_CHANNEL_PORT
#define INCF_ATTRIB_Data_PortNr_value_DEF       INCF_DEF_DATA_CHANNEL_PORT
#define INCF_ATTRIB_PortNr_value_MIN            0
#define INCF_ATTRIB_PortNr_value_MAX            65535
#define INCF_ELEM_GroupQueueSize                "GroupQueueSize"
#define INCF_ATTRIB_GroupQueueSize_value_DEF    500
#define INCF_ATTRIB_GroupQueueSize_value_MIN    100
#define INCF_ATTRIB_GroupQueueSize_value_MAX    10000
#define INCF_ELEM_FragmentSize                  "FragmentSize"
#define INCF_ATTRIB_FragmentSize_value_DEF      INCF_DEF_FRAGMENT_SIZE
#define INCF_ATTRIB_FragmentSize_value_MIN      100
#define INCF_ATTRIB_FragmentSize_value_MAX      10000
#define INCF_ELEM_Partitioning                  "Partitioning"
#define INCF_ELEM_GlobalPartition               "GlobalPartition"
#define INCF_ATTRIB_GlobalPartition_address     "Address"
#define INCF_ATTRIB_GlobalPartition_address_DEF INCF_DEF_GLOBAL_PARTITION
#define INCF_ELEM_NetworkPartitions             "NetworkPartitions"
#define INCF_ELEM_NetworkPartition              "NetworkPartition"
#define INCF_ATTRIB_NetworkPartitions_name      "Name"
#define INCF_ATTRIB_NetworkPartitions_address   "Address"
#define INCF_ATTRIB_NetworkPartitions_connected "Connected"
#define INCF_ATTRIB_NetworkPartitions_connected_DEF         OS_TRUE
#define INCF_ELEM_PartitionMappings             "PartitionMappings"
#define INCF_ELEM_PartitionMapping              "PartitionMapping"
#define INCF_ATTRIB_PartitionMapping_networkPartition       "NetworkPartition"
#define INCF_ATTRIB_PartitionMapping_topicPartitionCombo    "DCPSPartitionTopic"
#define INCF_ELEM_Tracing                       "Tracing"
#define INCF_ATTRIB_Tracing_isEnabled           "enabled"
#define INCF_ATTRIB_Tracing_isEnabled_DEF       OS_TRUE
#define INCF_ELEM_OutputFile                    "OutputFile"
#define INCF_ATTRIB_OutputFile_value_DEF        "ddsi-tracing-output.log"
#define INCF_ELEM_Timestamps                    "Timestamps"
#define INCF_ELEM_Categories                    "Categories"
#define INCF_ELEM_Default                       "Default"
#define INCF_ATTRIB_Default_value_DEF           1
#define INCF_ELEM_Configuration                 "Configuration"
#define INCF_ATTRIB_Configuration_value_DEF     1
#define INCF_ELEM_Construction                  "Construction"
#define INCF_ATTRIB_Construction_value_DEF      1
#define INCF_ELEM_Destruction                   "Destruction"
#define INCF_ATTRIB_Destruction_value_DEF       1
#define INCF_ELEM_Mainloop                      "Mainloop"
#define INCF_ATTRIB_Mainloop_value_DEF          1
#define INCF_ELEM_Groups                        "Groups"
#define INCF_ATTRIB_Groups_value_DEF            1
#define INCF_ELEM_Send                          "Send"
#define INCF_ATTRIB_Send_value_DEF              1
#define INCF_ELEM_Receive                       "Receive"
#define INCF_ATTRIB_Receive_value_DEF           1
#define INCF_ELEM_Test                          "Test"
#define INCF_ATTRIB_Test_value_DEF              1
#define INCF_ELEM_DiscoveryTracing              "Discovery"
#define INCF_ATTRIB_DiscoveryTracing_value_DEF  1
#define INCF_ELEM_Reporting                     "Reporting"
#define INCF_ELEM_Debugging                     "Debugging"
#define INCF_ELEM_WaitForDebugger               "WaitForDebugger"

OS_STRUCT(in_config)
{
    os_char* pathName;
    Coll_List ddsiServices;
};

static in_config _this = NULL;
/* static FILE* outputFile = NULL; */

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

void
in_configFree (
    in_config _this)
{
    Coll_List* ddsiServices;
    in_configDdsiService ddsiService;
    void * obj = NULL;
    
    assert(_this);

    /* free pathName */
    os_free (_this->pathName);

    /* free the resources allocated for the ddsiServices Coll_List */
    ddsiServices = in_configGetDdsiServices(_this);
    while ((obj = Coll_List_popBack(ddsiServices)) != NULL)
    {
       ddsiService = in_configDdsiService(obj);
       /* paranoid check */ 
       if (ddsiService)
       {
          in_configDdsiServiceFree (ddsiService);
       }
    }
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

in_configTracing
in_configGetConfigTracing ()
{
    in_config config;
    in_configDdsiService ddsiService;
    in_configTracing configTracing = NULL;
    Coll_List* ddsiServices;
    Coll_Iter* iterator;

    config = in_configGetInstance();
    if (config)
    {
        /* There will only be one ddsi service in the in_config but check that
         * the list of ddsi services contains at least one item */
        ddsiServices = in_configGetDdsiServices(config);
        iterator = Coll_List_getFirstElement(ddsiServices);
        if (iterator)
        {
           ddsiService = in_configDdsiService(Coll_Iter_getObject(iterator));
           if (ddsiService)
           {
              configTracing = in_configDdsiServiceGetTracing(ddsiService);
           }
        }
    }

    return configTracing;
}

os_boolean
in_configIsTracingEnabled ()
{
    in_configTracing configTracing;
    os_boolean isEnabled = OS_FALSE;

    configTracing = in_configGetConfigTracing();

    if (configTracing)
    {
       isEnabled = in_configTracingIsEnabled (configTracing);
    }

    return isEnabled;
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
            u_cfElementFree(element);
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
        if(name)
        {
            if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_DdsiService))
            {
                in_configTraverseDdsiServiceElement(u_cfElement(childNode));
            } /* else ignore the element, do not report that we ignore it though */
            os_free(name);
        }
        u_cfNodeFree(childNode);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
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
    os_uint32 errorCode;

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
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_DdsiService);

            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
                name,
                INCF_ELEM_DdsiService);
        }
        os_free(name);
        u_cfAttributeFree(attribute);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);
    /*step 2: verify required attributes are found, then instantiate the class*/
    if(!serviceName)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the required attribute '%s' within element '%s'! Ignoring element.",
            INCF_ATTRIB_DdsiService_name,
            INCF_ELEM_DdsiService);
    } else
    {
        /* check if a service with this name was not already known! */
        ddsiService = in_configGetDdsiServiceByName(_this, serviceName);
        if(ddsiService)
        {
            IN_REPORT_WARNING_3(
                IN_SPOT,
                "Detected duplicate definition for element '%s' with attribute '%s' set to value '%s'. Ignoring duplicate and all child elements.",
                INCF_ELEM_DdsiService,
                INCF_ATTRIB_DdsiService_name,
                serviceName);
        } else
        {
            ddsiService = in_configDdsiServiceNew(serviceName);
            if(!ddsiService)
            {
                IN_REPORT_ERROR("in_configTraverseDdsiServiceElement", "Out of memory.");
            } else
            {
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
                    } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Partitioning))
                    {
                        in_configTraversePartitioningElement(u_cfElement(childNode), ddsiService);
                    } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Debugging))
                    {
                        in_configTraverseDebuggingElement(u_cfElement(childNode), ddsiService);                      
                    } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Tracing))
                    {
                        in_configTraverseTracingElement(u_cfElement(childNode), ddsiService);
                    } else
                    {
                        IN_REPORT_WARNING_2(
                            IN_SPOT,
                            "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                            name,
                            INCF_ELEM_DdsiService);
                    }
                    os_free(name);
                    u_cfNodeFree(childNode);
                    childNode = u_cfNode(c_iterTakeFirst(children));
                }
                c_iterFree(children);

                errorCode = Coll_List_pushBack(&(in_configGetInstance()->ddsiServices),  ddsiService);
                if(errorCode != COLL_OK)
                {
                    IN_REPORT_ERROR("in_configTraverseDdsiServiceElement", "Out of memory.");
                }
            }
        }
        os_free(serviceName);
        /* finalize the configuration, set default values if not defined in config file */
        in_configFinalizeDdsiService(ddsiService);
    }
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

    partitioning = in_configDdsiServiceGetPartitioning(ddsiService);
    if(partitioning)
    {
        IN_REPORT_WARNING_4(
            IN_SPOT,
            "Detected duplicate definition for element '%s'. This element was defined within parent element '%s' which had attribute '%s' set to value '%s'. Ignoring duplicate and all child elements.",
            INCF_ELEM_Partitioning,
            INCF_ELEM_DdsiService,
            INCF_ATTRIB_DdsiService_name,
            in_configDdsiServiceGetName(ddsiService));
    } else
    {
        /* Step 1: read attributes if there are any, report warning that they are
         * ignored.
         */
        attributes = u_cfElementGetAttributes(element);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
        while(attribute)
        {
            name = u_cfNodeName(u_cfNode(attribute));
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
                name,
                INCF_ELEM_Partitioning);
            os_free(name);
            attribute = u_cfAttribute(c_iterTakeFirst(attributes));
        }
        c_iterFree(attributes);

        partitioning = in_configPartitioningNew();
        if(!partitioning)
        {
            IN_REPORT_ERROR("in_configTraversePartitioningElement", "Out of memory.");
        } else
        {
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
                } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_NetworkPartitions))
                {
                    in_configTraverseNetworkPartitionsElement(u_cfElement(childNode), partitioning);
                } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_PartitionMappings))
                {
                    in_configTraversePartitionMappingsElement(u_cfElement(childNode), partitioning);
                } else
                {
                    IN_REPORT_WARNING_2(
                        IN_SPOT,
                        "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                        name,
                        INCF_ELEM_Partitioning);
                }
                os_free(name);
                u_cfNodeFree(childNode);
                childNode = u_cfNode(c_iterTakeFirst(children));
            }
            c_iterFree(children);

            in_configDdsiServiceSetPartitioning(ddsiService, partitioning);
        }
    }
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
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_NetworkPartitions);
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
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_NetworkPartitions);
        }
        os_free(name);
        u_cfNodeFree(childNode);
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
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_PartitionMappings);
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
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_PartitionMappings);
        }
        os_free(name);
        u_cfNodeFree(childNode);
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
    os_char* addy = NULL;
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
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_GlobalPartition);
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
                name,
                INCF_ELEM_GlobalPartition);
        }
        os_free(name);
        u_cfAttributeFree(attribute);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);


    if(!addy)
    {
        IN_REPORT_WARNING_3(
            IN_SPOT,
            "Unable to locate the '%s' attribute within element '%s'! Reverting to the default value of '%s'.",
            INCF_ATTRIB_GlobalPartition_address,
            INCF_ELEM_GlobalPartition,
            INCF_ATTRIB_GlobalPartition_address_DEF);
        addy = os_malloc(strlen(INCF_ATTRIB_GlobalPartition_address_DEF));
        if(!addy)
        {
             IN_REPORT_ERROR("in_configTraverseGlobalPartitionElement", "Out of memory.");
        } else
        {
            os_strcpy(addy, INCF_ATTRIB_GlobalPartition_address_DEF);
        }
    }
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
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
            name,
            INCF_ELEM_GlobalPartition);
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);

}


void
in_configTraverseWaitForDebuggerElement(
    u_cfElement element,
    in_configDebug debug)
{
    c_iter attributes;
    u_cfAttribute attribute;
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
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
                name,
                INCF_ELEM_GlobalPartition);
        }
        os_free(name);
        u_cfAttributeFree(attribute);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    /* Step 2: traverse child elements, shouldnt be any, report warning if
     * any were found anyway.
     */
    children = u_cfElementGetChildren(element);
    childNode = u_cfNode(c_iterTakeFirst(children));
    while(childNode)
    {
    	os_uint32  waitTime = 0;
        v_cfKind nodeKind;

        name = u_cfNodeName(childNode);
        nodeKind = u_cfNodeKind(childNode);
        
        if(nodeKind == V_CFDATA)
        {
          
		  success = u_cfDataULongValue(u_cfData(childNode), &waitTime);

		  if(!success)
		  {
			  IN_REPORT_WARNING_2(
				  IN_SPOT,
				  "Failed to retrieve the value for attribute '%s' within element '%s'.",
				  name,
				  INCF_ELEM_WaitForDebugger);
		  } else 
		  {
			  in_configDebugSetWaitTime(debug, waitTime);
		  }
        } else
        {
			IN_REPORT_WARNING_2(
				IN_SPOT,
				"Unrecognized child element '%s' within element '%s'! This element will be ignored.",
				name,
				INCF_ELEM_GlobalPartition);
        }
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
    os_char* addy = NULL;
    os_char* name;
    os_char* nwName = NULL;
    os_boolean connected = INCF_ATTRIB_NetworkPartitions_connected_DEF;
    os_boolean connectedDefined = OS_FALSE;
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
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_NetworkPartition);
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_NetworkPartitions_address))
        {
            success = u_cfAttributeStringValue(attribute, &addy);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_NetworkPartition);
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_NetworkPartitions_connected))
        {
        	/* c_bool and os_boolean use two different representations, 
        	 * just casting may cause loss of information */ 
        	c_bool outVal;
        	
            success = u_cfAttributeBoolValue(attribute, (c_bool*)&outVal);
            connected = (outVal==FALSE) ? OS_FALSE : OS_TRUE;
            
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_NetworkPartition);
            } else
            {
                connectedDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
                name,
                INCF_ELEM_NetworkPartition);
        }
        os_free(name);
        u_cfAttributeFree(attribute);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);
    if(!nwName)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the required attribute '%s' within element '%s'! Ignoring element.",
            INCF_ATTRIB_NetworkPartitions_name,
            INCF_ELEM_NetworkPartition);
    } else
    {
        if(!addy)
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unable to locate the required attribute '%s' within element '%s'! Ignoring element.",
                INCF_ATTRIB_NetworkPartitions_address,
                INCF_ELEM_NetworkPartition);
        } else
        {
            if(!connectedDefined)
            {
                IN_REPORT_WARNING_3(
                    IN_SPOT,
                    "Unable to locate the '%s' attribute within element '%s'! Reverting to the default value of '%s'.",
                    INCF_ATTRIB_NetworkPartitions_connected,
                    INCF_ELEM_NetworkPartition,
                    INCF_ATTRIB_NetworkPartitions_connected_DEF);
            }
            nwPartition = in_configNetworkPartitionNew(nwName, addy, connected);
            if(!nwPartition)
            {
                IN_REPORT_ERROR("in_configTraverseNetworkPartitionElement", "Out of memory.");
            } else
            {
                in_configPartitioningAddNetworkPartition(partitioning, nwPartition);
                /* Step 2: traverse child elements, shouldnt be any, report warning if
                 * any were found anyway.
                 */
                children = u_cfElementGetChildren(element);
                childNode = u_cfNode(c_iterTakeFirst(children));
                while(childNode)
                {
                    name = u_cfNodeName(childNode);
                    IN_REPORT_WARNING_2(
                        IN_SPOT,
                        "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                        name,
                        INCF_ELEM_NetworkPartition);
                    os_free(name);
                    childNode = u_cfNode(c_iterTakeFirst(children));
                }
                c_iterFree(children);
            }
        }
    }
    if(nwName)
    {
        os_free(nwName);
    }
    if(addy)
    {
        os_free(addy);
    }
}

void
in_configTraversePartitionMappingElement(
    u_cfElement element,
    in_configPartitioning partitioning)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* partitionTopicCombo = NULL;
    os_char* name;
    os_char* nwPartition = NULL;
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
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_PartitionMapping);
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_PartitionMapping_topicPartitionCombo))
        {
            success = u_cfAttributeStringValue(attribute, &partitionTopicCombo);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_PartitionMapping);
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
                name,
                INCF_ELEM_PartitionMapping);
        }
        os_free(name);
        u_cfAttributeFree(attribute);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);
    if(!nwPartition)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the required attribute '%s' within element '%s'! Ignoring element.",
            INCF_ATTRIB_PartitionMapping_networkPartition,
            INCF_ELEM_PartitionMapping);
    } else
    {
        if(!partitionTopicCombo)
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unable to locate the required attribute '%s' within element '%s'! Ignoring element.",
                INCF_ATTRIB_PartitionMapping_topicPartitionCombo,
                INCF_ELEM_PartitionMapping);
        } else
        {
            partitionMapping = in_configPartitionMappingNew(
                nwPartition,
                partitionTopicCombo);
            if(!partitionMapping)
            {
                IN_REPORT_ERROR("in_configTraversePartitionMappingElement", "Out of memory.");
            } else
            {
                in_configPartitioningAddPartitionMapping(partitioning, partitionMapping);
                /* Step 2: traverse child elements, shouldnt be any, report warning if
                 * any were found anyway.
                 */
                children = u_cfElementGetChildren(element);
                childNode = u_cfNode(c_iterTakeFirst(children));
                while(childNode)
                {
                    name = u_cfNodeName(childNode);
                    IN_REPORT_WARNING_2(
                        IN_SPOT,
                        "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                        name,
                        INCF_ELEM_PartitionMapping);
                    os_free(name);
                    childNode = u_cfNode(c_iterTakeFirst(children));
                }
                c_iterFree(children);
            }
        }
    }
    if(nwPartition)
    {
        os_free(nwPartition);
    }
    if(partitionTopicCombo)
    {
        os_free(partitionTopicCombo);
    }
}

void
in_configTraverseTracingElement(
    u_cfElement element,
    in_configDdsiService ddsiService)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    in_configTracing tracing;
    os_boolean isEnabled = INCF_ATTRIB_Tracing_isEnabled_DEF;
    os_boolean isEnabledDefined = OS_FALSE;
    os_boolean success;

    tracing = in_configDdsiServiceGetTracing(ddsiService);
    if(tracing)
    {
        IN_REPORT_WARNING_4(
            IN_SPOT,
            "Detected duplicate definition for element '%s'. This element was defined within parent element '%s' which had attribute '%s' set to value '%s'. Ignoring duplicate and all child elements.",
            INCF_ELEM_Tracing,
            INCF_ELEM_DdsiService,
            INCF_ATTRIB_DdsiService_name,
            in_configDdsiServiceGetName(ddsiService));
    } else
    {
        tracing = in_configTracingNew();
        if(!tracing)
        {
            IN_REPORT_ERROR(IN_SPOT, "Out of memory.");
        } else
        {
            /* Step 1: read attributes if there are any, report warning that they are
             * ignored.
             */
            attributes = u_cfElementGetAttributes(element);
            attribute = u_cfAttribute(c_iterTakeFirst(attributes));
            while(attribute)
            {
                name = u_cfNodeName(u_cfNode(attribute));
                if(0 == strcmp(name, INCF_ATTRIB_Tracing_isEnabled))
                {
                	/* c_bool and os_boolean use two different representations, 
                	 * just casting may cause loss of information */ 
                	c_bool outVal = FALSE;
                	
                    success = u_cfAttributeBoolValue(attribute, (c_bool*)&outVal);
                    isEnabled = (outVal==FALSE) ? OS_FALSE : OS_TRUE;
                    
                    if(!success)
                    {
                        IN_REPORT_WARNING_2(
                            IN_SPOT,
                            "Failed to retrieve the value for attribute '%s' within element '%s'.",
                            name,
                            INCF_ELEM_Tracing);
                    } else
                    {
                        isEnabledDefined = OS_TRUE;
                    }
                } else
                {
                    IN_REPORT_WARNING_2(
                        IN_SPOT,
                        "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
                        name,
                        INCF_ELEM_Tracing);
                }
                os_free(name);
                attribute = u_cfAttribute(c_iterTakeFirst(attributes));
            }
            c_iterFree(attributes);
            if(!isEnabledDefined)
            {
                IN_REPORT_WARNING_3(
                    IN_SPOT,
                    "Unable to locate the '%s' attribute within element '%s'! Reverting to the default value of '%s'.",
                    INCF_ATTRIB_Tracing_isEnabled,
                    INCF_ELEM_Tracing,
                    INCF_ATTRIB_Tracing_isEnabled_DEF);
            }
            in_configTracingSetEnabled(tracing, isEnabled);
            /* Step 2: traverse child elements */
            children = u_cfElementGetChildren(element);
            childNode = u_cfNode(c_iterTakeFirst(children));
            while(childNode)
            {
                name = u_cfNodeName(childNode);
                nodeKind = u_cfNodeKind(childNode);
                if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_OutputFile))
                {
                    in_configTraverseOutputFileElement(u_cfElement(childNode), tracing);
                } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Categories))
                {
                    in_configTraverseCategoriesElement(u_cfElement(childNode), tracing);
                } else
                {
                    IN_REPORT_WARNING_2(
                        IN_SPOT,
                        "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                        name,
                        INCF_ELEM_Tracing);
                }
                os_free(name);
                childNode = u_cfNode(c_iterTakeFirst(children));
            }
            c_iterFree(children);
        }
        in_configDdsiServiceSetTracing(ddsiService, tracing);
    }
}

void
in_configTraverseOutputFileElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_char* outputFileName = NULL;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_OutputFile);
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
            success = u_cfDataStringValue(u_cfData(childNode), &outputFileName);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_OutputFile);
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_OutputFile);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!outputFileName)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_OutputFile,
            INCF_ATTRIB_OutputFile_value_DEF);
        outputFileName = os_malloc(sizeof(INCF_ATTRIB_OutputFile_value_DEF));
        if(!outputFileName)
        {
             IN_REPORT_ERROR(IN_SPOT, "Out of memory.");
        } else
        {
            os_strcpy(outputFileName, INCF_ATTRIB_OutputFile_value_DEF);
        }
    }
    if(outputFileName && in_configTracingIsEnabled(tracing))
    {
        in_configTracingSetOutputFile(tracing, outputFileName);
    }
}

void
in_configTraverseCategoriesElement(
    u_cfElement element,
    in_configTracing tracing)
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
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Categories);
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
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Default))
        {
            in_configTraverseDefaultElement(u_cfElement(childNode), tracing);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Configuration))
        {
            in_configTraverseConfigurationElement(u_cfElement(childNode), tracing);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Construction))
        {
            in_configTraverseConstructionElement(u_cfElement(childNode), tracing);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Destruction))
        {
            in_configTraverseDestructionElement(u_cfElement(childNode), tracing);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Mainloop))
        {
            in_configTraverseMainloopElement(u_cfElement(childNode), tracing);
        }  else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Groups))
        {
            in_configTraverseGroupsElement(u_cfElement(childNode), tracing);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Send))
        {
            in_configTraverseSendElement(u_cfElement(childNode), tracing);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Receive))
        {
            in_configTraverseReceiveElement(u_cfElement(childNode), tracing);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_Test))
        {
            in_configTraverseTestElement(u_cfElement(childNode), tracing);
        } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_DiscoveryTracing))
        {
            in_configTraverseDiscoveryTracingElement(u_cfElement(childNode), tracing);
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Categories);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
}

void
in_configTraverseDefaultElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 level = INCF_ATTRIB_Default_value_DEF;
    os_boolean levelDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Default);
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
            success = u_cfDataULongValue(u_cfData(childNode), &level);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Default);
            } else
            {
                levelDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Default);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!levelDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_Default,
            INCF_ATTRIB_Default_value_DEF);
    }
    in_configTracingSetDefaultLevel(tracing, level);
}

void
in_configTraverseConfigurationElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 level = INCF_ATTRIB_Configuration_value_DEF;
    os_boolean levelDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Configuration);
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
            success = u_cfDataULongValue(u_cfData(childNode), &level);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Configuration);
            } else
            {
                levelDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Configuration);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!levelDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_Configuration,
            INCF_ATTRIB_Configuration_value_DEF);
    }
    in_configTracingSetConfigurationLevel(tracing, level);
}

void
in_configTraverseConstructionElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 level = INCF_ATTRIB_Construction_value_DEF;
    os_boolean levelDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Construction);
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
            success = u_cfDataULongValue(u_cfData(childNode), &level);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Construction);
            } else
            {
                levelDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Construction);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!levelDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_Construction,
            INCF_ATTRIB_Construction_value_DEF);
    }
    in_configTracingSetInitLevel(tracing, level);
}

void
in_configTraverseDestructionElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 level = INCF_ATTRIB_Destruction_value_DEF;
    os_boolean levelDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Destruction);
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
            success = u_cfDataULongValue(u_cfData(childNode), &level);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Destruction);
            } else
            {
                levelDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Destruction);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!levelDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_Destruction,
            INCF_ATTRIB_Destruction_value_DEF);
    }
    in_configTracingSetDeinitLevel(tracing, level);
}

void
in_configTraverseMainloopElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 level = INCF_ATTRIB_Mainloop_value_DEF;
    os_boolean levelDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Mainloop);
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
            success = u_cfDataULongValue(u_cfData(childNode), &level);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Mainloop);
            } else
            {
                levelDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Mainloop);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!levelDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_Mainloop,
            INCF_ATTRIB_Mainloop_value_DEF);
    }
    in_configTracingSetMainloopLevel(tracing, level);
}

void
in_configTraverseGroupsElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 level = INCF_ATTRIB_Groups_value_DEF;
    os_boolean levelDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Groups);
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
            success = u_cfDataULongValue(u_cfData(childNode), &level);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Groups);
            } else
            {
                levelDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Groups);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!levelDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_Groups,
            INCF_ATTRIB_Groups_value_DEF);
    }
    in_configTracingSetGroupsLevel(tracing, level);
}

void
in_configTraverseSendElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 level = INCF_ATTRIB_Send_value_DEF;
    os_boolean levelDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Send);
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
            success = u_cfDataULongValue(u_cfData(childNode), &level);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Send);
            } else
            {
                levelDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Send);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!levelDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_Send,
            INCF_ATTRIB_Send_value_DEF);
    }
    in_configTracingSetWritingLevel(tracing, level);
}

void
in_configTraverseReceiveElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 level = INCF_ATTRIB_Receive_value_DEF;
    os_boolean levelDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Receive);
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
            success = u_cfDataULongValue(u_cfData(childNode), &level);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Receive);
            } else
            {
                levelDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Receive);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!levelDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_Receive,
            INCF_ATTRIB_Receive_value_DEF);
    }
    in_configTracingSetReadingLevel(tracing, level);
}

void
in_configTraverseTestElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 level = INCF_ATTRIB_Test_value_DEF;
    os_boolean levelDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Test);
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
            success = u_cfDataULongValue(u_cfData(childNode), &level);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Test);
            } else
            {
                levelDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Test);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!levelDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_Test,
            INCF_ATTRIB_Test_value_DEF);
    }
    in_configTracingSetTestLevel(tracing, level);
}

void
in_configTraverseDiscoveryTracingElement(
    u_cfElement element,
    in_configTracing tracing)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    os_boolean success;
    os_uint32 level = INCF_ATTRIB_DiscoveryTracing_value_DEF;
    os_boolean levelDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_DiscoveryTracing);
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
            success = u_cfDataULongValue(u_cfData(childNode), &level);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_DiscoveryTracing);
            } else
            {
                levelDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_DiscoveryTracing);
        }
        os_free(name);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!levelDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_DiscoveryTracing,
            INCF_ATTRIB_DiscoveryTracing_value_DEF);
    }
    in_configTracingSetDiscoveryLevel(tracing, level);
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
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_General);
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
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_General);
        }
        os_free(name);
        u_cfNodeFree(childNode);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
}

void
in_configTraverseDebuggingElement(
    u_cfElement element,
    in_configDdsiService ddsiService)
{
    c_iter attributes;
    u_cfAttribute attribute;
    os_char* name;
    c_iter children;
    u_cfNode childNode;
    v_cfKind nodeKind;
    in_configDebug ddsiDebug = NULL;
    
    assert(element);
    assert(ddsiService);
    
    /* allocate the configDebug object */
    ddsiDebug = in_configDebugNew();
    if (!ddsiDebug) {
    	IN_REPORT_ERROR(IN_SPOT, "out of memory");
    	return;
    }
    in_configDdsiServiceSetDebugging(ddsiService, ddsiDebug);
        
    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_General);
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
      
        /* Fro now just parse the WaitForDebugger */ 
        if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_WaitForDebugger))
        {
            in_configTraverseWaitForDebuggerElement(u_cfElement(childNode), ddsiDebug);
        } else 
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_General);
        }
        os_free(name);
        u_cfNodeFree(childNode);
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
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Channels);
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
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Channels);
        }
        os_free(name);
        u_cfNodeFree(childNode);
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
    os_boolean isEnabledDefined = OS_FALSE;

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
        	/* c_bool and os_boolean use two different representations, 
        	 * just casting may cause loss of information */ 
        	c_bool outVal = FALSE;
        	
            success = u_cfAttributeBoolValue(attribute, (c_bool*)&outVal);
            isEnabled = (outVal==FALSE) ? OS_FALSE : OS_TRUE;
            
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_DiscoveryChannel);
            } else
            {
                isEnabledDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
                name,
                INCF_ELEM_DiscoveryChannel);
        }
        os_free(name);
        u_cfAttributeFree(attribute);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);
    if(!isEnabledDefined)
    {
        IN_REPORT_WARNING_3(
            IN_SPOT,
            "Unable to locate the '%s' attribute within element '%s'! Reverting to the default value of '%s'.",
            INCF_ATTRIB_channel_isEnabled,
            INCF_ELEM_DiscoveryChannel,
            IN_STRINGIFY_BOOLEAN(INCF_ATTRIB_channel_isEnabled_DEF));
    }
    discoveryChannel = in_configDiscoveryChannelNew(isEnabled, ddsiService);
    if(!discoveryChannel)
    {
        IN_REPORT_ERROR("in_configTraverseDiscoveryChannelElement", "Out of memory.");
    } else
    {
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
            }
            else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_FragmentSize))
            {
                in_configTraverseFragmentSizeElement(u_cfElement(childNode), in_configChannel(discoveryChannel));
            }
            else
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                    name,
                    INCF_ELEM_DiscoveryChannel);
            }
            os_free(name);
            u_cfNodeFree(childNode);
            childNode = u_cfNode(c_iterTakeFirst(children));
        }
        c_iterFree(children);
        in_configDdsiServiceSetDiscoveryChannel(ddsiService, discoveryChannel);
    }

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
    os_boolean isEnabled = INCF_ATTRIB_channel_isEnabled_DEF;
    os_char* channelName = NULL;
    os_boolean priorityDefined = OS_FALSE;
    os_boolean isDefaultDefined = OS_FALSE;
    os_boolean isEnabledDefined = OS_FALSE;
    os_boolean channelNameDefined = OS_FALSE;
    in_configDataChannel dataChannel;
    os_boolean success;

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
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Channel);
            } else
            {
                channelNameDefined = OS_TRUE;
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_channel_priority))
        {
            success = u_cfAttributeULongValue(attribute, &priority);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Channel);
            } else
            {
                priorityDefined = OS_TRUE;
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_channel_isDefault))
        {
        	/* c_bool and os_boolean use two different representations, 
        	 * just casting may cause loss of information */ 
        	c_bool outVal = FALSE;
            
        	success = u_cfAttributeBoolValue(attribute, (c_bool*)&outVal);
            isDefault = (outVal==FALSE) ? OS_FALSE : OS_TRUE;
            
        	if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Channel);
            } else
            {
                isDefaultDefined = OS_TRUE;
            }
        } else if(0 == strcmp(name, INCF_ATTRIB_channel_isEnabled))
        {
        	/* c_bool and os_boolean use two different representations, 
        	 * just casting may cause loss of information */ 
        	c_bool outVal = FALSE;
        	
            success = u_cfAttributeBoolValue(attribute, (c_bool*)&outVal);
            isEnabled = (outVal==FALSE) ? OS_FALSE : OS_TRUE;
            
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Channel);
            } else
            {
                isEnabledDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
                name,
                INCF_ELEM_Channel);
        }
        os_free(name);
        u_cfAttributeFree(attribute);
        attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    }
    c_iterFree(attributes);

    if(!channelNameDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the required attribute '%s' within element '%s'! Ignoring element.",
            INCF_ATTRIB_channel_name,
            INCF_ELEM_Channel);
    } else
    {
        if(!priorityDefined)
        {
            IN_REPORT_WARNING_3(
                IN_SPOT,
                "Unable to locate the '%s' attribute within element '%s'! Reverting to the default value of '%d'.",
                INCF_ATTRIB_channel_priority,
                INCF_ELEM_Channel,
                INCF_ATTRIB_channel_priority_DEF);
        }
        if(!isDefaultDefined)
        {
            IN_REPORT_WARNING_3(
                IN_SPOT,
                "Unable to locate the '%s' attribute within element '%s'! Reverting to the default value of '%s'.",
                INCF_ATTRIB_channel_isDefault,
                INCF_ELEM_Channel,
                IN_STRINGIFY_BOOLEAN(INCF_ATTRIB_channel_isDefault_DEF));
        }
        if(!isEnabledDefined)
        {
            IN_REPORT_WARNING_3(
                IN_SPOT,
                "Unable to locate the '%s' attribute within element '%s'! Reverting to the default value of '%s'.",
                INCF_ATTRIB_channel_isEnabled,
                INCF_ELEM_Channel,
                IN_STRINGIFY_BOOLEAN(INCF_ATTRIB_channel_isEnabled_DEF));
        }
        if(priority <= INCF_ATTRIB_channel_priority_MIN)
        {
            IN_REPORT_WARNING_4(
                IN_SPOT,
                "Attribute '%s'  within element '%s' with value %d is below the minimum value for this attribute! Reverting to the minimum value of '%d'.",
                INCF_ATTRIB_channel_priority,
                INCF_ELEM_Channel,
                priority,
                INCF_ATTRIB_channel_priority_MIN);
            priority = INCF_ATTRIB_channel_priority_MIN;
        } else if(priority > INCF_ATTRIB_channel_priority_MAX)
        {
            IN_REPORT_WARNING_4(
                IN_SPOT,
                "Attribute '%s'  within element '%s' with value %d is above the maximum value for this attribute! Reverting to the maximum value of '%d'.",
                INCF_ATTRIB_channel_priority,
                INCF_ELEM_Channel,
                priority,
                INCF_ATTRIB_channel_priority_MAX);
            priority = INCF_ATTRIB_channel_priority_MAX;
        }
        if(isDefault)
        {
            if(in_configDdsiServiceHasDefaultChannel(ddsiService))
            {
                IN_REPORT_WARNING_7(
                    IN_SPOT,
                    "Detected a second default '%s' with '%s'='%s' within '%s' with '%s'='%s'. Only one default channel allowed per '%s', setting this channel default status to false.",
                    INCF_ELEM_Channel,
                    INCF_ATTRIB_channel_name,
                    channelName,
                    INCF_ELEM_DdsiService,
                    INCF_ATTRIB_DdsiService_name,
                    in_configDdsiServiceGetName(ddsiService),
                    INCF_ELEM_Channel);
                isDefault = OS_FALSE;
            } else
            {
                in_configDdsiServiceSetHasDefaultChannel(ddsiService, OS_TRUE);
            }
        }
        dataChannel = in_configDataChannelNew(channelName, priority, isDefault, isEnabled, ddsiService);
        os_free(channelName);
        if(!dataChannel)
        {
            IN_REPORT_ERROR("in_configTraverseDiscoveryChannelElement", "Out of memory.");
        } else
        {
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
                    in_configTraverseFragmentSizeElement(u_cfElement(childNode), in_configChannel(dataChannel));
                } else if(nodeKind == V_CFELEMENT && 0 == strcmp(name, INCF_ELEM_PortNr))
                {
                    in_configTraversePortNrElement(u_cfElement(childNode), in_configChannel(dataChannel));
                } else
                {
                    IN_REPORT_WARNING_2(
                        IN_SPOT,
                        "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                        name,
                        INCF_ELEM_DiscoveryChannel);
                }
                os_free(name);
                u_cfNodeFree(childNode);
                childNode = u_cfNode(c_iterTakeFirst(children));
            }
            c_iterFree(children);
            in_configDdsiServiceAddDataChannelConfig(ddsiService, dataChannel);
        }

    }
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
    os_boolean queueSizeDefined = OS_FALSE;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_GroupQueueSize);
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
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_GroupQueueSize);
            } else
            {
                queueSizeDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_GroupQueueSize);
        }
        os_free(name);
        u_cfNodeFree(childNode);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!queueSizeDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_GroupQueueSize,
            INCF_ATTRIB_GroupQueueSize_value_DEF);
    }
    if(queueSize < INCF_ATTRIB_GroupQueueSize_value_MIN)
    {
        IN_REPORT_WARNING_3(
            IN_SPOT,
            "The data value within element '%s' with value %d is below the minimum value for this attribute! Reverting to the minimum value of '%d'.",
            INCF_ELEM_GroupQueueSize,
            queueSize,
            INCF_ATTRIB_GroupQueueSize_value_MIN);
        queueSize = INCF_ATTRIB_GroupQueueSize_value_MIN;
    } else if(queueSize > INCF_ATTRIB_GroupQueueSize_value_MAX)
    {
        IN_REPORT_WARNING_3(
            IN_SPOT,
            "The data value within element '%s' with value %d is above the maximum value for this attribute! Reverting to the maximum value of '%d'.",
            INCF_ELEM_GroupQueueSize,
            queueSize,
            INCF_ATTRIB_GroupQueueSize_value_MAX);
        queueSize = INCF_ATTRIB_GroupQueueSize_value_MAX;
    }
    in_configDataChannelSetGroupQueueSize(dataChannel, queueSize);
}

void
in_configTraverseFragmentSizeElement(
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
    os_uint32 fragmentSize = INCF_ATTRIB_FragmentSize_value_DEF;
    os_boolean fragmentSizeDefined = OS_FALSE;
    os_uint32 fragmentSizeAligned;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_FragmentSize);
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
            success = u_cfDataSizeValue(u_cfData(childNode), &fragmentSize);
            if(!success)
            {
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_FragmentSize);
            } else
            {
                fragmentSizeDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_FragmentSize);
        }
        os_free(name);
        u_cfNodeFree(childNode);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!fragmentSizeDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_FragmentSize,
            INCF_ATTRIB_FragmentSize_value_DEF);
    }
    if(fragmentSize < INCF_ATTRIB_FragmentSize_value_MIN)
    {
        IN_REPORT_WARNING_3(
            IN_SPOT,
            "The data value within element '%s' with value %d is below the minimum value for this attribute! Reverting to the minimum value of '%d'.",
            INCF_ELEM_FragmentSize,
            fragmentSize,
            INCF_ATTRIB_FragmentSize_value_MIN);
        fragmentSize = INCF_ATTRIB_FragmentSize_value_MIN;
    } else if(fragmentSize > INCF_ATTRIB_FragmentSize_value_MAX)
    {
        IN_REPORT_WARNING_3(
            IN_SPOT,
            "The data value within element '%s' with value %d is above the maximum value for this attribute! Reverting to the maximum value of '%d'.",
            INCF_ELEM_FragmentSize,
            fragmentSize,
            INCF_ATTRIB_FragmentSize_value_MAX);
        fragmentSize = INCF_ATTRIB_FragmentSize_value_MAX;
    }
    /* check if fragment size has an efficient alignment */
    fragmentSizeAligned = (os_uint32) IN_ALIGN_UINT_FLOOR(fragmentSize, 8U);
    if (fragmentSize < fragmentSizeAligned)
    {
        IN_REPORT_WARNING_3(
            IN_SPOT,
            "Defined %s %d not multiple of 8U. Defining %s as a multiple of 8U will increase efficiency.",
            INCF_ELEM_FragmentSize,
            fragmentSize,
            INCF_ELEM_FragmentSize);
    }
    in_configChannelSetFragmentSize(channel, fragmentSize);
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
    os_uint32 portNr;
    os_boolean portNrDefined = OS_FALSE;
    in_configChannelKind kind;

    assert(element);
    assert(channel);

    kind = in_configChannelGetKind(channel);
    if(kind == IN_CONFIG_CHANNEL_DISCOVERY)
    {
        portNr = INCF_ATTRIB_Discovery_PortNr_value_DEF;
    } else
    {
        assert(kind == IN_CONFIG_CHANNEL_DATA);
        portNr = INCF_ATTRIB_Data_PortNr_value_DEF;
    }

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_PortNr);
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
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_PortNr);
            } else
            {
                portNrDefined = OS_TRUE;
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_PortNr);
        }
        os_free(name);
        u_cfNodeFree(childNode);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    c_iterFree(children);
    if(!portNrDefined)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%d'.",
            INCF_ELEM_PortNr,
            portNr);
    }
    if(portNr <= INCF_ATTRIB_PortNr_value_MIN)
    {
        IN_REPORT_WARNING_3(
            IN_SPOT,
            "The data value within element '%s' with value %d is below the minimum value for this attribute! Reverting to the minimum value of '%d'.",
            INCF_ELEM_PortNr,
            portNr,
            INCF_ATTRIB_PortNr_value_MIN);
        portNr = INCF_ATTRIB_PortNr_value_MIN;
    } else if(portNr > INCF_ATTRIB_PortNr_value_MAX)
    {
        IN_REPORT_WARNING_3(
            IN_SPOT,
            "The data value within element '%s' with value %d is above the maximum value for this attribute! Reverting to the maximum value of '%d'.",
            INCF_ELEM_PortNr,
            portNr,
            INCF_ATTRIB_PortNr_value_MAX);
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
    os_char* networkId = NULL;

    /* Step 1: read attributes if there are any, report warning that they are
     * ignored.
     */
    attributes = u_cfElementGetAttributes(element);
    attribute = u_cfAttribute(c_iterTakeFirst(attributes));
    while(attribute)
    {
        name = u_cfNodeName(u_cfNode(attribute));
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unrecognized attribute '%s' within element '%s'! This attribute will be ignored.",
            name,
            INCF_ELEM_Interface);
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
                IN_REPORT_WARNING_2(
                    IN_SPOT,
                    "Failed to retrieve the value for attribute '%s' within element '%s'.",
                    name,
                    INCF_ELEM_Interface);
            }
        } else
        {
            IN_REPORT_WARNING_2(
                IN_SPOT,
                "Unrecognized child element '%s' within element '%s'! This element will be ignored.",
                name,
                INCF_ELEM_Interface);
        }
        os_free(name);
        u_cfNodeFree(childNode);
        childNode = u_cfNode(c_iterTakeFirst(children));
    }
    if(!networkId)
    {
        IN_REPORT_WARNING_2(
            IN_SPOT,
            "Unable to locate the data value within element '%s'! Reverting to the default value of '%s'.",
            INCF_ELEM_Interface,
            INCF_ATTRIB_Interface_value_DEF);
        networkId = os_malloc(sizeof(INCF_ATTRIB_Interface_value_DEF));
        if(!networkId)
        {
             IN_REPORT_ERROR("in_configTraverseChannelElement", "Out of memory.");
        } else
        {
            os_strcpy(networkId, INCF_ATTRIB_Interface_value_DEF);
        }
    }
    c_iterFree(children);
    in_configDdsiServiceSetNetworkId(ddsiService, networkId);
    if(networkId)
    {
        os_free(networkId);
    }
}

void
in_configFinalizeDdsiService(
    in_configDdsiService ddsiService)
{
    os_char* tmp;
    in_configPartitioning partitioning;
    in_configTracing tracing;
    in_configDiscoveryChannel discoveryChannel;

    assert(ddsiService);

    /* TODO finalize the configuration components with default values. */

    tmp = in_configDdsiServiceGetInterfaceId(ddsiService);
    if(!tmp)
    {
        in_configDdsiServiceSetNetworkId(ddsiService, INCF_ATTRIB_Interface_value_DEF);
        /* TODO print message saying default was chosen */
    }

    partitioning = in_configDdsiServiceGetPartitioning(ddsiService);
    if(!partitioning)
    {
        partitioning = in_configPartitioningNew();

        in_configDdsiServiceSetPartitioning(ddsiService, partitioning);
    }
    in_configFinalizePartitioning(partitioning);

    tracing = in_configDdsiServiceGetTracing(ddsiService);
    if (tracing)
    {
        /* Finalize the Tracing, if it exists */
        in_configFinalizeTracing(tracing);
    }

    discoveryChannel = in_configDdsiServiceGetDiscoveryChannel(ddsiService);
    if(!discoveryChannel)
    {
        discoveryChannel = in_configDiscoveryChannelNew(INCF_ATTRIB_channel_isEnabled_DEF, ddsiService);
        in_configDdsiServiceSetDiscoveryChannel(ddsiService, discoveryChannel);
    }

/* TODO define if finalization is needed for the following:
    Coll_List channels;
*/
}

void
in_configFinalizePartitioning(
    in_configPartitioning partitioning)
{
    os_char* globalPartitionAddress;

    assert(partitioning);

    globalPartitionAddress = in_configPartitioningGetGlobalPartitionAddress(partitioning);
    if(!globalPartitionAddress)
    {
        globalPartitionAddress = os_malloc(strlen(INCF_ATTRIB_GlobalPartition_address_DEF)+1);
        if(!globalPartitionAddress)
        {
             IN_REPORT_ERROR("in_configFinalizePartitioning", "Out of memory.");
        } else
        {
            os_strcpy(globalPartitionAddress, INCF_ATTRIB_GlobalPartition_address_DEF);
        }
        in_configPartitioningSetGlobalPartitionAddress(partitioning, globalPartitionAddress);
        /* TODO print message saying default was chosen */
    }
}

void
in_configFinalizeTracing(
    in_configTracing tracing)
{
    os_char* outputFileName;
    os_boolean isEnabled;

    assert(tracing);

    /* check whether the tracing is enabled before opening the output file */
    isEnabled = in_configTracingIsEnabled(tracing);
    if (isEnabled)
    {
        /* if the output file has not been set within the xml, use the default */
        outputFileName = in_configTracingGetOutputFileName(tracing);
        if(!outputFileName)
        {
            outputFileName = os_malloc(strlen(INCF_ATTRIB_OutputFile_value_DEF) + 1);
            if(!outputFileName)
            {
                IN_REPORT_ERROR("in_configFinalizeTracing", "Out of memory.");
            } else
            {
                os_strcpy(outputFileName, INCF_ATTRIB_OutputFile_value_DEF);
            }
            in_configTracingSetOutputFile(tracing, outputFileName);

            /* report to the user that the default output file was chosen */
            IN_REPORT_INFO_1(1, "Tracing enabled but OutputFile not specified, using default %s", outputFileName);
        }
        /* now call fopen on the file so it is ready to be written to */
        in_configTracingOpenOutputFile(tracing);
    }
    /* TODO finalize timestamps */
}

