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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_CORE_ENTITY_SET_HPP_
#define ORG_OPENSPLICE_CORE_ENTITY_SET_HPP_

#include <dds/core/InstanceHandle.hpp>

#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/core/Mutex.hpp>
#include <org/opensplice/core/WeakReferenceSet.hpp>

#include <vector>
#include <memory>
#include <set>

namespace org
{
namespace opensplice
{
namespace core
{

class EntitySet
{
public:
    typedef std::vector<org::opensplice::core::ObjectDelegate::weak_ref_type>::iterator vectorIterator;
    typedef std::vector<org::opensplice::core::ObjectDelegate::weak_ref_type>           vector;

    /**
     *  @internal Inserts a EntityDelegate into the set.
     * @param entity The org::opensplice::core::EntityDelegate to store
     */
    void insert(org::opensplice::core::EntityDelegate& entity);

    /**
     *  @internal Erases a EntityDelegate from the set.
     * @param entity The org::opensplice::core::EntityDelegate to delete
     */
    void erase(org::opensplice::core::EntityDelegate& entity);

    /**
     *  @internal Check if Entity with specific handle is part of the set.
     * @param handle The dds::core::InstanceHandle to search for
     */
    bool contains(const dds::core::InstanceHandle& handle);

    /**
     *  @internal Call close() of all the Entities within the set (outside lock).
     */
    void all_close();

    /**
     *  @internal Call retain() of all the Entities within the set (outside lock).
     */
    void all_retain();

    /**
     *  @internal Call enable() of all the Entities within the set (outside lock).
     */
    void all_enable();

    /**
     *  @internal Copy internal set into a vector.
     */
    vector copy();

private:
    WeakReferenceSet<ObjectDelegate::weak_ref_type>::wset entities;
    Mutex mutex;
};

}
}
}


#endif /* ORG_OPENSPLICE_CORE_ENTITY_SET_HPP_ */
