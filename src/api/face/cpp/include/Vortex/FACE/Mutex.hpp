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

#ifndef VORTEX_FACE_MUTEX_HPP_
#define VORTEX_FACE_MUTEX_HPP_

#include "os_mutex.h"
#include "Vortex/FACE/Macros.hpp"

namespace Vortex {
namespace FACE {

class VORTEX_FACE_API Mutex
{
public:
    Mutex();
    virtual ~Mutex();

    void lock() const;
    bool try_lock() const;
    void unlock() const;
private:
    os_mutex mtx;
};


class VORTEX_FACE_API MutexScoped
{
public:
    MutexScoped(const Mutex& mutex);
    ~MutexScoped();

private:
    const Mutex& mtx;
};

}; /* namespace FACE */
}; /* namespace Vortex */


#endif /* VORTEX_FACE_MUTEX_HPP_ */
