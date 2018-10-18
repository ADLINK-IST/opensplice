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

#include <org/opensplice/core/UserObjectDelegate.hpp>
#include <org/opensplice/core/ReportUtils.hpp>

#include "u_object.h"


org::opensplice::core::UserObjectDelegate::UserObjectDelegate() :
        userHandle(NULL)
{
}

org::opensplice::core::UserObjectDelegate::~UserObjectDelegate()
{
    if (this->userHandle) {
        u_objectFree(this->userHandle);
    }
    this->userHandle = NULL;
}

void
org::opensplice::core::UserObjectDelegate::close()
{
    if (this->userHandle != NULL) {
        u_result result = u_objectClose(this->userHandle);
        if (result != U_RESULT_ALREADY_DELETED) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(result, "Unable to successfully close object");
        }
    }
    org::opensplice::core::ObjectDelegate::close();
}

u_object
org::opensplice::core::UserObjectDelegate::get_user_handle() {
    u_object handle;

    this->lock();
    handle = this->userHandle;
    this->unlock();

    return handle;
}
