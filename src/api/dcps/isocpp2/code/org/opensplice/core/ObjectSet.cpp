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

#include <org/opensplice/core/ObjectSet.hpp>
#include <org/opensplice/core/ScopedLock.hpp>

void
org::opensplice::core::ObjectSet::insert(org::opensplice::core::ObjectDelegate& obj)
{
    org::opensplice::core::ScopedMutexLock scopedLock(this->mutex);
    this->objects.insert(obj.get_weak_ref());
}

void
org::opensplice::core::ObjectSet::erase(org::opensplice::core::ObjectDelegate& obj)
{
    org::opensplice::core::ScopedMutexLock scopedLock(this->mutex);
    this->objects.erase(obj.get_weak_ref());
}

void
org::opensplice::core::ObjectSet::all_close()
{
    /* Copy the objects to use them outside the lock. */
    vector vctr = this->copy();
    /* Call close() of all Objects. */
    for (vectorIterator it = vctr.begin(); it != vctr.end(); ++it) {
        org::opensplice::core::ObjectDelegate::ref_type ref = it->lock();
        if (ref) {
            ref->close();
        }
    }
}

org::opensplice::core::ObjectSet::vector
org::opensplice::core::ObjectSet::copy()
{
    org::opensplice::core::ScopedMutexLock scopedLock(this->mutex);
    vector vctr(this->objects.size());
    std::copy(this->objects.begin(), this->objects.end(), vctr.begin());
    return vctr;
}
