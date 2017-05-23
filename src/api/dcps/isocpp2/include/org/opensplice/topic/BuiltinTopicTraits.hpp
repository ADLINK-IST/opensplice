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
#ifndef ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_TRAITS_HPP
#define ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_TRAITS_HPP

#include <dds/topic/BuiltinTopic.hpp>
#include <org/opensplice/topic/BuiltinTopic.hpp>
#include <org/opensplice/topic/TopicTraits.hpp>
#include <org/opensplice/topic/BuiltinTopicCopy.hpp>



namespace org { namespace opensplice { namespace topic {
template <>
class TopicTraits<dds::topic::ParticipantBuiltinTopicData>
{
public:
    static const char *getKeyList()
    {
        return "key.localId,key.systemId";
    }

    static const char *getTypeName()
    {
        return "kernelModule::v_participantInfo";
    }

    static std::string getDescriptor()
    {
        const char *elements[] = {
            "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\">",
"<Long/></Array></TypeDef><TypeDef name=\"octSeq\"><Sequence><Octet/></Sequence></TypeDef><Struct name=\"UserDataQosPolicy\">",
"<Member name=\"value\"><Type name=\"octSeq\"/></Member></Struct><Struct name=\"ParticipantBuiltinTopicData\">",
"<Member name=\"key\"><Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"user_data\"><Type name=\"UserDataQosPolicy\"/>",
"</Member></Struct></Module></MetaData>"
        };
        std::string descriptor;
        descriptor.reserve(499);
        for (int i = 0; i < 5; i++) {
            descriptor.append(elements[i]);
        }

        return descriptor;
    }

    static copyInFunction getCopyIn()
    {
        return (copyInFunction) __ParticipantBuiltinTopicData__copyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return (copyOutFunction) __ParticipantBuiltinTopicData__copyOut;
    }
};
}}}

namespace dds { namespace topic {
template <>
struct topic_type_name<dds::topic::ParticipantBuiltinTopicData>
{
    static std::string value()
    {
        return org::opensplice::topic::TopicTraits<dds::topic::ParticipantBuiltinTopicData>::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(dds::topic::ParticipantBuiltinTopicData);

namespace org { namespace opensplice { namespace topic {
template <>
class TopicTraits<dds::topic::TopicBuiltinTopicData>
{
public:
    static const char *getKeyList()
    {
        return "key.localId,key.systemId";
    }

    static const char *getTypeName()
    {
        return "kernelModule::v_topicInfo";
    }

    static std::string getDescriptor()
    {
        const char *elements[] = {
            "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\">",
"<Long/></Array></TypeDef><Enum name=\"DurabilityQosPolicyKind\"><Element name=\"VOLATILE_DURABILITY_QOS\" value=\"0\"/>",
"<Element name=\"TRANSIENT_LOCAL_DURABILITY_QOS\" value=\"1\"/><Element name=\"TRANSIENT_DURABILITY_QOS\" value=\"2\"/>",
"<Element name=\"PERSISTENT_DURABILITY_QOS\" value=\"3\"/></Enum><Struct name=\"Duration_t\"><Member name=\"sec\">",
"<Long/></Member><Member name=\"nanosec\"><ULong/></Member></Struct><Enum name=\"HistoryQosPolicyKind\">",
"<Element name=\"KEEP_LAST_HISTORY_QOS\" value=\"0\"/><Element name=\"KEEP_ALL_HISTORY_QOS\" value=\"1\"/>",
"</Enum><Enum name=\"LivelinessQosPolicyKind\"><Element name=\"AUTOMATIC_LIVELINESS_QOS\" value=\"0\"/>",
"<Element name=\"MANUAL_BY_PARTICIPANT_LIVELINESS_QOS\" value=\"1\"/><Element name=\"MANUAL_BY_TOPIC_LIVELINESS_QOS\" value=\"2\"/>",
"</Enum><Enum name=\"ReliabilityQosPolicyKind\"><Element name=\"BEST_EFFORT_RELIABILITY_QOS\" value=\"0\"/>",
"<Element name=\"RELIABLE_RELIABILITY_QOS\" value=\"1\"/></Enum><Struct name=\"TransportPriorityQosPolicy\">",
"<Member name=\"value\"><Long/></Member></Struct><Enum name=\"DestinationOrderQosPolicyKind\"><Element name=\"BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS\" value=\"0\"/>",
"<Element name=\"BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS\" value=\"1\"/></Enum><Struct name=\"ResourceLimitsQosPolicy\">",
"<Member name=\"max_samples\"><Long/></Member><Member name=\"max_instances\"><Long/></Member><Member name=\"max_samples_per_instance\">",
"<Long/></Member></Struct><Enum name=\"OwnershipQosPolicyKind\"><Element name=\"SHARED_OWNERSHIP_QOS\" value=\"0\"/>",
"<Element name=\"EXCLUSIVE_OWNERSHIP_QOS\" value=\"1\"/></Enum><TypeDef name=\"octSeq\"><Sequence><Octet/>",
"</Sequence></TypeDef><Struct name=\"DurabilityQosPolicy\"><Member name=\"kind\"><Type name=\"DurabilityQosPolicyKind\"/>",
"</Member></Struct><Struct name=\"LifespanQosPolicy\"><Member name=\"duration\"><Type name=\"Duration_t\"/>",
"</Member></Struct><Struct name=\"LatencyBudgetQosPolicy\"><Member name=\"duration\"><Type name=\"Duration_t\"/>",
"</Member></Struct><Struct name=\"DeadlineQosPolicy\"><Member name=\"period\"><Type name=\"Duration_t\"/>",
"</Member></Struct><Struct name=\"HistoryQosPolicy\"><Member name=\"kind\"><Type name=\"HistoryQosPolicyKind\"/>",
"</Member><Member name=\"depth\"><Long/></Member></Struct><Struct name=\"DurabilityServiceQosPolicy\">",
"<Member name=\"service_cleanup_delay\"><Type name=\"Duration_t\"/></Member><Member name=\"history_kind\">",
"<Type name=\"HistoryQosPolicyKind\"/></Member><Member name=\"history_depth\"><Long/></Member><Member name=\"max_samples\">",
"<Long/></Member><Member name=\"max_instances\"><Long/></Member><Member name=\"max_samples_per_instance\">",
"<Long/></Member></Struct><Struct name=\"LivelinessQosPolicy\"><Member name=\"kind\"><Type name=\"LivelinessQosPolicyKind\"/>",
"</Member><Member name=\"lease_duration\"><Type name=\"Duration_t\"/></Member></Struct><Struct name=\"ReliabilityQosPolicy\">",
"<Member name=\"kind\"><Type name=\"ReliabilityQosPolicyKind\"/></Member><Member name=\"max_blocking_time\">",
"<Type name=\"Duration_t\"/></Member><Member name=\"synchronous\"><Boolean/></Member></Struct><Struct name=\"DestinationOrderQosPolicy\">",
"<Member name=\"kind\"><Type name=\"DestinationOrderQosPolicyKind\"/></Member></Struct><Struct name=\"OwnershipQosPolicy\">",
"<Member name=\"kind\"><Type name=\"OwnershipQosPolicyKind\"/></Member></Struct><Struct name=\"TopicDataQosPolicy\">",
"<Member name=\"value\"><Type name=\"octSeq\"/></Member></Struct><Struct name=\"TopicBuiltinTopicData\">",
"<Member name=\"key\"><Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"name\"><String/></Member>",
"<Member name=\"type_name\"><String/></Member><Member name=\"durability\"><Type name=\"DurabilityQosPolicy\"/>",
"</Member><Member name=\"durability_service\"><Type name=\"DurabilityServiceQosPolicy\"/></Member><Member name=\"deadline\">",
"<Type name=\"DeadlineQosPolicy\"/></Member><Member name=\"latency_budget\"><Type name=\"LatencyBudgetQosPolicy\"/>",
"</Member><Member name=\"liveliness\"><Type name=\"LivelinessQosPolicy\"/></Member><Member name=\"reliability\">",
"<Type name=\"ReliabilityQosPolicy\"/></Member><Member name=\"transport_priority\"><Type name=\"TransportPriorityQosPolicy\"/>",
"</Member><Member name=\"lifespan\"><Type name=\"LifespanQosPolicy\"/></Member><Member name=\"destination_order\">",
"<Type name=\"DestinationOrderQosPolicy\"/></Member><Member name=\"history\"><Type name=\"HistoryQosPolicy\"/>",
"</Member><Member name=\"resource_limits\"><Type name=\"ResourceLimitsQosPolicy\"/></Member><Member name=\"ownership\">",
"<Type name=\"OwnershipQosPolicy\"/></Member><Member name=\"topic_data\"><Type name=\"TopicDataQosPolicy\"/>",
"</Member></Struct></Module></MetaData>"
        };
        std::string descriptor;
        descriptor.reserve(4735);
        for (int i = 0; i < 42; i++) {
            descriptor.append(elements[i]);
        }

        return descriptor;
    }

    static copyInFunction getCopyIn()
    {
        return (copyInFunction) __TopicBuiltinTopicData__copyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return (copyOutFunction) __TopicBuiltinTopicData__copyOut;
    }
};
}}}

namespace dds { namespace topic {
template <>
struct topic_type_name<dds::topic::TopicBuiltinTopicData>
{
    static std::string value()
    {
        return org::opensplice::topic::TopicTraits<dds::topic::TopicBuiltinTopicData>::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(dds::topic::TopicBuiltinTopicData);


namespace org { namespace opensplice { namespace topic {
template <>
class TopicTraits<dds::topic::PublicationBuiltinTopicData>
{
public:
    static const char *getKeyList()
    {
        return "key.localId,key.systemId";
    }

    static const char *getTypeName()
    {
        return "kernelModule::v_publicationInfo";
    }

    static std::string getDescriptor()
    {
        const char *elements[] = {
            "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\">",
"<Long/></Array></TypeDef><Enum name=\"DurabilityQosPolicyKind\"><Element name=\"VOLATILE_DURABILITY_QOS\" value=\"0\"/>",
"<Element name=\"TRANSIENT_LOCAL_DURABILITY_QOS\" value=\"1\"/><Element name=\"TRANSIENT_DURABILITY_QOS\" value=\"2\"/>",
"<Element name=\"PERSISTENT_DURABILITY_QOS\" value=\"3\"/></Enum><Struct name=\"Duration_t\"><Member name=\"sec\">",
"<Long/></Member><Member name=\"nanosec\"><ULong/></Member></Struct><Enum name=\"LivelinessQosPolicyKind\">",
"<Element name=\"AUTOMATIC_LIVELINESS_QOS\" value=\"0\"/><Element name=\"MANUAL_BY_PARTICIPANT_LIVELINESS_QOS\" value=\"1\"/>",
"<Element name=\"MANUAL_BY_TOPIC_LIVELINESS_QOS\" value=\"2\"/></Enum><Enum name=\"ReliabilityQosPolicyKind\">",
"<Element name=\"BEST_EFFORT_RELIABILITY_QOS\" value=\"0\"/><Element name=\"RELIABLE_RELIABILITY_QOS\" value=\"1\"/>",
"</Enum><Enum name=\"DestinationOrderQosPolicyKind\"><Element name=\"BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS\" value=\"0\"/>",
"<Element name=\"BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS\" value=\"1\"/></Enum><TypeDef name=\"octSeq\">",
"<Sequence><Octet/></Sequence></TypeDef><Enum name=\"OwnershipQosPolicyKind\"><Element name=\"SHARED_OWNERSHIP_QOS\" value=\"0\"/>",
"<Element name=\"EXCLUSIVE_OWNERSHIP_QOS\" value=\"1\"/></Enum><Struct name=\"OwnershipStrengthQosPolicy\">",
"<Member name=\"value\"><Long/></Member></Struct><Enum name=\"PresentationQosPolicyAccessScopeKind\"><Element name=\"INSTANCE_PRESENTATION_QOS\" value=\"0\"/>",
"<Element name=\"TOPIC_PRESENTATION_QOS\" value=\"1\"/><Element name=\"GROUP_PRESENTATION_QOS\" value=\"2\"/>",
"</Enum><TypeDef name=\"StringSeq\"><Sequence><String/></Sequence></TypeDef><Struct name=\"DurabilityQosPolicy\">",
"<Member name=\"kind\"><Type name=\"DurabilityQosPolicyKind\"/></Member></Struct><Struct name=\"LifespanQosPolicy\">",
"<Member name=\"duration\"><Type name=\"Duration_t\"/></Member></Struct><Struct name=\"LatencyBudgetQosPolicy\">",
"<Member name=\"duration\"><Type name=\"Duration_t\"/></Member></Struct><Struct name=\"DeadlineQosPolicy\">",
"<Member name=\"period\"><Type name=\"Duration_t\"/></Member></Struct><Struct name=\"LivelinessQosPolicy\">",
"<Member name=\"kind\"><Type name=\"LivelinessQosPolicyKind\"/></Member><Member name=\"lease_duration\">",
"<Type name=\"Duration_t\"/></Member></Struct><Struct name=\"ReliabilityQosPolicy\"><Member name=\"kind\">",
"<Type name=\"ReliabilityQosPolicyKind\"/></Member><Member name=\"max_blocking_time\"><Type name=\"Duration_t\"/>",
"</Member><Member name=\"synchronous\"><Boolean/></Member></Struct><Struct name=\"DestinationOrderQosPolicy\">",
"<Member name=\"kind\"><Type name=\"DestinationOrderQosPolicyKind\"/></Member></Struct><Struct name=\"GroupDataQosPolicy\">",
"<Member name=\"value\"><Type name=\"octSeq\"/></Member></Struct><Struct name=\"TopicDataQosPolicy\"><Member name=\"value\">",
"<Type name=\"octSeq\"/></Member></Struct><Struct name=\"UserDataQosPolicy\"><Member name=\"value\"><Type name=\"octSeq\"/>",
"</Member></Struct><Struct name=\"OwnershipQosPolicy\"><Member name=\"kind\"><Type name=\"OwnershipQosPolicyKind\"/>",
"</Member></Struct><Struct name=\"PresentationQosPolicy\"><Member name=\"access_scope\"><Type name=\"PresentationQosPolicyAccessScopeKind\"/>",
"</Member><Member name=\"coherent_access\"><Boolean/></Member><Member name=\"ordered_access\"><Boolean/>",
"</Member></Struct><Struct name=\"PartitionQosPolicy\"><Member name=\"name\"><Type name=\"StringSeq\"/>",
"</Member></Struct><Struct name=\"PublicationBuiltinTopicData\"><Member name=\"key\"><Type name=\"BuiltinTopicKey_t\"/>",
"</Member><Member name=\"participant_key\"><Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"topic_name\">",
"<String/></Member><Member name=\"type_name\"><String/></Member><Member name=\"durability\"><Type name=\"DurabilityQosPolicy\"/>",
"</Member><Member name=\"deadline\"><Type name=\"DeadlineQosPolicy\"/></Member><Member name=\"latency_budget\">",
"<Type name=\"LatencyBudgetQosPolicy\"/></Member><Member name=\"liveliness\"><Type name=\"LivelinessQosPolicy\"/>",
"</Member><Member name=\"reliability\"><Type name=\"ReliabilityQosPolicy\"/></Member><Member name=\"lifespan\">",
"<Type name=\"LifespanQosPolicy\"/></Member><Member name=\"destination_order\"><Type name=\"DestinationOrderQosPolicy\"/>",
"</Member><Member name=\"user_data\"><Type name=\"UserDataQosPolicy\"/></Member><Member name=\"ownership\">",
"<Type name=\"OwnershipQosPolicy\"/></Member><Member name=\"ownership_strength\"><Type name=\"OwnershipStrengthQosPolicy\"/>",
"</Member><Member name=\"presentation\"><Type name=\"PresentationQosPolicy\"/></Member><Member name=\"partition\">",
"<Type name=\"PartitionQosPolicy\"/></Member><Member name=\"topic_data\"><Type name=\"TopicDataQosPolicy\"/>",
"</Member><Member name=\"group_data\"><Type name=\"GroupDataQosPolicy\"/></Member></Struct></Module></MetaData>"
        };
        std::string descriptor;
        descriptor.reserve(4813);
        for (int i = 0; i < 42; i++) {
            descriptor.append(elements[i]);
        }

        return descriptor;
    }

    static copyInFunction getCopyIn()
    {
        return (copyInFunction) __PublicationBuiltinTopicData__copyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return (copyOutFunction) __PublicationBuiltinTopicData__copyOut;
    }
};
}}}

namespace dds { namespace topic {
template <>
struct topic_type_name<dds::topic::PublicationBuiltinTopicData>
{
    static std::string value()
    {
        return org::opensplice::topic::TopicTraits<dds::topic::PublicationBuiltinTopicData>::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(dds::topic::PublicationBuiltinTopicData);

namespace org { namespace opensplice { namespace topic {
template <>
class TopicTraits<dds::topic::SubscriptionBuiltinTopicData>
{
public:
    static const char *getKeyList()
    {
        return "key.localId,key.systemId";
    }

    static const char *getTypeName()
    {
        return "kernelModule::v_subscriptionInfo";
    }

    static std::string getDescriptor()
    {
        const char *elements[] = {
            "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\">",
"<Long/></Array></TypeDef><Enum name=\"DurabilityQosPolicyKind\"><Element name=\"VOLATILE_DURABILITY_QOS\" value=\"0\"/>",
"<Element name=\"TRANSIENT_LOCAL_DURABILITY_QOS\" value=\"1\"/><Element name=\"TRANSIENT_DURABILITY_QOS\" value=\"2\"/>",
"<Element name=\"PERSISTENT_DURABILITY_QOS\" value=\"3\"/></Enum><Struct name=\"Duration_t\"><Member name=\"sec\">",
"<Long/></Member><Member name=\"nanosec\"><ULong/></Member></Struct><Enum name=\"LivelinessQosPolicyKind\">",
"<Element name=\"AUTOMATIC_LIVELINESS_QOS\" value=\"0\"/><Element name=\"MANUAL_BY_PARTICIPANT_LIVELINESS_QOS\" value=\"1\"/>",
"<Element name=\"MANUAL_BY_TOPIC_LIVELINESS_QOS\" value=\"2\"/></Enum><Enum name=\"ReliabilityQosPolicyKind\">",
"<Element name=\"BEST_EFFORT_RELIABILITY_QOS\" value=\"0\"/><Element name=\"RELIABLE_RELIABILITY_QOS\" value=\"1\"/>",
"</Enum><Enum name=\"OwnershipQosPolicyKind\"><Element name=\"SHARED_OWNERSHIP_QOS\" value=\"0\"/><Element name=\"EXCLUSIVE_OWNERSHIP_QOS\" value=\"1\"/>",
"</Enum><Enum name=\"DestinationOrderQosPolicyKind\"><Element name=\"BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS\" value=\"0\"/>",
"<Element name=\"BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS\" value=\"1\"/></Enum><TypeDef name=\"octSeq\">",
"<Sequence><Octet/></Sequence></TypeDef><Enum name=\"PresentationQosPolicyAccessScopeKind\"><Element name=\"INSTANCE_PRESENTATION_QOS\" value=\"0\"/>",
"<Element name=\"TOPIC_PRESENTATION_QOS\" value=\"1\"/><Element name=\"GROUP_PRESENTATION_QOS\" value=\"2\"/>",
"</Enum><TypeDef name=\"StringSeq\"><Sequence><String/></Sequence></TypeDef><Struct name=\"DurabilityQosPolicy\">",
"<Member name=\"kind\"><Type name=\"DurabilityQosPolicyKind\"/></Member></Struct><Struct name=\"TimeBasedFilterQosPolicy\">",
"<Member name=\"minimum_separation\"><Type name=\"Duration_t\"/></Member></Struct><Struct name=\"LatencyBudgetQosPolicy\">",
"<Member name=\"duration\"><Type name=\"Duration_t\"/></Member></Struct><Struct name=\"DeadlineQosPolicy\">",
"<Member name=\"period\"><Type name=\"Duration_t\"/></Member></Struct><Struct name=\"LivelinessQosPolicy\">",
"<Member name=\"kind\"><Type name=\"LivelinessQosPolicyKind\"/></Member><Member name=\"lease_duration\">",
"<Type name=\"Duration_t\"/></Member></Struct><Struct name=\"ReliabilityQosPolicy\"><Member name=\"kind\">",
"<Type name=\"ReliabilityQosPolicyKind\"/></Member><Member name=\"max_blocking_time\"><Type name=\"Duration_t\"/>",
"</Member><Member name=\"synchronous\"><Boolean/></Member></Struct><Struct name=\"OwnershipQosPolicy\">",
"<Member name=\"kind\"><Type name=\"OwnershipQosPolicyKind\"/></Member></Struct><Struct name=\"DestinationOrderQosPolicy\">",
"<Member name=\"kind\"><Type name=\"DestinationOrderQosPolicyKind\"/></Member></Struct><Struct name=\"GroupDataQosPolicy\">",
"<Member name=\"value\"><Type name=\"octSeq\"/></Member></Struct><Struct name=\"TopicDataQosPolicy\"><Member name=\"value\">",
"<Type name=\"octSeq\"/></Member></Struct><Struct name=\"UserDataQosPolicy\"><Member name=\"value\"><Type name=\"octSeq\"/>",
"</Member></Struct><Struct name=\"PresentationQosPolicy\"><Member name=\"access_scope\"><Type name=\"PresentationQosPolicyAccessScopeKind\"/>",
"</Member><Member name=\"coherent_access\"><Boolean/></Member><Member name=\"ordered_access\"><Boolean/>",
"</Member></Struct><Struct name=\"PartitionQosPolicy\"><Member name=\"name\"><Type name=\"StringSeq\"/>",
"</Member></Struct><Struct name=\"SubscriptionBuiltinTopicData\"><Member name=\"key\"><Type name=\"BuiltinTopicKey_t\"/>",
"</Member><Member name=\"participant_key\"><Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"topic_name\">",
"<String/></Member><Member name=\"type_name\"><String/></Member><Member name=\"durability\"><Type name=\"DurabilityQosPolicy\"/>",
"</Member><Member name=\"deadline\"><Type name=\"DeadlineQosPolicy\"/></Member><Member name=\"latency_budget\">",
"<Type name=\"LatencyBudgetQosPolicy\"/></Member><Member name=\"liveliness\"><Type name=\"LivelinessQosPolicy\"/>",
"</Member><Member name=\"reliability\"><Type name=\"ReliabilityQosPolicy\"/></Member><Member name=\"ownership\">",
"<Type name=\"OwnershipQosPolicy\"/></Member><Member name=\"destination_order\"><Type name=\"DestinationOrderQosPolicy\"/>",
"</Member><Member name=\"user_data\"><Type name=\"UserDataQosPolicy\"/></Member><Member name=\"time_based_filter\">",
"<Type name=\"TimeBasedFilterQosPolicy\"/></Member><Member name=\"presentation\"><Type name=\"PresentationQosPolicy\"/>",
"</Member><Member name=\"partition\"><Type name=\"PartitionQosPolicy\"/></Member><Member name=\"topic_data\">",
"<Type name=\"TopicDataQosPolicy\"/></Member><Member name=\"group_data\"><Type name=\"GroupDataQosPolicy\"/>",
"</Member></Struct></Module></MetaData>"
        };
        std::string descriptor;
        descriptor.reserve(4667);
        for (int i = 0; i < 41; i++) {
            descriptor.append(elements[i]);
        }

        return descriptor;
    }

    static copyInFunction getCopyIn()
    {
        return (copyInFunction) __SubscriptionBuiltinTopicData__copyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return (copyOutFunction) __SubscriptionBuiltinTopicData__copyOut;
    }
};
}}}

namespace dds { namespace topic {
template <>
struct topic_type_name<dds::topic::SubscriptionBuiltinTopicData>
{
    static std::string value()
    {
        return org::opensplice::topic::TopicTraits<dds::topic::SubscriptionBuiltinTopicData>::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(dds::topic::SubscriptionBuiltinTopicData);

namespace org { namespace opensplice { namespace topic {
template <>
class TopicTraits<org::opensplice::topic::CMParticipantBuiltinTopicData>
{
public:
    static const char *getKeyList()
    {
        return "key.localId,key.systemId";
    }

    static const char *getTypeName()
    {
        return "kernelModule::v_participantCMInfo";
    }

    static std::string getDescriptor()
    {
        const char *elements[] = {
            "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\">",
"<Long/></Array></TypeDef><Struct name=\"ProductDataQosPolicy\"><Member name=\"value\"><String/></Member>",
"</Struct><Struct name=\"CMParticipantBuiltinTopicData\"><Member name=\"key\"><Type name=\"BuiltinTopicKey_t\"/>",
"</Member><Member name=\"product\"><Type name=\"ProductDataQosPolicy\"/></Member></Struct></Module></MetaData>"
        };
        std::string descriptor;
        descriptor.reserve(427);
        for (int i = 0; i < 4; i++) {
            descriptor.append(elements[i]);
        }

        return descriptor;
    }

    static copyInFunction getCopyIn()
    {
        return (copyInFunction) __CMParticipantBuiltinTopicData__copyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return (copyOutFunction) __CMParticipantBuiltinTopicData__copyOut;
    }
};
}}}

namespace dds { namespace topic {
template <>
struct topic_type_name<org::opensplice::topic::CMParticipantBuiltinTopicData>
{
    static std::string value()
    {
        return org::opensplice::topic::TopicTraits<org::opensplice::topic::CMParticipantBuiltinTopicData>::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(org::opensplice::topic::CMParticipantBuiltinTopicData);

namespace org { namespace opensplice { namespace topic {
template <>
class TopicTraits<org::opensplice::topic::CMPublisherBuiltinTopicData>
{
public:
    static const char *getKeyList()
    {
        return "key.localId,key.systemId";
    }

    static const char *getTypeName()
    {
        return "kernelModule::v_publisherCMInfo";
    }

    static std::string getDescriptor()
    {
        const char *elements[] = {
            "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\">",
"<Long/></Array></TypeDef><Struct name=\"ProductDataQosPolicy\"><Member name=\"value\"><String/></Member>",
"</Struct><Struct name=\"EntityFactoryQosPolicy\"><Member name=\"autoenable_created_entities\"><Boolean/>",
"</Member></Struct><TypeDef name=\"StringSeq\"><Sequence><String/></Sequence></TypeDef><Struct name=\"PartitionQosPolicy\">",
"<Member name=\"name\"><Type name=\"StringSeq\"/></Member></Struct><Struct name=\"CMPublisherBuiltinTopicData\">",
"<Member name=\"key\"><Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"product\"><Type name=\"ProductDataQosPolicy\"/>",
"</Member><Member name=\"participant_key\"><Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"name\">",
"<String/></Member><Member name=\"entity_factory\"><Type name=\"EntityFactoryQosPolicy\"/></Member><Member name=\"partition\">",
"<Type name=\"PartitionQosPolicy\"/></Member></Struct></Module></MetaData>"
        };
        std::string descriptor;
        descriptor.reserve(975);
        for (int i = 0; i < 9; i++) {
            descriptor.append(elements[i]);
        }

        return descriptor;
    }

    static copyInFunction getCopyIn()
    {
        return (copyInFunction) __CMPublisherBuiltinTopicData__copyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return (copyOutFunction) __CMPublisherBuiltinTopicData__copyOut;
    }
};
}}}

namespace dds { namespace topic {
template <>
struct topic_type_name<org::opensplice::topic::CMPublisherBuiltinTopicData>
{
    static std::string value()
    {
        return org::opensplice::topic::TopicTraits<org::opensplice::topic::CMPublisherBuiltinTopicData>::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(org::opensplice::topic::CMPublisherBuiltinTopicData);

namespace org { namespace opensplice { namespace topic {
template <>
class TopicTraits<org::opensplice::topic::CMSubscriberBuiltinTopicData>
{
public:
    static const char *getKeyList()
    {
        return "key.localId,key.systemId";
    }

    static const char *getTypeName()
    {
        return "kernelModule::v_subscriberCMInfo";
    }

    static std::string getDescriptor()
    {
        const char *elements[] = {
            "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\">",
"<Long/></Array></TypeDef><Struct name=\"ProductDataQosPolicy\"><Member name=\"value\"><String/></Member>",
"</Struct><Struct name=\"EntityFactoryQosPolicy\"><Member name=\"autoenable_created_entities\"><Boolean/>",
"</Member></Struct><Struct name=\"ShareQosPolicy\"><Member name=\"name\"><String/></Member><Member name=\"enable\">",
"<Boolean/></Member></Struct><TypeDef name=\"StringSeq\"><Sequence><String/></Sequence></TypeDef><Struct name=\"PartitionQosPolicy\">",
"<Member name=\"name\"><Type name=\"StringSeq\"/></Member></Struct><Struct name=\"CMSubscriberBuiltinTopicData\">",
"<Member name=\"key\"><Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"product\"><Type name=\"ProductDataQosPolicy\"/>",
"</Member><Member name=\"participant_key\"><Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"name\">",
"<String/></Member><Member name=\"entity_factory\"><Type name=\"EntityFactoryQosPolicy\"/></Member><Member name=\"share\">",
"<Type name=\"ShareQosPolicy\"/></Member><Member name=\"partition\"><Type name=\"PartitionQosPolicy\"/>",
"</Member></Struct></Module></MetaData>"
        };
        std::string descriptor;
        descriptor.reserve(1163);
        for (int i = 0; i < 11; i++) {
            descriptor.append(elements[i]);
        }

        return descriptor;
    }

    static copyInFunction getCopyIn()
    {
        return (copyInFunction) __CMSubscriberBuiltinTopicData__copyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return (copyOutFunction) __CMSubscriberBuiltinTopicData__copyOut;
    }
};
}}}

namespace dds { namespace topic {
template <>
struct topic_type_name<org::opensplice::topic::CMSubscriberBuiltinTopicData>
{
    static std::string value()
    {
        return org::opensplice::topic::TopicTraits<org::opensplice::topic::CMSubscriberBuiltinTopicData>::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(org::opensplice::topic::CMSubscriberBuiltinTopicData);

namespace org { namespace opensplice { namespace topic {
template <>
class TopicTraits<org::opensplice::topic::CMDataWriterBuiltinTopicData>
{
public:
    static const char *getKeyList()
    {
        return "key.localId,key.systemId";
    }

    static const char *getTypeName()
    {
        return "kernelModule::v_dataWriterCMInfo";
    }

    static std::string getDescriptor()
    {
        const char *elements[] = {
            "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\">",
"<Long/></Array></TypeDef><Struct name=\"ProductDataQosPolicy\"><Member name=\"value\"><String/></Member>",
"</Struct><Enum name=\"HistoryQosPolicyKind\"><Element name=\"KEEP_LAST_HISTORY_QOS\" value=\"0\"/><Element name=\"KEEP_ALL_HISTORY_QOS\" value=\"1\"/>",
"</Enum><Struct name=\"ResourceLimitsQosPolicy\"><Member name=\"max_samples\"><Long/></Member><Member name=\"max_instances\">",
"<Long/></Member><Member name=\"max_samples_per_instance\"><Long/></Member></Struct><Struct name=\"Duration_t\">",
"<Member name=\"sec\"><Long/></Member><Member name=\"nanosec\"><ULong/></Member></Struct><Struct name=\"HistoryQosPolicy\">",
"<Member name=\"kind\"><Type name=\"HistoryQosPolicyKind\"/></Member><Member name=\"depth\"><Long/></Member>",
"</Struct><Struct name=\"WriterDataLifecycleQosPolicy\"><Member name=\"autodispose_unregistered_instances\">",
"<Boolean/></Member><Member name=\"autopurge_suspended_samples_delay\"><Type name=\"Duration_t\"/></Member>",
"<Member name=\"autounregister_instance_delay\"><Type name=\"Duration_t\"/></Member></Struct><Struct name=\"CMDataWriterBuiltinTopicData\">",
"<Member name=\"key\"><Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"product\"><Type name=\"ProductDataQosPolicy\"/>",
"</Member><Member name=\"publisher_key\"><Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"name\">",
"<String/></Member><Member name=\"history\"><Type name=\"HistoryQosPolicy\"/></Member><Member name=\"resource_limits\">",
"<Type name=\"ResourceLimitsQosPolicy\"/></Member><Member name=\"writer_data_lifecycle\"><Type name=\"WriterDataLifecycleQosPolicy\"/>",
"</Member></Struct></Module></MetaData>"
        };
        std::string descriptor;
        descriptor.reserve(1692);
        for (int i = 0; i < 15; i++) {
            descriptor.append(elements[i]);
        }

        return descriptor;
    }

    static copyInFunction getCopyIn()
    {
        return (copyInFunction) __CMDataWriterBuiltinTopicData__copyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return (copyOutFunction) __CMDataWriterBuiltinTopicData__copyOut;
    }
};
}}}

namespace dds { namespace topic {
template <>
struct topic_type_name<org::opensplice::topic::CMDataWriterBuiltinTopicData>
{
    static std::string value()
    {
        return org::opensplice::topic::TopicTraits<org::opensplice::topic::CMDataWriterBuiltinTopicData>::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(org::opensplice::topic::CMDataWriterBuiltinTopicData);

namespace org { namespace opensplice { namespace topic {
template <>
class TopicTraits<org::opensplice::topic::CMDataReaderBuiltinTopicData>
{
public:
    static const char *getKeyList()
    {
        return "key.localId,key.systemId";
    }

    static const char *getTypeName()
    {
        return "kernelModule::v_dataReaderCMInfo";
    }

    static std::string getDescriptor()
    {
        const char *elements[] = {
            "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\">",
"<Long/></Array></TypeDef><Struct name=\"ProductDataQosPolicy\"><Member name=\"value\"><String/></Member>",
"</Struct><Enum name=\"HistoryQosPolicyKind\"><Element name=\"KEEP_LAST_HISTORY_QOS\" value=\"0\"/><Element name=\"KEEP_ALL_HISTORY_QOS\" value=\"1\"/>",
"</Enum><Struct name=\"ResourceLimitsQosPolicy\"><Member name=\"max_samples\"><Long/></Member><Member name=\"max_instances\">",
"<Long/></Member><Member name=\"max_samples_per_instance\"><Long/></Member></Struct><Struct name=\"Duration_t\">",
"<Member name=\"sec\"><Long/></Member><Member name=\"nanosec\"><ULong/></Member></Struct><Enum name=\"InvalidSampleVisibilityQosPolicyKind\">",
"<Element name=\"NO_INVALID_SAMPLES\" value=\"0\"/><Element name=\"MINIMUM_INVALID_SAMPLES\" value=\"1\"/>",
"<Element name=\"ALL_INVALID_SAMPLES\" value=\"2\"/></Enum><Struct name=\"UserKeyQosPolicy\"><Member name=\"enable\">",
"<Boolean/></Member><Member name=\"expression\"><String/></Member></Struct><Struct name=\"ShareQosPolicy\">",
"<Member name=\"name\"><String/></Member><Member name=\"enable\"><Boolean/></Member></Struct><Struct name=\"HistoryQosPolicy\">",
"<Member name=\"kind\"><Type name=\"HistoryQosPolicyKind\"/></Member><Member name=\"depth\"><Long/></Member>",
"</Struct><Struct name=\"ReaderLifespanQosPolicy\"><Member name=\"use_lifespan\"><Boolean/></Member><Member name=\"duration\">",
"<Type name=\"Duration_t\"/></Member></Struct><Struct name=\"InvalidSampleVisibilityQosPolicy\"><Member name=\"kind\">",
"<Type name=\"InvalidSampleVisibilityQosPolicyKind\"/></Member></Struct><Struct name=\"ReaderDataLifecycleQosPolicy\">",
"<Member name=\"autopurge_nowriter_samples_delay\"><Type name=\"Duration_t\"/></Member><Member name=\"autopurge_disposed_samples_delay\">",
"<Type name=\"Duration_t\"/></Member><Member name=\"autopurge_dispose_all\"><Boolean/></Member><Member name=\"enable_invalid_samples\">",
"<Boolean/></Member><Member name=\"invalid_sample_visibility\"><Type name=\"InvalidSampleVisibilityQosPolicy\"/>",
"</Member></Struct><Struct name=\"CMDataReaderBuiltinTopicData\"><Member name=\"key\"><Type name=\"BuiltinTopicKey_t\"/>",
"</Member><Member name=\"product\"><Type name=\"ProductDataQosPolicy\"/></Member><Member name=\"subscriber_key\">",
"<Type name=\"BuiltinTopicKey_t\"/></Member><Member name=\"name\"><String/></Member><Member name=\"history\">",
"<Type name=\"HistoryQosPolicy\"/></Member><Member name=\"resource_limits\"><Type name=\"ResourceLimitsQosPolicy\"/>",
"</Member><Member name=\"reader_data_lifecycle\"><Type name=\"ReaderDataLifecycleQosPolicy\"/></Member>",
"<Member name=\"subscription_keys\"><Type name=\"UserKeyQosPolicy\"/></Member><Member name=\"reader_lifespan\">",
"<Type name=\"ReaderLifespanQosPolicy\"/></Member><Member name=\"share\"><Type name=\"ShareQosPolicy\"/>",
"</Member></Struct></Module></MetaData>"
        };
        std::string descriptor;
        descriptor.reserve(2839);
        for (int i = 0; i < 25; i++) {
            descriptor.append(elements[i]);
        }

        return descriptor;
    }

    static copyInFunction getCopyIn()
    {
        return (copyInFunction) __CMDataReaderBuiltinTopicData__copyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return (copyOutFunction) __CMDataReaderBuiltinTopicData__copyOut;
    }
};
}}}

namespace dds { namespace topic {
template <>
struct topic_type_name<org::opensplice::topic::CMDataReaderBuiltinTopicData>
{
    static std::string value()
    {
        return org::opensplice::topic::TopicTraits<org::opensplice::topic::CMDataReaderBuiltinTopicData>::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(org::opensplice::topic::CMDataReaderBuiltinTopicData);

namespace org { namespace opensplice { namespace topic {
template <>
class TopicTraits<org::opensplice::topic::TypeBuiltinTopicData>
{
public:
    static const char *getKeyList()
    {
        return "name,data_representation_id,type_hash.msb,type_hash.lsb";
    }

    static const char *getTypeName()
    {
        return "kernelModule::v_typeInfo";
    }

    static std::string getDescriptor()
    {
        const char *elements[] = {
            "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"DataRepresentationId_t\"><Short/></TypeDef>",
"<Struct name=\"TypeHash\"><Member name=\"msb\"><ULongLong/></Member><Member name=\"lsb\"><ULongLong/>",
"</Member></Struct><TypeDef name=\"octSeq\"><Sequence><Octet/></Sequence></TypeDef><Struct name=\"TypeBuiltinTopicData\">",
"<Member name=\"name\"><String/></Member><Member name=\"data_representation_id\"><Type name=\"DataRepresentationId_t\"/>",
"</Member><Member name=\"type_hash\"><Type name=\"TypeHash\"/></Member><Member name=\"meta_data\"><Type name=\"octSeq\"/>",
"</Member><Member name=\"extentions\"><Type name=\"octSeq\"/></Member></Struct></Module></MetaData>"
        };
        std::string descriptor;
        descriptor.reserve(666);
        for (int i = 0; i < 6; i++) {
            descriptor.append(elements[i]);
        }

        return descriptor;
    }

    static copyInFunction getCopyIn()
    {
        return (copyInFunction) __TypeBuiltinTopicData__copyIn;
    }

    static copyOutFunction getCopyOut()
    {
        return (copyOutFunction) __TypeBuiltinTopicData__copyOut;
    }
};
}}}

namespace dds { namespace topic {
template <>
struct topic_type_name<org::opensplice::topic::TypeBuiltinTopicData>
{
    static std::string value()
    {
        return org::opensplice::topic::TopicTraits<org::opensplice::topic::TypeBuiltinTopicData>::getTypeName();
    }
};
}}

REGISTER_TOPIC_TYPE(org::opensplice::topic::TypeBuiltinTopicData);

#endif /* ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_TRAITS_HPP */
