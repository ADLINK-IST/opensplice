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
    const dds::pub::Publisher &pub,
    dds::pub::PublisherListener *listener) :
    listener_(listener)
{
    pub_ = dds::core::WeakReference<dds::pub::Publisher>(pub);
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
