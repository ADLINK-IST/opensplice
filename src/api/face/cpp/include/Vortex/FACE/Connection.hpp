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
#ifndef VORTEX_FACE_CONNECTION_HPP_
#define VORTEX_FACE_CONNECTION_HPP_

#include "Vortex_FACE.hpp"
#include "Vortex/FACE/FaceInstance.hpp"
#include "Vortex/FACE/AnyConnection.hpp"
#include "Vortex/FACE/ReportSupport.hpp"
#include "Vortex/FACE/DataListener.hpp"

namespace Vortex {
namespace FACE {

template <typename TYPE>
class Connection : public AnyConnection
{
public:
    typedef typename Vortex::FACE::smart_ptr_traits< Connection<TYPE> >::shared_ptr shared_ptr;

    Connection() :
        topic(dds::core::null),
        reader(dds::core::null),
        writer(dds::core::null),
        waitset(dds::core::null),
        participant(dds::core::null),
        listener(NULL)
    {
    }

    virtual ~Connection()
    {
        /* Be sure to remove the listener before the reader is destroyed.
         * Otherwise it could cause big problems (like core dumps). */
        if (this->listener != NULL) {
            (void)this->unregisterCallback();
        }
    }

    /* Get the Connection<TYPE>, related to the given connectionId. */
    static typename Connection<TYPE>::shared_ptr
    get(const ::FACE::CONNECTION_ID_TYPE &connectionId,
        ::FACE::RETURN_CODE_TYPE &status)
    {
        typename Connection<TYPE>::shared_ptr connection;
        FaceInstance::shared_ptr instance = FaceInstance::getInstance();
        if (instance.get() != NULL) {
            AnyConnection::shared_ptr any = instance->getConnection(connectionId);
            if (any.get() != NULL) {
                connection = OSPL_CXX11_STD_MODULE::dynamic_pointer_cast< Connection<TYPE> >(any);
                if (connection.get() != NULL) {
                    status = ::FACE::NO_ERROR;
                } else {
                    status = ::FACE::INVALID_CONFIG;
                    FACE_REPORT_ERROR(status, "Failed to get connection '%d' for type <%s>", (int)connectionId, Connection::typeName().c_str());
                }
            } else {
                status = ::FACE::INVALID_PARAM;
                FACE_REPORT_ERROR(status, "Failed to find connection '%d'", (int)connectionId);
            }
        } else {
            status = ::FACE::INVALID_CONFIG;
            FACE_REPORT_ERROR(status, "Instance not initialized");
        }
        return connection;
    }

    /* Send the message (AKA write the sample). */
    ::FACE::RETURN_CODE_TYPE
    send(const TYPE &message,
         const ::FACE::TIMEOUT_TYPE &timeout)
    {
        ::FACE::RETURN_CODE_TYPE status = ::FACE::INVALID_MODE;
        if (this->writer != dds::core::null) {
            try {
                dds::pub::qos::DataWriterQos qos = this->writer.qos();
                if ((qos.policy<dds::core::policy::Reliability>().kind() == dds::core::policy::ReliabilityKind::RELIABLE) &&
                    (this->copyIn(timeout) > qos.policy<dds::core::policy::Reliability>().max_blocking_time())) {
                    status = ::FACE::INVALID_PARAM;
                } else {
                    this->writer.write(message);
                    AnyConnection::setLastValidity(::FACE::VALID);
                    status = ::FACE::NO_ERROR;
                }
            } catch (const dds::core::Exception& e) {
                AnyConnection::setLastValidity(::FACE::INVALID);
                status = Vortex::FACE::exceptionToReturnCode(e);
            } catch (...) {
                AnyConnection::setLastValidity(::FACE::INVALID);
                status = ::FACE::NO_ACTION;
                assert(false);
            }
        } else {
            FACE_REPORT_ERROR(status, "Write not allowed for '%s'<%s>", AnyConnection::getName().c_str(), Connection::typeName().c_str());
        }
        return status;
    }

    /* Receive the message (AKA take the sample). */
    ::FACE::RETURN_CODE_TYPE
    receive(TYPE &message,
            const ::FACE::TIMEOUT_TYPE &timeout)
    {
        ::FACE::RETURN_CODE_TYPE status = ::FACE::INVALID_MODE;
        if (this->reader != dds::core::null) {
            if (timeout >= 0 || timeout == ::FACE::INF_TIME_VALUE) {
                try {
                    status = ::FACE::NO_ACTION;
                    if (timeout > 0) {
                        this->waitset.wait(this->copyIn(timeout));
                    }
                    dds::sub::LoanedSamples<TYPE> samples = this->reader.select().max_samples(1).take();
                    if (samples.length() == 1) {
                        const dds::sub::SampleInfo& info = samples.begin()->info();
                        if (info.valid()) {
                            message = samples.begin()->data();
                            AnyConnection::setLastValidity(::FACE::VALID);
                            status = ::FACE::NO_ERROR;
                        } else {
                            FACE_REPORT_ERROR(status, "Invalid data received for '%s'<%s>",
                                    AnyConnection::getName().c_str(), Connection::typeName().c_str());
                        }
                    }
                } catch (const dds::core::Exception& e) {
                    AnyConnection::setLastValidity(::FACE::INVALID);
                    status = Vortex::FACE::exceptionToReturnCode(e);
                } catch (...) {
                    AnyConnection::setLastValidity(::FACE::INVALID);
                    status = ::FACE::NO_ACTION;
                    assert(false);
                }
            } else {
                status = ::FACE::INVALID_PARAM;
                FACE_REPORT_ERROR(status, "Invalid timeout '%" PA_PRId64 "' for '%s'<%s>",
                        timeout, AnyConnection::getName().c_str(), Connection::typeName().c_str());
            }
        } else {
            FACE_REPORT_ERROR(status, "Read not allowed for '%s'<%s>", AnyConnection::getName().c_str(), Connection::typeName().c_str());
        }
        return status;
    }

    /* Register the data callback (AKA set reader listener). */
    ::FACE::RETURN_CODE_TYPE
    registerCallback(typename ::FACE::Read_Callback<TYPE>::send_event cb,
                     const ::FACE::WAITSET_TYPE &mask)
    {
        ::FACE::RETURN_CODE_TYPE status = ::FACE::INVALID_MODE;
        if (this->reader != dds::core::null) {
            try {
                if (this->listener == NULL) {
                    /* Register a new listener at the dds reader, to be triggered when data is available. */
                    this->listener = new Vortex::FACE::DataListener<TYPE>(Connection::typeName(), AnyConnection::getName(), cb);
                    this->reader.listener(this->listener, dds::core::status::StatusMask::data_available());
                    AnyConnection::setLastValidity(::FACE::VALID);
                    status = ::FACE::NO_ERROR;
                } else {
                    FACE_REPORT_ERROR(status, "Cannot register multiple callbacks for connection '%s'<%s>, unregister the existing callback first", AnyConnection::getName().c_str(), Connection::typeName().c_str());
                    status = ::FACE::NO_ACTION;
                }
            } catch (const dds::core::Exception& e) {
                AnyConnection::setLastValidity(::FACE::INVALID);
                status = Vortex::FACE::exceptionToReturnCode(e);
            } catch (...) {
                AnyConnection::setLastValidity(::FACE::INVALID);
                status = ::FACE::NO_ACTION;
                assert(false);
            }
        } else {
            FACE_REPORT_ERROR(status, "No reader for connection '%s'<%s>, unable to register callback", AnyConnection::getName().c_str(), Connection::typeName().c_str());
        }
        return status;
    }

    /* Un-register the data callback (AKA reset reader listener). */
    virtual ::FACE::RETURN_CODE_TYPE
    unregisterCallback()
    {
        ::FACE::RETURN_CODE_TYPE status = ::FACE::INVALID_MODE;
        if (this->reader != dds::core::null) {
            try {
                if (this->listener != NULL) {
                    /* Reset the listener from the reader and destroy it. */
                    this->reader.listener(NULL, dds::core::status::StatusMask::none());
                    delete this->listener;
                    this->listener = NULL;
                    AnyConnection::setLastValidity(::FACE::VALID);
                    status = ::FACE::NO_ERROR;
                } else {
                    FACE_REPORT_ERROR(status, "Cannot unregister non-existent callback for connection '%s'<%s>", AnyConnection::getName().c_str(), Connection::typeName().c_str());
                    status = ::FACE::NO_ACTION;
                }
            } catch (const dds::core::Exception& e) {
                AnyConnection::setLastValidity(::FACE::INVALID);
                status = Vortex::FACE::exceptionToReturnCode(e);
            } catch (...) {
                AnyConnection::setLastValidity(::FACE::INVALID);
                status = ::FACE::NO_ACTION;
                assert(false);
            }
        } else {
            FACE_REPORT_ERROR(status, "No reader for connection '%s'<%s>, unable to unregister callback", AnyConnection::getName().c_str(), Connection::typeName().c_str());
        }
        return status;
    }

    virtual int32_t getDomainId() const {
        if (this->participant != dds::core::null) {
            return this->participant.domain_id();
        }
    }

protected:
    virtual void
    initWriter()
    {
        this->initParticipant();
        this->initTopic();
        dds::pub::Publisher publisher(this->participant, this->config->getPublisherQos());
        this->writer = dds::pub::DataWriter<TYPE>(publisher,
                                                  this->topic,
                                                  this->config->getWriterQos());
    }

    virtual void
    initReader()
    {
        this->initParticipant();
        this->initTopic();
        dds::sub::Subscriber subscriber(this->participant, this->config->getSubscriberQos());
        this->reader = dds::sub::DataReader<TYPE>(subscriber,
                                                  this->topic,
                                                  this->config->getReaderQos());
        dds::core::cond::StatusCondition condition(this->reader);
        condition.enabled_statuses(dds::core::status::StatusMask::data_available());
        this->waitset = dds::core::cond::WaitSet();
        this->waitset.attach_condition(condition);
    }

private:
    void
    initTopic()
    {
        if (this->topic == dds::core::null) {
            this->topic = dds::topic::Topic<TYPE>(this->participant,
                                                  this->config->getTopicName(),
                                                  this->config->getTopicQos());
        }
    }

    void
    initParticipant()
    {
        if (this->participant == dds::core::null) {
            this->participant = dds::domain::DomainParticipant(this->config->getDomainId(),
                                                               this->config->getParticipantQos());
        }
    }

    static std::string
    typeName()
    {
        return dds::topic::topic_type_name<TYPE>::value();
    }

    dds::topic::Topic<TYPE>    topic;
    dds::sub::DataReader<TYPE> reader;
    dds::pub::DataWriter<TYPE> writer;
    dds::core::cond::WaitSet waitset;
    dds::domain::DomainParticipant participant;
    typename Vortex::FACE::DataListener<TYPE> *listener;
};

}; /* namespace FACE */
}; /* namespace Vortex */


#endif /* VORTEX_FACE_CONNECTION_HPP_ */
