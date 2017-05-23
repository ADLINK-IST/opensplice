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

#include <org/opensplice/core/ObjectDelegate.hpp>
#include <org/opensplice/core/ReportUtils.hpp>

#include "u_object.h"


org::opensplice::core::ObjectDelegate::ObjectDelegate() :
        closed(false), domainId(-1)
{
}

org::opensplice::core::ObjectDelegate::~ObjectDelegate()
{
}

void
org::opensplice::core::ObjectDelegate::check() const
{
    /* This method is not-thread-safe, and should only
     * be used with a lock.
     */
    if (closed) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ALREADY_CLOSED_ERROR, "Trying to invoke an oparation on an object that was already closed");
    }
}

void
org::opensplice::core::ObjectDelegate::lock() const
{
    this->mutex.lock();
    try {
        check();
    } catch (...) {
        this->mutex.unlock();
        throw;
    }
}

void
org::opensplice::core::ObjectDelegate::unlock() const
{
    this->mutex.unlock();
}

void
org::opensplice::core::ObjectDelegate::close()
{
    this->closed = true;
}

void
org::opensplice::core::ObjectDelegate::set_weak_ref(ObjectDelegate::weak_ref_type weak_ref)
{
    this->myself = weak_ref;
}

org::opensplice::core::ObjectDelegate::weak_ref_type
org::opensplice::core::ObjectDelegate::get_weak_ref() const
{
    return this->myself;
}

org::opensplice::core::ObjectDelegate::ref_type
org::opensplice::core::ObjectDelegate::get_strong_ref() const
{
    return this->myself.lock();
}
