#ifndef _FACE_TS_COMMON_H_
#define _FACE_TS_COMMON_H_

#include <dds/core/ddscore.hpp>
#include "FACE_common.h"

namespace FACE
{
    typedef std::string CONNECTION_NAME_TYPE;

    typedef int32_t MESSAGE_SIZE_TYPE;

    typedef int64_t MESSAGE_TYPE_GUID;

    enum CONNECTION_DIRECTION_TYPE {
        SOURCE,
        DESTINATION,
        BI_DIRECTIONAL,
        ONE_WAY_REQUEST_SOURCE,
        ONE_WAY_REQUEST_DESTINATION,
        TWO_WAY_REQUEST_SYNCHRONOUS_SOURCE,
        TWO_WAY_REQUEST_SYNCHRONOUS_DESTINATION,
        TWO_WAY_REQUEST_REPLY_ASYNCHRONOUS_SOURCE,
        TWO_WAY_REQUEST_REPLY_ASYNCHRONOUS_DESTINATION,
        NOT_DEFINED_CONNECTION_DIRECTION_TYPE
    };

    typedef int64_t CONNECTION_ID_TYPE;

    typedef int64_t TRANSACTION_ID_TYPE;

    enum CONNECTION_TYPE {
        SAMPLING_PORT,
        QUEUING_PORT,
        SOCKET,
        MQ,
        SHM,
        CORBA,
        DDS
    };

    typedef dds::core::array< bool, 32 > WAITSET_TYPE;

    enum QUEUING_DISCIPLINE_TYPE {
        FIFO,
        PRIORITY,
        NOT_DEFINED_QUEUING_DISCIPLINE_TYPE
    };

    enum CONNECTION_DOMAIN_TYPE {
        UNIX,
        INET,
        NOT_DEFINED_CONNECTION_DOMAIN_TYPE
    };

    enum SOCKET_TYPE {
        STREAM,
        DGRAM,
        SEQPACKET,
        NOT_DEFINED_SOCKET_TYPE
    };

    enum RECEIVE_FLAG_TYPE {
        PEEK,
        OOB_RECEIVE_FLAG_TYPE,
        WAITALL,
        NOT_DEFINED_RECEIVE_FLAG_TYPE
    };

    enum SEND_FLAG_TYPE {
        EOR,
        OOB_SEND_FLAG_TYPE,
        NOSIGNAL,
        NOT_DEFINED_SEND_FLAG_TYPE
    };

    enum VALIDITY_TYPE {
        INVALID,
        VALID
    };

    enum MESSAGING_PATTERN_TYPE {
        PUB_SUB,
        CLIENT,
        SERVER
    };

    typedef int32_t WAITING_RANGE_TYPE;

    class TRANSPORT_CONNECTION_STATUS_TYPE OSPL_DDS_FINAL
    {
    public:
        FACE::MESSAGE_RANGE_TYPE MESSAGE_;
        FACE::MESSAGE_RANGE_TYPE MAX_MESSAGE_;
        FACE::MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE_;
        FACE::CONNECTION_DIRECTION_TYPE CONNECTION_DIRECTION_;
        FACE::WAITING_RANGE_TYPE WAITING_PROCESSES_OR_MESSAGES_;
        FACE::SYSTEM_TIME_TYPE REFRESH_PERIOD_;
        FACE::VALIDITY_TYPE LAST_MSG_VALIDITY_;

    public:
        TRANSPORT_CONNECTION_STATUS_TYPE() :
                MESSAGE_(0),
                MAX_MESSAGE_(0),
                MAX_MESSAGE_SIZE_(0),
                CONNECTION_DIRECTION_(OSPL_ENUM_LABEL(FACE,CONNECTION_DIRECTION_TYPE,SOURCE)),
                WAITING_PROCESSES_OR_MESSAGES_(0),
                REFRESH_PERIOD_(0),
                LAST_MSG_VALIDITY_(OSPL_ENUM_LABEL(FACE,VALIDITY_TYPE,INVALID)) {}

        explicit TRANSPORT_CONNECTION_STATUS_TYPE(
            FACE::MESSAGE_RANGE_TYPE MESSAGE,
            FACE::MESSAGE_RANGE_TYPE MAX_MESSAGE,
            FACE::MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE,
            FACE::CONNECTION_DIRECTION_TYPE CONNECTION_DIRECTION,
            FACE::WAITING_RANGE_TYPE WAITING_PROCESSES_OR_MESSAGES,
            FACE::SYSTEM_TIME_TYPE REFRESH_PERIOD,
            FACE::VALIDITY_TYPE LAST_MSG_VALIDITY) : 
                MESSAGE_(MESSAGE),
                MAX_MESSAGE_(MAX_MESSAGE),
                MAX_MESSAGE_SIZE_(MAX_MESSAGE_SIZE),
                CONNECTION_DIRECTION_(CONNECTION_DIRECTION),
                WAITING_PROCESSES_OR_MESSAGES_(WAITING_PROCESSES_OR_MESSAGES),
                REFRESH_PERIOD_(REFRESH_PERIOD),
                LAST_MSG_VALIDITY_(LAST_MSG_VALIDITY) {}

        TRANSPORT_CONNECTION_STATUS_TYPE(const TRANSPORT_CONNECTION_STATUS_TYPE &_other) : 
                MESSAGE_(_other.MESSAGE_),
                MAX_MESSAGE_(_other.MAX_MESSAGE_),
                MAX_MESSAGE_SIZE_(_other.MAX_MESSAGE_SIZE_),
                CONNECTION_DIRECTION_(_other.CONNECTION_DIRECTION_),
                WAITING_PROCESSES_OR_MESSAGES_(_other.WAITING_PROCESSES_OR_MESSAGES_),
                REFRESH_PERIOD_(_other.REFRESH_PERIOD_),
                LAST_MSG_VALIDITY_(_other.LAST_MSG_VALIDITY_) {}

#ifdef OSPL_DDS_CXX11
        TRANSPORT_CONNECTION_STATUS_TYPE(TRANSPORT_CONNECTION_STATUS_TYPE &&_other) : 
                MESSAGE_(::std::move(_other.MESSAGE_)),
                MAX_MESSAGE_(::std::move(_other.MAX_MESSAGE_)),
                MAX_MESSAGE_SIZE_(::std::move(_other.MAX_MESSAGE_SIZE_)),
                CONNECTION_DIRECTION_(::std::move(_other.CONNECTION_DIRECTION_)),
                WAITING_PROCESSES_OR_MESSAGES_(::std::move(_other.WAITING_PROCESSES_OR_MESSAGES_)),
                REFRESH_PERIOD_(::std::move(_other.REFRESH_PERIOD_)),
                LAST_MSG_VALIDITY_(::std::move(_other.LAST_MSG_VALIDITY_)) {}

        TRANSPORT_CONNECTION_STATUS_TYPE& operator=(TRANSPORT_CONNECTION_STATUS_TYPE &&_other)
        {
            if (this != &_other) {
                MESSAGE_ = ::std::move(_other.MESSAGE_);
                MAX_MESSAGE_ = ::std::move(_other.MAX_MESSAGE_);
                MAX_MESSAGE_SIZE_ = ::std::move(_other.MAX_MESSAGE_SIZE_);
                CONNECTION_DIRECTION_ = ::std::move(_other.CONNECTION_DIRECTION_);
                WAITING_PROCESSES_OR_MESSAGES_ = ::std::move(_other.WAITING_PROCESSES_OR_MESSAGES_);
                REFRESH_PERIOD_ = ::std::move(_other.REFRESH_PERIOD_);
                LAST_MSG_VALIDITY_ = ::std::move(_other.LAST_MSG_VALIDITY_);
            }
            return *this;
        }
#endif

        TRANSPORT_CONNECTION_STATUS_TYPE& operator=(const TRANSPORT_CONNECTION_STATUS_TYPE &_other)
        {
            if (this != &_other) {
                MESSAGE_ = _other.MESSAGE_;
                MAX_MESSAGE_ = _other.MAX_MESSAGE_;
                MAX_MESSAGE_SIZE_ = _other.MAX_MESSAGE_SIZE_;
                CONNECTION_DIRECTION_ = _other.CONNECTION_DIRECTION_;
                WAITING_PROCESSES_OR_MESSAGES_ = _other.WAITING_PROCESSES_OR_MESSAGES_;
                REFRESH_PERIOD_ = _other.REFRESH_PERIOD_;
                LAST_MSG_VALIDITY_ = _other.LAST_MSG_VALIDITY_;
            }
            return *this;
        }

        bool operator==(const TRANSPORT_CONNECTION_STATUS_TYPE& _other) const
        {
            return MESSAGE_ == _other.MESSAGE_ &&
                MAX_MESSAGE_ == _other.MAX_MESSAGE_ &&
                MAX_MESSAGE_SIZE_ == _other.MAX_MESSAGE_SIZE_ &&
                CONNECTION_DIRECTION_ == _other.CONNECTION_DIRECTION_ &&
                WAITING_PROCESSES_OR_MESSAGES_ == _other.WAITING_PROCESSES_OR_MESSAGES_ &&
                REFRESH_PERIOD_ == _other.REFRESH_PERIOD_ &&
                LAST_MSG_VALIDITY_ == _other.LAST_MSG_VALIDITY_;
        }

        bool operator!=(const TRANSPORT_CONNECTION_STATUS_TYPE& _other) const
        {
            return !(*this == _other);
        }

        FACE::MESSAGE_RANGE_TYPE MESSAGE() const { return this->MESSAGE_; }
        FACE::MESSAGE_RANGE_TYPE& MESSAGE() { return this->MESSAGE_; }
        void MESSAGE(FACE::MESSAGE_RANGE_TYPE _val_) { this->MESSAGE_ = _val_; }
        FACE::MESSAGE_RANGE_TYPE MAX_MESSAGE() const { return this->MAX_MESSAGE_; }
        FACE::MESSAGE_RANGE_TYPE& MAX_MESSAGE() { return this->MAX_MESSAGE_; }
        void MAX_MESSAGE(FACE::MESSAGE_RANGE_TYPE _val_) { this->MAX_MESSAGE_ = _val_; }
        FACE::MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE() const { return this->MAX_MESSAGE_SIZE_; }
        FACE::MESSAGE_SIZE_TYPE& MAX_MESSAGE_SIZE() { return this->MAX_MESSAGE_SIZE_; }
        void MAX_MESSAGE_SIZE(FACE::MESSAGE_SIZE_TYPE _val_) { this->MAX_MESSAGE_SIZE_ = _val_; }
        FACE::CONNECTION_DIRECTION_TYPE CONNECTION_DIRECTION() const { return this->CONNECTION_DIRECTION_; }
        FACE::CONNECTION_DIRECTION_TYPE& CONNECTION_DIRECTION() { return this->CONNECTION_DIRECTION_; }
        void CONNECTION_DIRECTION(FACE::CONNECTION_DIRECTION_TYPE _val_) { this->CONNECTION_DIRECTION_ = _val_; }
        FACE::WAITING_RANGE_TYPE WAITING_PROCESSES_OR_MESSAGES() const { return this->WAITING_PROCESSES_OR_MESSAGES_; }
        FACE::WAITING_RANGE_TYPE& WAITING_PROCESSES_OR_MESSAGES() { return this->WAITING_PROCESSES_OR_MESSAGES_; }
        void WAITING_PROCESSES_OR_MESSAGES(FACE::WAITING_RANGE_TYPE _val_) { this->WAITING_PROCESSES_OR_MESSAGES_ = _val_; }
        FACE::SYSTEM_TIME_TYPE REFRESH_PERIOD() const { return this->REFRESH_PERIOD_; }
        FACE::SYSTEM_TIME_TYPE& REFRESH_PERIOD() { return this->REFRESH_PERIOD_; }
        void REFRESH_PERIOD(FACE::SYSTEM_TIME_TYPE _val_) { this->REFRESH_PERIOD_ = _val_; }
        FACE::VALIDITY_TYPE LAST_MSG_VALIDITY() const { return this->LAST_MSG_VALIDITY_; }
        FACE::VALIDITY_TYPE& LAST_MSG_VALIDITY() { return this->LAST_MSG_VALIDITY_; }
        void LAST_MSG_VALIDITY(FACE::VALIDITY_TYPE _val_) { this->LAST_MSG_VALIDITY_ = _val_; }
    };

}

#endif /* _FACE_TS_COMMON_H_ */
