#ifndef OMG_DDS_CORE_VALUE_HPP_
#define OMG_DDS_CORE_VALUE_HPP_

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


namespace dds {
  namespace core {
    template <typename D>
    class Value;
  }
}

/**
 * This class provides the basic behavior for Value types.
 */
template <typename D>
class dds::core::Value {
protected:
  Value();
  Value(const Value& p);

public:
  template <typename ARG>
  Value(const ARG& arg);

  // -- We can't assume that the compiler supports variadic templates,
  // -- `yet. this code should be refactored to take advantage of compier that
  // -- do support variadic templates.
  template <typename ARG1, typename ARG2>
  Value(const ARG1& arg1, const ARG2& arg2);

  template <typename ARG1, typename ARG2, typename ARG3>
  Value(const ARG1& arg1, const ARG2& arg2, const ARG3& arg3);

  template <typename ARG1, typename ARG2, typename ARG3, typename ARG4>
  Value(const ARG1& arg1, const ARG2& arg2, const ARG3& arg3, const ARG4& arg4);

  template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
  Value(const ARG1& arg1, const ARG2& arg2, const ARG3& arg3, const ARG4& arg4, const ARG5& arg5);

  template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
  Value(const ARG1& arg1, const ARG2& arg2, const ARG3& arg3, const ARG4& arg4, const ARG5& arg5, const ARG6& arg6);

public:
  ~Value();

public:
  Value& operator=(const Value& other);

  bool operator==(const Value& other) const;

  bool operator !=(const Value& other) const;

public:
  /**
   * Return the delegate.
   */
  const D* operator->() const;

  /**
   * Return the delegate.
   */
  D* operator->();

  /**
   * Return the delegate.
   */
  const D& delegate() const;

  /**
   * Return the delegate.
   */
  D& delegate();

  operator D& ();

  operator const D& () const;
protected:
  D d_;
};


#endif /* OMG_DDS_CORE_VALUE_HPP_ */
