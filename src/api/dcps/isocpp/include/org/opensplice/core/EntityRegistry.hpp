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

#ifndef ORG_OPENSPLICE_SUB_SUBSCRIBER_REGISTRY_HPP_
#define ORG_OPENSPLICE_SUB_SUBSCRIBER_REGISTRY_HPP_

#include <dds/core/WeakReference.hpp>

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
     *  @internal Inserts a DDS::Entity key, dds::core::Entity value pair into the registry.
     * @param key The DDS::Entity to use as a key
     * @param val The dds::core::Entity that encapsulates the key
     */
    static void insert(T key, U val)
    {
        registry()[key] = dds::core::WeakReference<U>(val);
    }

    /**
     *  @internal Removes a DDS::Entity from the registry
     * @param key Entity to remove
     */
    static void remove(T key)
    {
        registry().erase(key);
    }

    /**
     *  @internal Checks the registry for a dds::core::Entity that wraps the supplied DDS::Entity
     * and returns it. If no match is found dds::core::null is returned.
     * @param key DDS::Entity to find an encapsulating dds::core:Entity for.
     * @return dds::core::Entity if a match is found, dds::core::null if not.
     */
    static U get(T key)
    {
        typename std::map<T, dds::core::WeakReference<U> >::iterator it;
        it = registry().find(key);

        U entity(dds::core::null);
        if(it != registry().end())
        {
            entity = it->second.lock();
        }
        return entity;
    }

private:
    static std::map<T, dds::core::WeakReference<U> >& registry();
};

}
}
}

#define INSTANTIATE_TYPED_REGISTRIES(TYPE) \
    namespace org { namespace opensplice { namespace core { \
    template <> \
    OS_API_EXPORT std::map<DDS::DataReader_ptr, dds::core::WeakReference<dds::sub::DataReader<TYPE> > >& \
    org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, dds::sub::DataReader<TYPE> >::registry() \
    { \
        static std::map<DDS::DataReader_ptr, dds::core::WeakReference<dds::sub::DataReader<TYPE> > > registry_; \
        return registry_; \
    }\
    \
    template <> \
    OS_API_EXPORT std::map<DDS::DataWriter_ptr, dds::core::WeakReference<dds::pub::DataWriter<TYPE> > >& \
    org::opensplice::core::EntityRegistry<DDS::DataWriter_ptr, dds::pub::DataWriter<TYPE> >::registry() \
    { \
        static std::map<DDS::DataWriter_ptr, dds::core::WeakReference<dds::pub::DataWriter<TYPE> > > registry_; \
        return registry_; \
    } \
    \
    template <> \
    OS_API_EXPORT std::map<std::string, dds::core::WeakReference<dds::topic::Topic<TYPE> > >& \
    org::opensplice::core::EntityRegistry<std::string, dds::topic::Topic<TYPE> >::registry() \
    { \
        static std::map<std::string, dds::core::WeakReference<dds::topic::Topic<TYPE> > > registry_; \
        return registry_; \
    }\
    }}}

#endif /* ORG_OPENSPLICE_SUB_SUBSCRIBER_REGISTRY_HPP_ */

