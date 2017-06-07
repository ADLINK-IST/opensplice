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
#include "Vortex/FACE/ReportSupport.hpp"


namespace FACE {
namespace TS {


void Initialize(
            const ::FACE::CONFIGURATION_RESOURCE   &configuration,
                  ::FACE::RETURN_CODE_TYPE         &return_code)
{
    FACE_REPORT_STACK_BEGIN();

    return_code = Vortex::FACE::FaceInstance::createInstance(configuration);

    FACE_REPORT_STACK_END(return_code != ::FACE::NO_ERROR);
}


void Create_Connection(
            const ::FACE::CONNECTION_NAME_TYPE      &connection_name,
            const ::FACE::MESSAGING_PATTERN_TYPE    &pattern,
                  ::FACE::CONNECTION_ID_TYPE        &connection_id,
                  ::FACE::CONNECTION_DIRECTION_TYPE &connection_direction,
                  ::FACE::MESSAGE_SIZE_TYPE         &max_message_size,
            const ::FACE::TIMEOUT_TYPE              &timeout,
                  ::FACE::RETURN_CODE_TYPE          &return_code)
{
    FACE_REPORT_STACK_BEGIN();

    if (pattern == ::FACE::PUB_SUB) {
        Vortex::FACE::FaceInstance::shared_ptr instance = Vortex::FACE::FaceInstance::getInstance();
        if (instance.get() != NULL) {
            return_code = instance->createConnection(connection_name,
                                                     pattern,
                                                     connection_id,
                                                     connection_direction,
                                                     max_message_size,
                                                     timeout);
        } else {
            return_code = ::FACE::INVALID_CONFIG;
            FACE_REPORT_ERROR(return_code, "Configuration is invalid");
        }
    } else {
        return_code = ::FACE::INVALID_PARAM;
        FACE_REPORT_ERROR(return_code, "Expected pattern = PUB_SUB");
    }

    FACE_REPORT_STACK_END(return_code != ::FACE::NO_ERROR);
}


void Destroy_Connection(
            const ::FACE::CONNECTION_ID_TYPE   &connection_id,
                  ::FACE::RETURN_CODE_TYPE     &return_code)
{
    FACE_REPORT_STACK_BEGIN();

    Vortex::FACE::FaceInstance::shared_ptr instance = Vortex::FACE::FaceInstance::getInstance();
    if (instance.get() != NULL) {
        return_code = instance->destroyConnection(connection_id);
    } else {
        return_code = ::FACE::INVALID_CONFIG;
        FACE_REPORT_ERROR(return_code, "Instance not initialized");
    }

    FACE_REPORT_STACK_END(return_code != ::FACE::NO_ERROR);
}


void Get_Connection_Parameters(
                  ::FACE::CONNECTION_NAME_TYPE &connection_name,
                  ::FACE::CONNECTION_ID_TYPE &connection_id,
                  ::FACE::TRANSPORT_CONNECTION_STATUS_TYPE &connection_status,
                  ::FACE::RETURN_CODE_TYPE &return_code)
{
    FACE_REPORT_STACK_BEGIN();

    Vortex::FACE::FaceInstance::shared_ptr instance = Vortex::FACE::FaceInstance::getInstance();
    if (instance.get() != NULL) {
        Vortex::FACE::AnyConnection::shared_ptr connection = instance->getConnection(connection_id);
        if (connection.get() != NULL) {
            return_code = connection->parameters(connection_name,
                                                 connection_status);
        } else {
            return_code = ::FACE::INVALID_PARAM;
            FACE_REPORT_ERROR(return_code, "Failed to find connection '%d'", (int)connection_id);
        }
    } else {
        return_code = ::FACE::INVALID_CONFIG;
        FACE_REPORT_ERROR(return_code, "Instance not initialized");
    }

    FACE_REPORT_STACK_END(return_code != ::FACE::NO_ERROR);
}


void Unregister_Callback(
            const ::FACE::CONNECTION_ID_TYPE &connection_id,
                  ::FACE::RETURN_CODE_TYPE   &return_code)
{
    FACE_REPORT_STACK_BEGIN();

    Vortex::FACE::FaceInstance::shared_ptr instance = Vortex::FACE::FaceInstance::getInstance();
    if (instance.get() != NULL) {
        Vortex::FACE::AnyConnection::shared_ptr connection = instance->getConnection(connection_id);
        if (connection.get() != NULL) {
            return_code = connection->unregisterCallback();
        } else {
            return_code = ::FACE::INVALID_PARAM;
            FACE_REPORT_ERROR(return_code, "Failed to find connection '%d'", (int)connection_id);
        }
    } else {
        return_code = ::FACE::INVALID_CONFIG;
        FACE_REPORT_ERROR(return_code, "Instance not initialized");
    }

    FACE_REPORT_STACK_END(return_code != ::FACE::NO_ERROR);
}


}; /* namespace TS */
}; /* namespace FACE */
