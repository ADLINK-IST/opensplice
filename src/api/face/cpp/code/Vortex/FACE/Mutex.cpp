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

#include "Vortex/FACE/Mutex.hpp"


Vortex::FACE::Mutex::Mutex()
{
    os_result osr;
    osr = os_mutexInit(&this->mtx, NULL);
    if (osr != os_resultSuccess) {
        assert(false);
        /* TODO: (OSPL-8901) Add trace. */
    }

}

Vortex::FACE::Mutex::~Mutex()
{
    os_mutexDestroy(&this->mtx);
}

void
Vortex::FACE::Mutex::lock() const
{
    os_mutexLock((os_mutex *)&this->mtx);
}

bool
Vortex::FACE::Mutex::try_lock() const
{
    os_result osr = os_mutexTryLock((os_mutex *)&this->mtx);
    if (osr == os_resultSuccess) { return true;  }
    if (osr == os_resultBusy   ) { return false; }
    assert(false);
    return false;
}

void
Vortex::FACE::Mutex::unlock() const
{
    os_mutexUnlock((os_mutex *)&this->mtx);
}


Vortex::FACE::MutexScoped::MutexScoped(const Mutex& mutex) : mtx(mutex)
{
    mtx.lock();
}

Vortex::FACE::MutexScoped::~MutexScoped()
{
    try {
        mtx.unlock();
    } catch (...) {
        /* Don't know what to do anymore (it should have never failed)... */
        assert(false);
    }
}

