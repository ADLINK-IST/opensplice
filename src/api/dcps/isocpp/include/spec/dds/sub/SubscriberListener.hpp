#ifndef OMG_DDS_SUB_SUBSCRIBER_LISTENER_HPP_
#define OMG_DDS_SUB_SUBSCRIBER_LISTENER_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/sub/AnyDataReaderListener.hpp>
#include <dds/sub/Subscriber.hpp>

namespace dds
{
namespace sub
{


class OMG_DDS_API SubscriberListener : public virtual AnyDataReaderListener
{
public:
    typedef ::dds::core::smart_ptr_traits<SubscriberListener>::ref_type ref_type;

public:
    virtual ~SubscriberListener();

public:
    virtual void on_data_on_readers(Subscriber& sub) = 0;
};

class OMG_DDS_API NoOpSubscriberListener :
    public virtual SubscriberListener,
    public virtual NoOpAnyDataReaderListener
{
public:
    virtual ~NoOpSubscriberListener();

public:
    virtual void on_data_on_readers(Subscriber& sub);
};

}
}

#endif /* OMG_DDS_SUB_SUBSCRIBER_LISTENER_HPP_ */
