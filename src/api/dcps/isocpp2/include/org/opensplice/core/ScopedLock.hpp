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

#ifndef ORG_OPENSPLICE_CORE_SCOPEDLOCK_HPP_
#define ORG_OPENSPLICE_CORE_SCOPEDLOCK_HPP_

#include <org/opensplice/core/Mutex.hpp>
#include <org/opensplice/core/ObjectDelegate.hpp>

#include "os_report.h"

namespace org
{
namespace opensplice
{
namespace core
{

template <typename LOCKABLE>
class OMG_DDS_API ScopedLock
{
public:
    ScopedLock(const LOCKABLE& obj, bool lock = true)
        : lockable(obj),
          owner(lock)
    {
        if (lock) {
            lockable.lock();
        }
    }

    virtual ~ScopedLock()
    {
        if (owner) {
            try {
                lockable.unlock();
            } catch (...) {
                /* Don't know what to do anymore (it should have never failed)... */
                assert(false);
            }
        }
    }

    void lock()
    {
        assert(!owner);
        lockable.lock();
        owner = true;
    }

    bool try_lock()
    {
        bool locked;
        locked = lockable.try_lock();
        if (locked) {
            owner = true;
        }
        return locked;
    }

    void unlock()
    {
        assert(owner);
        owner = false;
        lockable.unlock();
    }

    bool own() const
    {
        return owner;
    }

private:
    const LOCKABLE& lockable;
    bool owner;
};

typedef ScopedLock<Mutex> ScopedMutexLock;
typedef ScopedLock<ObjectDelegate> ScopedObjectLock;

}
}
}



#endif /* ORG_OPENSPLICE_CORE_SCOPEDLOCK_HPP_ */
