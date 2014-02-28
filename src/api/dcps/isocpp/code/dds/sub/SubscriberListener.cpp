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


#include <dds/sub/SubscriberListener.hpp>

namespace dds
{
namespace sub
{
SubscriberListener::~SubscriberListener() { }

NoOpSubscriberListener::~NoOpSubscriberListener() { }

void NoOpSubscriberListener::on_data_on_readers(dds::sub::Subscriber&) { }
}
}
