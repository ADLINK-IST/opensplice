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


namespace dds
{
namespace core
{
template <typename D>
class Value;
}
}

/**
 * All objects that have a value-type have a deep-copy assignment and copy
 * construction semantics.
 * It should also be pointed out that value-types are not 'pure-value-types' in
 * the sense that they are immutable (as in functional programming languages).
 *
 * The DDS-PSM-Cxx makes value-types mutable to limit the number of copies as well
 * as to limit the time-overhead necessary to change a value-type
 * (note that for immutable value-types the only form of change is to create a new
 * value-type).
 *
 * QoS, Policy, Statuses, and Topic samples are all modeled as value-types.
 *
 */

template <typename D>
class dds::core::Value
{
protected:
    Value();
    Value(const Value& p);

public:
    /**
     * Create a Value of one value
     * @param arg VALUETYPE value
     */
    template <typename ARG>
    Value(const ARG& arg);

    /**
     * Create a Value of two values
     * @param arg1 VALUETYPES value
     * @param arg2 VALUETYPES value
     */
    template <typename ARG1, typename ARG2>
    Value(const ARG1& arg1, const ARG2& arg2);

    /**
     * Create a Value of three values
     * @param arg1 VALUETYPES value
     * @param arg2 VALUETYPES value
     * @param arg3 VALUETYPES value
     */
    template <typename ARG1, typename ARG2, typename ARG3>
    Value(const ARG1& arg1, const ARG2& arg2, const ARG3& arg3);

    /**
     * Create a Value of four values
     * @param arg1 VALUETYPES value
     * @param arg2 VALUETYPES value
     * @param arg3 VALUETYPES value
     * @param arg4 VALUETYPES value
     */
    template <typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    Value(const ARG1& arg1, const ARG2& arg2, const ARG3& arg3, const ARG4& arg4);

    /**
     * Create a Value of five values
     * @param arg1 VALUETYPES value
     * @param arg2 VALUETYPES value
     * @param arg3 VALUETYPES value
     * @param arg4 VALUETYPES value
     * @param arg5 VALUETYPES value
     */
    template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
    Value(const ARG1& arg1, const ARG2& arg2, const ARG3& arg3, const ARG4& arg4, const ARG5& arg5);

    /**
     * Create a Value of six values
     * @param arg1 VALUETYPES value
     * @param arg2 VALUETYPES value
     * @param arg3 VALUETYPES value
     * @param arg4 VALUETYPES value
     * @param arg5 VALUETYPES value
     * @param arg6 VALUETYPES value
     */
    template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
    Value(const ARG1& arg1, const ARG2& arg2, const ARG3& arg3, const ARG4& arg4, const ARG5& arg5, const ARG6& arg6);

public:
    ~Value();

public:
    /**
     * Assigns new delegate to this Value
     * @param other Value
     */
    Value& operator=(const Value& other);

    /**
     * Compare this Value with another Value
     *
     * @param other Value
     * @return true if equal
     */
    bool operator==(const Value& other) const;

    /**
     * Compare this Value with another Value
     *
     * @param other Value
     * @return true if not equal
     */
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

    /**
     * Return the delegate.
     */
    operator D& ();

    /**
     * Return the delegate.
     */
    operator const D& () const;
protected:
    D d_;
};


#endif /* OMG_DDS_CORE_VALUE_HPP_ */
