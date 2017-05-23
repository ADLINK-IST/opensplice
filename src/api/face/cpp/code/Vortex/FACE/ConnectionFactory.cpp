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
#include "Vortex/FACE/ConnectionFactory.hpp"


Vortex::FACE::AnyConnection*
Vortex::FACE::ConnectionFactory::createConnection(const std::string &typeName) {
    /* Find the <type name, typed create function>
     * pair within the map. */
    CF_MAP_TYPE::iterator it = getMap()->find(ConnectionFactory::translate(typeName));
    if(it == getMap()->end()) {
        return NULL;
    }
    /* The second() is actually the templated create
     * function ConnectionFactory::newConnection<T>,
     * so this will create the Connection<TYPE>. */
    return it->second();
}


bool
Vortex::FACE::ConnectionFactory::knows(const std::string &typeName) {
    CF_MAP_TYPE::iterator it = getMap()->find(ConnectionFactory::translate(typeName));
    if(it == getMap()->end()) {
        return false;
    }
    return true;
}


Vortex::FACE::ConnectionFactory::CF_MAP_TYPE*
Vortex::FACE::ConnectionFactory::getMap() {
    /* This new() will only happen once.
     * It's here to prevent the “static initialization order fiasco”. */
    static Vortex::FACE::smart_ptr_traits< CF_MAP_TYPE >::shared_ptr map(new CF_MAP_TYPE);
    return map.get();
}

std::string
Vortex::FACE::ConnectionFactory::translate(const std::string &typeName) {
    size_t start_pos = 0;
    std::string ret  = typeName;
    std::string from = ".";
    std::string to   = "::";
    while((start_pos = ret.find(from, start_pos)) != std::string::npos) {
        ret.replace(start_pos, from.length(), to);
    }
    return ret;
}

