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

#include <sstream>

#include <dds/core/Exception.hpp>
#include <dds/core/InstanceHandle.hpp>

#include <org/opensplice/core/EntityDelegate.hpp>


volatile unsigned int org::opensplice::core::EntityDelegate::entityID_ = 0;

org::opensplice::core::EntityDelegate::EntityDelegate()
: enabled_(true)
{ }

org::opensplice::core::EntityDelegate::~EntityDelegate()
{ }

/** @todo Implementation required - OSPL-2640, currently autoenabled
 * see http://jira.prismtech.com:8080/browse/OSPL-2640 **/
void
org::opensplice::core::EntityDelegate::enable() {
   // implementation-defined
   //entity_.in()->enable();
   //enabled_ = true;
}

const dds::core::status::StatusMask
org::opensplice::core::EntityDelegate::status_changes() {
    return dds::core::status::StatusMask(entity_.in()->get_status_changes());
}

const dds::core::InstanceHandle
org::opensplice::core::EntityDelegate::instance_handle() const {
    return dds::core::InstanceHandle(entity_.in()->get_instance_handle());
}

/** @todo Implementation required - OSPL-2640, currently autoenabled
 * see http://jira.prismtech.com:8080/browse/OSPL-2640 **/
void
org::opensplice::core::EntityDelegate::close() {  }

void
org::opensplice::core::EntityDelegate::retain() {  }

DDS::Entity_ptr org::opensplice::core::EntityDelegate::get_dds_entity()
{
    return entity_.in();
}
