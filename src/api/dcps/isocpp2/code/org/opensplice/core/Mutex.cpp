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

#include <org/opensplice/core/Mutex.hpp>
#include <org/opensplice/core/ReportUtils.hpp>


org::opensplice::core::Mutex::Mutex()
{
    os_result osr;

    osr = os_mutexInit(&this->mtx, NULL);
    if (osr != os_resultSuccess) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not initialize mutex.");
    }

}

org::opensplice::core::Mutex::~Mutex()
{
    os_mutexDestroy(&this->mtx);
}

void
org::opensplice::core::Mutex::lock() const
{
    os_mutexLock((os_mutex *)&this->mtx);
}

bool
org::opensplice::core::Mutex::try_lock() const
{
    os_result osr = os_mutexTryLock((os_mutex *)&this->mtx);
    if (osr == os_resultSuccess) { return true;  }
    if (osr == os_resultBusy   ) { return false; }
    assert(false);
    return false;
}

void
org::opensplice::core::Mutex::unlock() const
{
    os_mutexUnlock((os_mutex *)&this->mtx);
}
