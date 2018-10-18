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
#ifndef VORTEX_FACE_CONNECTION_FACTORY_HPP_
#define VORTEX_FACE_CONNECTION_FACTORY_HPP_

#include "Vortex_FACE.hpp"
#include "Vortex/FACE/AnyConnection.hpp"

namespace Vortex {
namespace FACE {

/*
 * This class is mainly a map of <type name, typed create function>
 * pairs. The 'type name' is the key, while the 'typed create function'
 * can be used to create Connection<TYPE> objects.
 *
 * When the type is found in the map, then the 'second' part of
 * the found pair is actually the templated create function
 * "newConnection<TYPE>".
 * Meaning that the it->second() will return a new instance of
 * the Connection<TYPE> cast to AnyConnection.
 */
class VORTEX_FACE_API ConnectionFactory
{
public:
    typedef std::map<std::string, AnyConnection*(*)()> CF_MAP_TYPE;

    /*
     * This function can be used to create a Connection<TYPE>:
     *   AnyConnection            *anyConnection   = ConnectionFactory::createConnection("Space::Type1");
     *   Connection<Space::Type1> *typedConnection = dynamic_cast< Connection<Space::Type1> >(anyConnection);
     *   typedConnection->doTypedStuff();
     */
    static AnyConnection*
    createConnection(const std::string &typeName);

    /*
     * This function can be used to check if this factory can create
     * Connection of the given type.
     */
    static bool
    knows(const std::string &typeName);

protected:
    /* Templated function to create a typed Connection. */
    template<typename TYPE>
    static AnyConnection*
    newConnection() {
        return new TYPE;
    }

    /* Function to get the global factory map. */
    static CF_MAP_TYPE*
    getMap();

private:
    /* Translate java style typename "Space.Type1" to a
     * C++ style typeName "Space::Type1". */
    std::string
    static translate(const std::string &typeName);
};



/*
 * This class will add a templated create function "newConnection<TYPE>"
 * to the ConnectionFactory map with the type name as key.
 *
 * This class will be a static member variable of the specialized typed
 * connections which means that typed creation function is registered
 * automatically for the given typed connection.
 *
 * Example for Space::Type1:
 *
 *   class SpaceType1ConnectionRegistration
 *   {
 *       static ConnectionFactoryTypeRegister< Connection<Space::Type1> > reg;
 *   };
 *   ConnectionFactoryTypeRegister< Connection<Space::Type1> > SpaceType1ConnectionRegistration::reg("Space::Type1");
 *
 * The initialization of SpaceType1ConnectionRegistration::reg; will result
 * in a <type name, typed create function> pair in the ConnectionFactory map.
 */
template<typename TYPE>
struct ConnectionFactoryTypeRegister : ConnectionFactory {
    /* When this class is a static object, then this constructor is called at
     * startup. This means that the related type is registered at startup as well */
    ConnectionFactoryTypeRegister(std::string const& typeName) {
        /* Add the <type name, typed create function> pair to the factory. */
        getMap()->insert(std::pair<std::string, AnyConnection*(*)()>(typeName, &newConnection<TYPE>));
    }
};


}; /* namespace FACE */
}; /* namespace Vortex */

#endif /* VORTEX_FACE_REPORT_SUPPORT_HPP_ */
