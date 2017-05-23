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
#include "Vortex_FACE.hpp"

#include "Vortex/FACE/AnyConnection.hpp"
#include "Vortex/FACE/ConnectionConfig.hpp"
#include "Vortex/FACE/ReportSupport.hpp"


Vortex::FACE::AnyConnection::AnyConnection() :
    lastValidity(::FACE::VALID)
{
}


Vortex::FACE::AnyConnection::~AnyConnection()
{
}


::FACE::RETURN_CODE_TYPE
Vortex::FACE::AnyConnection::init(Vortex::FACE::ConnectionConfig::shared_ptr cfg)
{
    ::FACE::RETURN_CODE_TYPE status = ::FACE::NO_ERROR;

    assert(cfg.get());

    this->config = cfg;
    this->lastValidity = ::FACE::VALID;
    try {
        switch (this->config->getDirection()) {
            case ::FACE::SOURCE:
                this->initWriter();
                break;
            case ::FACE::DESTINATION:
                this->initReader();
                break;
            case ::FACE::BI_DIRECTIONAL:
                this->initReader();
                this->initWriter();
                break;
            default:
                status = ::FACE::INVALID_CONFIG;
                FACE_REPORT_ERROR(status, "Direction in configuration is not valid");
                assert(FALSE);
                break;
        }
    } catch (const dds::core::Exception& e) {
        status = Vortex::FACE::exceptionToReturnCode(e);
        this->lastValidity = ::FACE::INVALID;
    } catch (...) {
        status = ::FACE::NO_ACTION;
        this->lastValidity = ::FACE::INVALID;
        assert(false);
    }

    return status;
}


std::string
Vortex::FACE::AnyConnection::getName()
{
    return this->config->getConnectionName();
}


::FACE::CONNECTION_DIRECTION_TYPE
Vortex::FACE::AnyConnection::getDirection()
{
    return this->config->getDirection();
}


::FACE::RETURN_CODE_TYPE
Vortex::FACE::AnyConnection::parameters(::FACE::CONNECTION_NAME_TYPE             &connectionName,
                                        ::FACE::TRANSPORT_CONNECTION_STATUS_TYPE &connectionStatus)
{
    connectionName = this->config->getConnectionName();

    connectionStatus.MESSAGE_                        = 0;
    connectionStatus.MAX_MESSAGE_                    = 1;
    connectionStatus.MAX_MESSAGE_SIZE_               = 0;
    connectionStatus.CONNECTION_DIRECTION_           = this->config->getDirection();
    connectionStatus.WAITING_PROCESSES_OR_MESSAGES_  = 0;
    connectionStatus.REFRESH_PERIOD_                 = this->config->getRefreshPeriod();
    connectionStatus.LAST_MSG_VALIDITY_              = this->lastValidity;

    return ::FACE::NO_ERROR;
}

void
Vortex::FACE::AnyConnection::setLastValidity(::FACE::VALIDITY_TYPE validity)
{
    this->lastValidity = validity;
}

dds::core::Duration
Vortex::FACE::AnyConnection::copyIn(const ::FACE::TIMEOUT_TYPE &timeout)
{
    dds::core::Duration d;
    if (timeout == ::FACE::INF_TIME_VALUE) {
        d = dds::core::Duration::infinite();
    } else {
        d.sec(timeout / 1000000000);
        d.nanosec(timeout % 1000000000);
    }
    return d;
}
