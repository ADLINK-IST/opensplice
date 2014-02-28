/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
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

