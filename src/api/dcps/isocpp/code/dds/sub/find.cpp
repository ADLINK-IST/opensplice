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


#include <dds/sub/find.hpp>

namespace dds
{
namespace sub
{

const dds::sub::Subscriber builtin_subscriber(const dds::domain::DomainParticipant& dp)
{
    DDS::Subscriber_ptr key = dp->dp_->get_builtin_subscriber();
    dds::sub::Subscriber sub = org::opensplice::core::EntityRegistry<DDS::Subscriber_ptr, dds::sub::Subscriber>::get(key);
    if(sub == dds::core::null)
    {
        sub = dds::sub::Subscriber(dp);
        org::opensplice::core::EntityRegistry<DDS::Subscriber_ptr, dds::sub::Subscriber>::remove(sub->sub_.get());
        sub->init_builtin(key);
        sub.retain();
        org::opensplice::core::EntityRegistry<DDS::Subscriber_ptr, dds::sub::Subscriber>::insert(key, sub);
    }

    return sub;
}

}
}
