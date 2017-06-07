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
#ifndef VORTEX_FACE_CONFIGURATION_HPP_
#define VORTEX_FACE_CONFIGURATION_HPP_

#include "Vortex_FACE.hpp"
#include "Vortex/FACE/ConnectionConfig.hpp"

C_CLASS(cf_data);
C_CLASS(cf_element);

namespace Vortex {
namespace FACE {

typedef enum {
    CONFIG_SCOPE_NONE,
    CONFIG_SCOPE_LIST,
    CONFIG_SCOPE_CONNECTION
} CONFIG_SCOPE;

class VORTEX_FACE_API Config
{
public:
    typedef std::map<std::string, ConnectionConfig::shared_ptr> CFG_MAP_TYPE;

    Config();

    ::FACE::RETURN_CODE_TYPE
    parse(const std::string &url);

    ConnectionConfig::shared_ptr
    find(const std::string &name);

    static std::string
    addUrlPrefix(const std::string &url);

private:
    ::FACE::RETURN_CODE_TYPE
    parseElement(cf_element element);

    ::FACE::RETURN_CODE_TYPE
    parseContainedElements(c_iter elements);

    ::FACE::RETURN_CODE_TYPE
    parseElementData(cf_data data);

    CFG_MAP_TYPE configs;
    ConnectionConfig *connection;
    std::string scopedTag;
    CONFIG_SCOPE scope;
};

}; /* namespace FACE */
}; /* namespace Vortex */

#endif /* VORTEX_FACE_CONFIGURATION_HPP_ */
