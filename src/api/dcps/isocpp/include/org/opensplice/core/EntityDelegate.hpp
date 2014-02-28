/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
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
    DDS::Entity_var entity_;
};

}
}
}

#endif /* ORG_OPENSPLICE_CORE_ENTITY_DELEGATE_HPP_ */
