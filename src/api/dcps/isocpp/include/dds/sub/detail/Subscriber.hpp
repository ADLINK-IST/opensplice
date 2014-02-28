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
#ifndef OSPL_DDS_SUB_DETAIL_SUBSCRIBER_HPP_
#define OSPL_DDS_SUB_DETAIL_SUBSCRIBER_HPP_

/**
 * @file
 */

// Implementation

#include <org/opensplice/sub/SubscriberDelegate.hpp>
#include <dds/sub/TSubscriber.hpp>

// #include <dds/core/status/Status.hpp>

namespace dds
{
namespace sub
{
namespace detail
{

typedef dds::sub::TSubscriber<org::opensplice::sub::SubscriberDelegate> Subscriber;

}
}
}
// End of implementation
#endif /* OSPL_DDS_SUB_DETAIL_SUBSCRIBER_HPP_ */
