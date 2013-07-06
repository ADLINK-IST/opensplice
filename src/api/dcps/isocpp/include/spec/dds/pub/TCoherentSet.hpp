#ifndef OMG_TDDS_PUB_COHERENT_SET_HPP_
#define OMG_TDDS_PUB_COHERENT_SET_HPP_

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

#include <dds/pub/Publisher.hpp>

namespace dds {
  namespace pub {
    template <typename DELEGATE>
    class TCoherentSet;
  }
}


template <typename DELEGATE>
class dds::pub::TCoherentSet : public dds::core::Value<DELEGATE> {
public:
  /**
   * This operation requests that the application will begin a
   * coherent set of modifications using DataWriter objects
   * attached to the Publisher. The coherent set will be
   * completed by a matching call to end.  A
   * coherent set is a set of modifications that must be
   * propagated in such a way that they are interpreted at the
   * receivers' side as a consistent set of modifications; that
   * is, the receiver will only be able to access the data after
   * all the modifications in the set are available at the
   * receiver end.  A connectivity change may occur in the middle
   * of a set of coherent changes; for example, the set of
   * partitions used by the Publisher or one of its Subscribers
   * may change, a late-joining DataReader may appear on the
   * network, or a communication failure may occur. In the event
   * that such a change prevents an entity from receiving the
   * entire set of coherent changes, that entity must behave as if
   * it had received none of the set.  These calls can be
   * nested. In that case, the coherent set terminates only with
   * the last call to end.  The support for
   * coherent changes enables a publishing application to change
   * the value of several data-instances that could belong to the
   * same or different topics and have those changes be seen
   * atomically by the readers. This is useful in cases where
   * the values are inter-related. For example, if there are two
   * data instances representing the altitude and velocity
   * vector of the same aircraft and both are changed, it may be
   * useful to communicate those values in a way the reader can
   * see both together; otherwise, it may e.g., erroneously
   * interpret that the aircraft is on a collision course.
   */
  explicit TCoherentSet(const dds::pub::Publisher& pub);

public:
  /**
   * This operation terminates the coherent set initiated by the
   * constructor. If there is no matching call to the constructor,
   * the operation will raise the PreconditionNotMetError.
   */
  void end();

public:
  /**
   * This operation terminates the coherent set initiated by the
   * constructor.
   */
  ~TCoherentSet();
};


#endif /* OMG_TDDS_PUB_COHERENT_SET_HPP_ */
