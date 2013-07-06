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

#include <dds/topic/AnyTopicListener.hpp>

namespace dds
{
namespace topic
{

AnyTopicListener::~AnyTopicListener() { }

NoOpAnyTopicListener::~NoOpAnyTopicListener() { }

void NoOpAnyTopicListener::on_inconsistent_topic(
    AnyTopic&,
    const dds::core::status::InconsistentTopicStatus&) { }

}
}
