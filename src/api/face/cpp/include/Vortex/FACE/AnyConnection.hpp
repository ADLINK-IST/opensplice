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
#ifndef VORTEX_FACE_ANY_CONNECTION_HPP_
#define VORTEX_FACE_ANY_CONNECTION_HPP_

#include "Vortex_FACE.hpp"
#include "Vortex/FACE/ConnectionConfig.hpp"

namespace Vortex {
namespace FACE {

class VORTEX_FACE_API AnyConnection {
public:
    typedef Vortex::FACE::smart_ptr_traits< AnyConnection >::shared_ptr shared_ptr;

    virtual
    ~AnyConnection();

    ::FACE::RETURN_CODE_TYPE
    init(Vortex::FACE::ConnectionConfig::shared_ptr cfg);

    ::FACE::RETURN_CODE_TYPE
    parameters(::FACE::CONNECTION_NAME_TYPE             &connectionName,
               ::FACE::TRANSPORT_CONNECTION_STATUS_TYPE &connectionStatus);

    std::string
    getName();

    ::FACE::CONNECTION_DIRECTION_TYPE
    getDirection();

    virtual ::FACE::RETURN_CODE_TYPE
    unregisterCallback() = 0;

    virtual int32_t
	getDomainId() const = 0;

protected:
    AnyConnection();

    virtual void initWriter() = 0;
    virtual void initReader() = 0;

    void setLastValidity (::FACE::VALIDITY_TYPE validity);

    Vortex::FACE::ConnectionConfig::shared_ptr config;

    static dds::core::Duration copyIn(const ::FACE::TIMEOUT_TYPE &timeout);

private:
    ::FACE::VALIDITY_TYPE lastValidity;
};


}; /* namespace FACE */
}; /* namespace Vortex */

#endif /* VORTEX_FACE_ANY_CONNECTION_HPP_ */
