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

#ifndef ORG_OPENSPLICE_CORE_OBJECT_SET_HPP_
#define ORG_OPENSPLICE_CORE_OBJECT_SET_HPP_

#include <dds/core/InstanceHandle.hpp>

#include <org/opensplice/core/ObjectDelegate.hpp>
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

class ObjectSet
{
public:
    typedef std::set<org::opensplice::core::ObjectDelegate::weak_ref_type>::iterator    setIterator;
    typedef std::vector<org::opensplice::core::ObjectDelegate::weak_ref_type>::iterator vectorIterator;
    typedef std::vector<org::opensplice::core::ObjectDelegate::weak_ref_type>           vector;

    /**
     *  @internal Inserts a EntityDelegate into the set.
     * @param entity The org::opensplice::core::ObjectDelegate to store
     */
    void insert(org::opensplice::core::ObjectDelegate& obj);

    /**
     *  @internal Erases a EntityDelegate from the set.
     * @param entity The org::opensplice::core::ObjectDelegate to delete
     */
    void erase(org::opensplice::core::ObjectDelegate& obj);

    /**
     *  @internal Call close() of all the Objects within the set (outside lock).
     */
    void all_close();

    /**
     *  @internal Copy internal set into a vector.
     */
    vector copy();

private:
    WeakReferenceSet<ObjectDelegate::weak_ref_type>::wset objects;
    Mutex mutex;
};

}
}
}


#endif /* ORG_OPENSPLICE_CORE_OBJECT_SET_HPP_ */
