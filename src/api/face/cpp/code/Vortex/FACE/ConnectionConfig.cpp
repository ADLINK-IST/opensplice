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
#include <sstream>

#include "dds/dds.hpp"

#include "Vortex_FACE.hpp"

#include "Vortex/FACE/Config.hpp"
#include "Vortex/FACE/ConnectionConfig.hpp"
#include "Vortex/FACE/ConnectionFactory.hpp"
#include "Vortex/FACE/ReportSupport.hpp"


Vortex::FACE::ConnectionConfig::ConnectionConfig() :
    valid(false)
{
}

::FACE::RETURN_CODE_TYPE
Vortex::FACE::ConnectionConfig::set(const std::string &tag, const std::string &value)
{
    ::FACE::RETURN_CODE_TYPE status = ::FACE::INVALID_CONFIG;

    if (tag.length() > this->TAG_PREFIX_LEN) {
        /* Ignore the tag prefix. */
        const char* c_tag = tag.c_str() + this->TAG_PREFIX_LEN;

        status = ::FACE::NO_ERROR;

             if (strcmp(c_tag, "name")                          == 0) { this->connectionName   = value; }
        else if (strcmp(c_tag, "type_name")                     == 0) { this->typeName         = value; }
        else if (strcmp(c_tag, "topic_name")                    == 0) { this->topicName        = value; }
        else if (strcmp(c_tag, "type")                          == 0) { this->type             = value; }
        else if (strcmp(c_tag, "platform_view_guid")            == 0) { this->guid             = value; }
        else if (strcmp(c_tag, "refresh_period")                == 0) { this->refresh          = value; }
        else if (strcmp(c_tag, "domain_id")                     == 0) { this->domainId         = value; }
        else if (strcmp(c_tag, "direction")                     == 0) { this->direction        = value; }
        else if (strcmp(c_tag, "qos::uri")                      == 0) { this->qosUri           = value; }
        else if (strcmp(c_tag, "qos::profile")                  == 0) { this->qosProfile       = value; }
        else if (strcmp(c_tag, "qos::domainparticipant_qos_id") == 0) { this->qosParticipantId = value; }
        else if (strcmp(c_tag, "qos::topic_qos_id")             == 0) { this->qosTopicId       = value; }
        else if (strcmp(c_tag, "qos::publisher_qos_id")         == 0) { this->qosPublisherId   = value; }
        else if (strcmp(c_tag, "qos::datawriter_qos_id")        == 0) { this->qosWriterId      = value; }
        else if (strcmp(c_tag, "qos::subscriber_qos_id")        == 0) { this->qosSubscriberId  = value; }
        else if (strcmp(c_tag, "qos::datareader_qos_id")        == 0) { this->qosReaderId      = value; }
        else {
            status = ::FACE::INVALID_CONFIG;
        }
    }

    return status;
}

::FACE::RETURN_CODE_TYPE
Vortex::FACE::ConnectionConfig::validate()
{
    ::FACE::RETURN_CODE_TYPE status = ::FACE::NO_ERROR;
    bool ok;
    std::string name("unknown");
    std::ostringstream problem;
    if (this->connectionName.empty()) {
        problem << std::endl << "              - Mandatory <name> tag not available.";
    } else {
        name = this->connectionName;
    }
    if (this->typeName.empty()) {
        problem << std::endl << "              - Mandatory <type_name> tag not available.";
    } else {
        if (!(Vortex::FACE::ConnectionFactory::knows(this->typeName))) {
            problem << std::endl << "              - Data type <type_name> \"" << this->typeName << "\" is unknown."
                    << std::endl << "                 This can be caused by two issues:"
                    << std::endl << "                   1) The type_name does not equal the data type class (with scope considered)."
                    << std::endl << "                   2) No FACE::TS::Send_Message or FACE::TS::Receive_Message is used yet."
                    << std::endl << "                      For certain compilers, it is needed that these typed functions are"
                    << std::endl << "                      used before the compiler includes the static variable that registers"
                    << std::endl << "                      the data type at the FACE middleware.";
        }
    }
    if (this->topicName.empty()) {
        problem << std::endl << "              - Mandatory <topic_name> tag not available.";
    }
    if (this->type.empty()) {
        problem << std::endl << "              - Mandatory <type> tag not available.";
    } else {
        if (strcmp(this->type.c_str(), "DDS") != 0) {
            problem << std::endl << "              - Connection <type> \"" << this->type << "\" not supported.";
        }
    }
    if (this->direction.empty()) {
        problem << std::endl << "              - Mandatory <direction> tag not available.";
    } else {
        if (this->getDirection() == ::FACE::NOT_DEFINED_CONNECTION_DIRECTION_TYPE) {
            problem << std::endl << "              - Connection <direction> \"" << this->direction << "\" not supported.";
        }
    }
    if (!this->guid.empty()) {
        ok = true;
        this->strtoll(this->guid, ok);
        if (!ok) {
            problem << std::endl << "              - Connection <platform_view_guid> \"" << this->guid << "\" not a number.";
        }
    }
    if (!this->refresh.empty()) {
        ok = true;
        this->strtoll(this->refresh, ok);
        if (!ok) {
            problem << std::endl << "              - Connection <refresh_period> \"" << this->refresh << "\" not a number.";
        }
    }
    if (!this->domainId.empty()) {
        ok = true;
        this->strtoll(this->domainId, ok);
        if (!ok) {
            problem << std::endl << "              - Connection <domain_id> \"" << this->domainId << "\" not a number.";
        }
    }

    if (!(problem.str().empty())) {
        status = ::FACE::INVALID_CONFIG;
        FACE_REPORT_ERROR(status, "Connection \"%s\" has invalid configuration: %s", name.c_str(), problem.str().c_str());
    } else {
        /* Validate the QoSses separately because the QosProvider can throw
         * an exception, which will mess up the tracing. */
        if ((this->qosUri.empty()           == false) ||
            (this->qosProfile.empty()       == false) ||
            (this->qosParticipantId.empty() == false) ||
            (this->qosTopicId.empty()       == false) ||
            (this->qosPublisherId.empty()   == false) ||
            (this->qosWriterId.empty()      == false) ||
            (this->qosSubscriberId.empty()  == false) ||
            (this->qosReaderId.empty()      == false) )
        {
            if (this->qosUri.empty()) {
                status = ::FACE::INVALID_CONFIG;
                problem << std::endl << "              - If a <qos> element is supplied, then the <qos><uri> element is mandatory";
                FACE_REPORT_ERROR(status, "Connection \"%s\" has invalid configuration: %s", name.c_str(), problem.str().c_str());
            } else {
                try {
                    dds::core::QosProvider qp(Vortex::FACE::Config::addUrlPrefix(this->qosUri), this->qosProfile);
                    if (this->qosParticipantId.empty()) {
                        this->participantQos = qp.participant_qos();
                    } else {
                        this->participantQos = qp.participant_qos(this->qosParticipantId);
                    }
                    if (this->qosSubscriberId.empty()) {
                        this->subscriberQos = qp.subscriber_qos();
                    } else {
                        this->subscriberQos = qp.subscriber_qos(this->qosSubscriberId);
                    }
                    if (this->qosPublisherId.empty()) {
                        this->publisherQos = qp.publisher_qos();
                    } else {
                        this->publisherQos = qp.publisher_qos(this->qosPublisherId);
                    }
                    if (this->qosTopicId.empty()) {
                        this->topicQos = qp.topic_qos();
                    } else {
                        this->topicQos = qp.topic_qos(this->qosTopicId);
                    }
                    if (this->qosWriterId.empty()) {
                        this->writerQos = qp.datawriter_qos();
                    } else {
                        this->writerQos = qp.datawriter_qos(this->qosWriterId);
                    }
                    if (this->qosReaderId.empty()) {
                        this->readerQos = qp.datareader_qos();
                    } else {
                        this->readerQos = qp.datareader_qos(this->qosReaderId);
                    }
                } catch (const dds::core::Exception& e) {
                    status = ::FACE::INVALID_CONFIG;
                } catch (...) {
                    status = ::FACE::INVALID_CONFIG;
                    assert(false);
                }
            }
        }
    }

    this->valid = (status == ::FACE::NO_ERROR);

    return status;
}

std::string
Vortex::FACE::ConnectionConfig::getConnectionName()
{
    assert(this->valid);
    return this->connectionName;
}

std::string
Vortex::FACE::ConnectionConfig::getTopicName()
{
    assert(this->valid);
    return this->topicName;
}

std::string
Vortex::FACE::ConnectionConfig::getTypeName()
{
    assert(this->valid);
    return this->typeName;
}


::FACE::CONNECTION_DIRECTION_TYPE
Vortex::FACE::ConnectionConfig::getDirection()
{
    ::FACE::CONNECTION_DIRECTION_TYPE retval = ::FACE::NOT_DEFINED_CONNECTION_DIRECTION_TYPE;

    if      (strcmp(this->direction.c_str(), "SOURCE")         == 0) { retval = ::FACE::SOURCE;         }
    else if (strcmp(this->direction.c_str(), "DESTINATION")    == 0) { retval = ::FACE::DESTINATION;    }
    else if (strcmp(this->direction.c_str(), "BI_DIRECTIONAL") == 0) { retval = ::FACE::BI_DIRECTIONAL; }

    return retval;
}

::FACE::MESSAGE_TYPE_GUID
Vortex::FACE::ConnectionConfig::getGuid()
{
    assert(this->valid);
    if (this->guid.empty()) {
        return 0;
    } else {
        bool dummy;
        return (::FACE::MESSAGE_TYPE_GUID)this->strtoll(this->guid, dummy);
    }
}

::FACE::SYSTEM_TIME_TYPE
Vortex::FACE::ConnectionConfig::getRefreshPeriod()
{
    assert(this->valid);
    if (this->refresh.empty()) {
        return 0;
    } else {
        bool dummy;
        return (::FACE::SYSTEM_TIME_TYPE)this->strtoll(this->refresh, dummy);
    }
}

uint32_t
Vortex::FACE::ConnectionConfig::getDomainId()
{
    assert(this->valid);
    if (this->domainId.empty()) {
        return org::opensplice::domain::default_id();
    } else {
        bool dummy;
        return (uint32_t)this->strtoll(this->domainId, dummy);
    }
}

dds::domain::qos::DomainParticipantQos
Vortex::FACE::ConnectionConfig::getParticipantQos()
{
    assert(this->valid);
    /* Will be default when not set. */
    return this->participantQos;
}

dds::pub::qos::PublisherQos
Vortex::FACE::ConnectionConfig::getPublisherQos()
{
    assert(this->valid);
    /* The QoS will be default when not set. */

    /* The connection name should be the partition when to
     * QoS does not have one yet. */
    dds::core::policy::Partition partitionPolicy = this->publisherQos.policy<dds::core::policy::Partition>();
    dds::core::StringSeq         partitionNames  = partitionPolicy.name();
    if (partitionNames.empty()) {
        this->publisherQos << dds::core::policy::Partition(this->connectionName);
    }

    return this->publisherQos;
}

dds::sub::qos::SubscriberQos
Vortex::FACE::ConnectionConfig::getSubscriberQos()
{
    /* Remember to insert this->connectionName as partition
     * if the QoS doesn't have a partition already. */
    assert(this->valid);
    /* The QoS will be default when not set. */

    /* The connection name should be the partition when to
     * QoS does not have one yet. */
    dds::core::policy::Partition partitionPolicy = this->subscriberQos.policy<dds::core::policy::Partition>();
    dds::core::StringSeq         partitionNames  = partitionPolicy.name();
    if (partitionNames.empty()) {
        this->subscriberQos << dds::core::policy::Partition(this->connectionName);
    }

    return this->subscriberQos;
}

dds::topic::qos::TopicQos
Vortex::FACE::ConnectionConfig::getTopicQos()
{
    assert(this->valid);
    /* Will be default when not set. */
    return this->topicQos;
}

dds::pub::qos::DataWriterQos
Vortex::FACE::ConnectionConfig::getWriterQos()
{
    assert(this->valid);
    /* Will be default when not set. */
    return this->writerQos;
}

dds::sub::qos::DataReaderQos
Vortex::FACE::ConnectionConfig::getReaderQos()
{
    assert(this->valid);
    /* Will be default when not set. */
    return this->readerQos;
}

long long
Vortex::FACE::ConnectionConfig::strtoll(const std::string &value, bool &ok)
{
    long long retval = 0;
    const c_char *c_val = value.c_str();
    c_char *c_end;

    ok = true;

    retval = os_strtoll(c_val, &c_end, 0);
    if ((c_end == c_val) || (*c_end != '\0')) {
        /* TODO: Call this at validation and return error. */
        //this->valid = false;
        ok = false;
    }

    return retval;
}

const size_t Vortex::FACE::ConnectionConfig::TAG_PREFIX_LEN = strlen("::connections_list::connection::");

