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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_CORE_ENTITY_REGISTRY_HPP_
#define ORG_OPENSPLICE_CORE_ENTITY_REGISTRY_HPP_

#include <dds/core/detail/WeakReferenceImpl.hpp>
#include <org/opensplice/core/Mutex.hpp>

#include <map>

namespace org
{
namespace opensplice
{
namespace core
{

template <typename T, typename U>
class EntityRegistry
{
public:
    /**
     *  @internal Inserts a EntityDelegate key, dds::core::Entity value pair into the registry.
     * @param key The org::opensplice::core::Entity to use as a key
     * @param val The dds::core::Entity that encapsulates the key
     */
    void insert(T key, U& val)
    {
        mutex.lock();
        registry[key] = dds::core::WeakReference<U>(val);
        mutex.unlock();
    }

    /**
     *  @internal Removes a DDS::Entity from the registry
     * @param key Entity to remove
     */
    void remove(T key)
    {
        mutex.lock();
        registry.erase(key);
        mutex.unlock();
    }

    /**
     *  @internal Checks the registry for a dds::core::Entity that wraps the supplied DDS::Entity
     * and returns it. If no match is found dds::core::null is returned.
     * @param key DDS::Entity to find an encapsulating dds::core:Entity for.
     * @return dds::core::Entity if a match is found, dds::core::null if not.
     */
    U get(T key)
    {
        typename std::map<T, dds::core::WeakReference<U> >::iterator it;

        mutex.lock();
        it = registry.find(key);

        U entity(dds::core::null);
        if(it != registry.end())
        {
            entity = it->second.lock();
        }
        mutex.unlock();

        return entity;
    }

private:
    std::map<T, dds::core::WeakReference<U> > registry;
    org::opensplice::core::Mutex mutex;
};

}
}
}


#endif /* ORG_OPENSPLICE_CORE_ENTITY_REGISTRY_HPP_ */
