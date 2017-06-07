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

#include "Vortex/FACE/FaceInstance.hpp"

#include "Vortex/FACE/AnyConnection.hpp"
#include "Vortex/FACE/Connection.hpp"
#include "Vortex/FACE/ConnectionFactory.hpp"
#include "Vortex/FACE/ConnectionConfig.hpp"
#include "Vortex/FACE/ReportSupport.hpp"


Vortex::FACE::FaceInstance::FaceInstance() : config(NULL), domainId(-1)
{
    pa_st32(&pa_id, 0);
}


::FACE::RETURN_CODE_TYPE
Vortex::FACE::FaceInstance::createInstance(
                const ::FACE::CONFIGURATION_RESOURCE &configuration)
{
    ::FACE::RETURN_CODE_TYPE status = ::FACE::NO_ACTION;
    Vortex::FACE::MutexScoped lock(mutex);

    /* First make sure the user layer is initialized to allow Thread-specific
     * memory for error reporting. */
    if (u_userInitialise() != U_RESULT_OK) {
        OS_REPORT(OS_ERROR, "createInstance", 1, "Error. Initialization of user-layer failed.");
    }

    if (instance.get() == NULL) {
        instance.reset(new FaceInstance);
        status = instance->init(configuration);
        if (status != ::FACE::NO_ERROR) {
            FACE_REPORT_ERROR(status, "Initialization with \"%s\" failed.", configuration.c_str());
            instance.reset();
        }
    }
    return status;
}

Vortex::FACE::AnyConnection::shared_ptr
Vortex::FACE::FaceInstance::getConnection(
                const ::FACE::CONNECTION_ID_TYPE &connectionId)
{
    if (connectionId < 0) {
        /* Invalid: return empty pointer. */
        FACE_REPORT_ERROR(::FACE::INVALID_PARAM, "Invalid connection id '%d'.", (int)connectionId);
        return AnyConnection::shared_ptr();
    }

    FI_MAP_TYPE::iterator it = this->connections.find(connectionId);
    if(it == this->connections.end()) {
        /* Not available: return empty pointer. */
        FACE_REPORT_ERROR(::FACE::INVALID_PARAM, "Connection id '%d' not available.", (int)connectionId);
        return AnyConnection::shared_ptr();
    }

    /* Return the found AnyConnection shared_ptr. */
    return it->second;
}

Vortex::FACE::AnyConnection::shared_ptr
Vortex::FACE::FaceInstance::findConnection(
                const ::FACE::CONNECTION_NAME_TYPE &connectionName,
                      ::FACE::CONNECTION_ID_TYPE   &connectionId)
{
    /* Find connection based on name. */
    for (FI_MAP_TYPE::iterator it = this->connections.begin(); it != this->connections.end(); ++it) {
        if (strcmp(connectionName.c_str(), it->second->getName().c_str()) == 0) {
            /* Return the key as connectionId and the value as AnyConnection shared_ptr. */
            connectionId = it->first;
            return it->second;
        }
    }

    /* Nothing found. */
    connectionId = -1;
    return AnyConnection::shared_ptr();
}


::FACE::RETURN_CODE_TYPE
Vortex::FACE::FaceInstance::destroyConnection(const ::FACE::CONNECTION_ID_TYPE &connectionId)
{
    if (this->connections.erase(connectionId) == 1) {
        return ::FACE::NO_ERROR;
    }
    return ::FACE::INVALID_PARAM;
}

::FACE::RETURN_CODE_TYPE
Vortex::FACE::FaceInstance::init(
                const ::FACE::CONFIGURATION_RESOURCE &configuration)
{
    ::FACE::RETURN_CODE_TYPE status;
    this->config = new Vortex::FACE::Config();
    status = this->config->parse(configuration);
    if (status != ::FACE::NO_ERROR) {
        delete this->config;
        this->config = NULL;
    }
    return status;
}

Vortex::FACE::FaceInstance::shared_ptr
Vortex::FACE::FaceInstance::getInstance()
{
    return instance;
}

::FACE::RETURN_CODE_TYPE
Vortex::FACE::FaceInstance::createConnection(
                const ::FACE::CONNECTION_NAME_TYPE      &connectionName,
                const ::FACE::MESSAGING_PATTERN_TYPE    &pattern,
                      ::FACE::CONNECTION_ID_TYPE        &connectionId,
                      ::FACE::CONNECTION_DIRECTION_TYPE &connectionDirection,
                      ::FACE::MESSAGE_SIZE_TYPE         &maxMessageSize,
                const ::FACE::TIMEOUT_TYPE              &timeout)
{
    ::FACE::RETURN_CODE_TYPE status = ::FACE::INVALID_CONFIG;
    ::FACE::CONNECTION_ID_TYPE startId;
    Vortex::FACE::AnyConnection::shared_ptr connection;
    Vortex::FACE::ConnectionConfig::shared_ptr config;
    Vortex::FACE::MutexScoped lock(mutex);

    /* Don't create this connection when already available. */
    connection = this->findConnection(connectionName, connectionId);
    if (connection.get() == NULL) {
        /* Get the configuration of this particular connection. */
        config = this->config->find(connectionName);
        if (config.get() != NULL) {
            connectionDirection = config->getDirection();
            /* Create the typed connection according to the type name. */
            connection.reset(Vortex::FACE::ConnectionFactory::createConnection(config->getTypeName()));
            if (connection.get() != NULL) {
                status = connection->init(config);
                if (status == ::FACE::NO_ERROR) {
                    this->domainId = connection->getDomainId();
                    /* Remember previous id to detect possible (unlikely) usage of all connection ids. */
                    startId = (::FACE::CONNECTION_ID_TYPE)pa_ld32(&pa_id);
                    /* Store this new connection (retry when id is already in use). */
                    connectionId = (::FACE::CONNECTION_ID_TYPE)pa_inc32_nv(&pa_id);
                    while ((connectionId != startId) &&
                           (this->connections.insert(std::make_pair(connectionId, connection)).second == false))
                    {
                        connectionId = (::FACE::CONNECTION_ID_TYPE)pa_inc32_nv(&pa_id);
                    }
                    if (connectionId == startId) {
                        connectionId = -1;
                        status = ::FACE::NOT_AVAILABLE;
                        FACE_REPORT_ERROR(status, "Could not get id for connection \"%s\" (all ids are already in use).", connectionName.c_str());
                    } else {
                        int32_t curid, newid = connection->getDomainId();
                        for (FI_MAP_TYPE::iterator it = this->connections.begin(); it != this->connections.end(); ++it) {
                            curid = it->second->getDomainId();
                            if (curid != newid) {
                                newid = (int32_t)org::opensplice::domain::default_id();
                            }
                        }
                        this->domainId = newid;
                    }
                } else {
                    FACE_REPORT_ERROR(status, "Could not initialize connection \"%s\".", connectionName.c_str());
                }
            } else {
                FACE_REPORT_ERROR(status, "Could not create connection \"%s\": out of resources.", connectionName.c_str());
            }
        } else {
            FACE_REPORT_ERROR(status, "Could not find config for connection \"%s\".", connectionName.c_str());
        }
    } else {
        /* Connection was already created. */
        status = ::FACE::NO_ACTION;
        connectionDirection = connection->getDirection();
    }

    /* maxMessageSize will be ignored... */
    maxMessageSize = 0;

    return status;
}

int32_t
Vortex::FACE::FaceInstance::getDomainId() const
{
	return this->domainId;
}


Vortex::FACE::Mutex                    Vortex::FACE::FaceInstance::mutex;
Vortex::FACE::FaceInstance::shared_ptr Vortex::FACE::FaceInstance::instance;
