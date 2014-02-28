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

#include <dds/pub/PublisherListener.hpp>
#include <org/opensplice/pub/PublisherEventForwarder.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{

template<>
PublisherEventForwarder<dds::pub::Publisher>::PublisherEventForwarder(
    const dds::pub::Publisher& pub,
    dds::pub::PublisherListener* listener) :
    pub_(pub),
    listener_(listener)

{
}

template<>
PublisherEventForwarder<dds::pub::Publisher>::~PublisherEventForwarder()
{
}

template<>
dds::pub::PublisherListener*
PublisherEventForwarder<dds::pub::Publisher>::listener()
{
    return listener_;
}

}
}
}
