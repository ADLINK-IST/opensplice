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

#ifndef VORTEX_FACE_FACE_INSTANCE_HPP_
#define VORTEX_FACE_FACE_INSTANCE_HPP_

#include "os_atomics.h"

#include "Vortex_FACE.hpp"
#include "Vortex/FACE/Mutex.hpp"
#include "Vortex/FACE/Config.hpp"
#include "Vortex/FACE/AnyConnection.hpp"

namespace Vortex {
namespace FACE {

class VORTEX_FACE_API FaceInstance {
public: /* static */
    typedef Vortex::FACE::smart_ptr_traits< FaceInstance >::shared_ptr shared_ptr;
    typedef std::map< ::FACE::CONNECTION_ID_TYPE, AnyConnection::shared_ptr > FI_MAP_TYPE;

    FaceInstance();

    static ::FACE::RETURN_CODE_TYPE
    createInstance(const ::FACE::CONFIGURATION_RESOURCE   &configuration);

    static FaceInstance::shared_ptr
    getInstance();

public: /* object */
    ::FACE::RETURN_CODE_TYPE
    createConnection(const ::FACE::CONNECTION_NAME_TYPE      &connectionName,
                     const ::FACE::MESSAGING_PATTERN_TYPE    &pattern,
                           ::FACE::CONNECTION_ID_TYPE        &connectionId,
                           ::FACE::CONNECTION_DIRECTION_TYPE &connectionDirection,
                           ::FACE::MESSAGE_SIZE_TYPE         &maxMessageSize,
                     const ::FACE::TIMEOUT_TYPE              &timeout);

    AnyConnection::shared_ptr
    getConnection(const ::FACE::CONNECTION_ID_TYPE &connectionId);

    ::FACE::RETURN_CODE_TYPE
    destroyConnection(const ::FACE::CONNECTION_ID_TYPE &connectionId);

    int32_t
	getDomainId() const;

private: /* object */
    ::FACE::RETURN_CODE_TYPE
    init(const ::FACE::CONFIGURATION_RESOURCE   &configuration);

    AnyConnection::shared_ptr
    findConnection(const ::FACE::CONNECTION_NAME_TYPE &connectionName,
                         ::FACE::CONNECTION_ID_TYPE   &connectionId);

    FI_MAP_TYPE connections;
    Vortex::FACE::Config *config;
    pa_uint32_t pa_id;
    int32_t domainId;

private: /* static */
    static Mutex mutex;
    static FaceInstance::shared_ptr instance;
};


}; /* namespace FACE */
}; /* namespace Vortex */

#endif /* VORTEX_FACE_REPORT_SUPPORT_HPP_ */
