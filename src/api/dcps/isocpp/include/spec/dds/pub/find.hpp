#ifndef OMG_DDS_PUB_FIND_HPP_
#define OMG_DDS_PUB_FIND_HPP_

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

#include <string>

/** @todo This does not compile - no such example file (in spec). Need to add one */
#include <dds/pub/detail/find.hpp>

namespace dds { namespace pub {

  /**
   * This function retrieves a previously-created DataWriter
   * belonging to the Publisher that is attached to a Topic with a
   * matching topic_name. If no such DataWriter exists, the operation
   * will return an empty container.  The use of this
   * operation on the built-in Publisher allows access to the
   * built-in DataWriter entities for the built-in topics
   *
   * @return the total number of elements found. Notice that
   * at most max_size will be copied using the provided iterator.
   *
   */
  template <typename WRITER, typename FwdIterator>
  uint32_t
  find(const dds::pub::Publisher& pub, const std::string& topic_name,
      FwdIterator begin, uint32_t max_size);

  /**
   * This function retrieves a previously-created DataWriter
   * belonging to the Publisher that is attached to a Topic with a
   * matching topic_name. If no such DataWriter exists, the operation
   * will return an empty container.  The use of this
   * operation on the built-in Publisher allows access to the
   * built-in DataWriter entities for the built-in topics
   *
   * @return the total number of elements found. Notice that
   * at most max_size will be copied using the provided iterator.
   *
   */
  template <typename WRITER, typename BinIterator>
  uint32_t
  find(const dds::pub::Publisher& pub, const std::string& topic_name,
      BinIterator begin);


} }

#endif /* OMG_DDS_PUB_FIND_HPP_ */
