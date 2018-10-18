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

#ifndef ORG_OPENSPLICE_CORE_STATUS_TSTATUS_HPP_
#define ORG_OPENSPLICE_CORE_STATUS_TSTATUS_HPP_

namespace org
{
namespace opensplice
{
namespace core
{
namespace status
{


/**
 *
 */
template <typename D>
class TAllDataDisposedTopicStatus : public dds::core::Value<D>
{
public:
    TAllDataDisposedTopicStatus() : dds::core::Value<D>() { }

public:
    /**
     * @return Total cumulative count of dispose all data events on the the Topic.
     */
    int32_t total_count() const
    {
        return this->delegate().total_count();
    }

    /**
     * @return The incremental number of dispose all data events since the last time the listener
     * was called or the status was read.
     */
    int32_t total_count_change() const
    {
        return this->delegate().total_count_change();
    }
};



}
}
}
}


#endif /* ORG_OPENSPLICE_CORE_STATUS_TSTATUS_HPP_ */
