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

#include <org/opensplice/core/EntitySet.hpp>
#include <org/opensplice/core/ScopedLock.hpp>

void
org::opensplice::core::EntitySet::insert(org::opensplice::core::EntityDelegate& entity)
{
    org::opensplice::core::ScopedMutexLock scopedLock(this->mutex);
    this->entities.insert(entity.get_weak_ref());
}

void
org::opensplice::core::EntitySet::erase(org::opensplice::core::EntityDelegate& entity)
{
    org::opensplice::core::ScopedMutexLock scopedLock(this->mutex);
    this->entities.erase(entity.get_weak_ref());
}

bool
org::opensplice::core::EntitySet::contains(const dds::core::InstanceHandle& handle)
{
    org::opensplice::core::ScopedMutexLock scopedLock(this->mutex);
    bool contains = false;

    WeakReferenceSet<ObjectDelegate::weak_ref_type>::iterator it;

    for (it = this->entities.begin(); !contains && (it != this->entities.end()); it++) {
        org::opensplice::core::ObjectDelegate::ref_type ref = it->lock();
        if (ref) {
            org::opensplice::core::EntityDelegate::ref_type entity =
                    OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<EntityDelegate>(ref);

            /* Check if current entity is the one that is searched for. */
            contains = (entity->instance_handle() == handle);
            if (!contains) {
                /* Check if current entity contains the one searched for. */
                contains = entity->contains_entity(handle);
            }
        }
    }

    return contains;
}

void
org::opensplice::core::EntitySet::all_close()
{
    /* Copy the entities to use them outside the lock. */
    vector vctr = this->copy();
    /* Call close() of all Entities. */
    for (vectorIterator it = vctr.begin(); it != vctr.end(); ++it) {
        org::opensplice::core::ObjectDelegate::ref_type ref = it->lock();
        if (ref) {
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<EntityDelegate>(ref).get()->close();
        }
    }
}

void
org::opensplice::core::EntitySet::all_retain()
{
    /* Copy the entities to use them outside the lock. */
    vector vctr = this->copy();
    /* Call retain() of all Entities. */
    for (vectorIterator it = vctr.begin(); it != vctr.end(); ++it) {
        org::opensplice::core::ObjectDelegate::ref_type ref = it->lock();
         if (ref) {
             OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<EntityDelegate>(ref).get()->retain();
         }
    }
}

void
org::opensplice::core::EntitySet::all_enable()
{
    /* Copy the entities to use them outside the lock. */
    vector vctr = this->copy();
    /* Call enable() of all Entities. */
    for (vectorIterator it = vctr.begin(); it != vctr.end(); ++it) {
        org::opensplice::core::ObjectDelegate::ref_type ref = it->lock();
         if (ref) {
             OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<EntityDelegate>(ref).get()->close();
         }
    }
}

org::opensplice::core::EntitySet::vector
org::opensplice::core::EntitySet::copy()
{
    org::opensplice::core::ScopedMutexLock scopedLock(this->mutex);
    vector vctr(this->entities.size());
    std::copy(this->entities.begin(), this->entities.end(), vctr.begin());
    return vctr;
}

