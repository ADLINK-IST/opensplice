#ifndef OMG_DDS_SUB_TCOHERENT_ACCESS_HPP_
#define OMG_DDS_SUB_TCOHERENT_ACCESS_HPP_

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

#include <dds/core/Value.hpp>


namespace dds {
  namespace sub {
    template <typename DELEGATE>
    class TCoherentAccess;
  }
}

template <typename DELEGATE>
class dds::sub::TCoherentAccess : public dds::core::Value<DELEGATE> {
public:
  /**
   * This operation indicates that the application is about to access
   * the data samples in any of the DataReader objects attached to the
   * Subscriber.  The application is required to use this operation
   * only if PRESENTATION QosPolicy of the Subscriber to which the
   * DataReader belongs has the access_scope set to "GROUP." In the
   * aforementioned case, the operation begin_access must be called
   * prior to calling any of the sample-accessing operations, namely:
   * data_readers on the Subscriber and read, take,
   * read_w_condition, take_w_condition on any DataReader. Otherwise
   * the sample-accessing operations will return the error
   * PRECONDITION_NOT_MET. Once the application has finished accessing
   * the data samples it must call end_access.  It is not required for
   * the application to call begin_access/end_access if the
   * PRESENTATION QosPolicy has the access_scope set to something
   * other than GROUP. Calling begin_access/end_access in this case
   * is not considered an error and has no effect.  The calls to
   * begin_access/end_access may be nested. In that case, the
   * application must call end_access as many times as it called
   * begin_access.
   */
  explicit TCoherentAccess(const dds::sub::Subscriber& sub);

public:
  /**
   * This operation indicates that the application is about to access
   * the data samples in any of the DataReader objects attached to the
   * Subscriber.  The application is required to use this operation
   * only if PRESENTATION QosPolicy of the Subscriber to which the
   * DataReader belongs has the access_scope set to GROUP. In the
   * aforementioned case, the operation begin_access must be called
   * prior to calling any of the sample-accessing operations, namely:
   * data_readers on the Subscriber and read, take,
   * read_w_condition, take_w_condition on any DataReader. Otherwise
   * the sample-accessing operations will return the error
   * PRECONDITION_NOT_MET. Once the application has finished accessing
   * the data samples it must call end_access.  It is not required for
   * the application to call begin_access/end_access if the
   * PRESENTATION QosPolicy has the access_scope set to something
   * other than ‘GROUP.’ Calling begin_access/end_access in this case
   * is not considered an error and has no effect.  The calls to
   * begin_access/end_access may be nested. In that case, the
   * application must call end_access as many times as it called
   * begin_access.
   */
  void end();         // ends access explicitly

public:
  /**
   * This operation indicates that the application is about to access
   * the data samples in any of the DataReader objects attached to the
   * Subscriber.  The application is required to use this operation
   * only if PRESENTATION QosPolicy of the Subscriber to which the
   * DataReader belongs has the access_scope set to GROUP. In the
   * aforementioned case, the operation begin_access must be called
   * prior to calling any of the sample-accessing operations, namely:
   * data_readers on the Subscriber and read, take,
   * read_w_condition, take_w_condition on any DataReader. Otherwise
   * the sample-accessing operations will return the error
   * PRECONDITION_NOT_MET. Once the application has finished accessing
   * the data samples it must call end_access.  It is not required for
   * the application to call begin_access/end_access if the
   * PRESENTATION QosPolicy has the access_scope set to something
   * other than GROUP. Calling begin_access/end_access in this case
   * is not considered an error and has no effect.  The calls to
   * begin_access/end_access may be nested. In that case, the
   * application must call end_access as many times as it called
   * begin_access.
   */
  ~TCoherentAccess();  // ends access implicitly

private:
  TCoherentAccess(const TCoherentAccess&);
  TCoherentAccess& operator=(const TCoherentAccess&);
};


#endif /* OMG_TDDS_SUB_TCOHERENT_ACCESS_HPP_ */
