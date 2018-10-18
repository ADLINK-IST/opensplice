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

#ifndef ORG_OPENSPLICE_CORE_ENTITY_DELEGATE_HPP_
#define ORG_OPENSPLICE_CORE_ENTITY_DELEGATE_HPP_

#include <org/opensplice/core/config.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <dds/core/status/State.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

class OSPL_ISOCPP_IMPL_API EntityDelegate
{
public:
    EntityDelegate();
    virtual ~EntityDelegate();

public:
    /** @internal @todo This operator not implemented so should we remove ? Or make PV ?
    Doesn't seem to be presetn on at least some subclasses
    EntityDelegate& operator=(const EntityDelegate& other); */

public:
    /**
     *  @internal Enables this entity.
     */
    virtual void enable();

    virtual const ::dds::core::status::StatusMask status_changes();

    virtual const ::dds::core::InstanceHandle instance_handle() const;

    virtual void close();

    virtual void retain();

    virtual DDS::Entity_ptr get_dds_entity();

protected:
    static volatile unsigned int entityID_;
    bool enabled_;
public:
    DDS::Entity_var entity_;
};

}
}
}

#endif /* ORG_OPENSPLICE_CORE_ENTITY_DELEGATE_HPP_ */
