#ifndef OMG_DDS_CORE_REFERENCE_HPP_
#define OMG_DDS_CORE_REFERENCE_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
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

#include <dds/core/types.hpp>
#include <dds/core/refmacros.hpp>
#include <dds/core/Exception.hpp>


namespace dds
{
namespace core
{


/**
 * All objects that have a reference-type have an associated shallow (polymorphic)
 * assignment operator that simply changes the value of the reference.
 * Furthermore, reference-types are safe, meaning that under no circumstances can
 * a reference point to an invalid object.
 * At any single point in time a reference can either refer to the null object or
 * to a valid object.
 *
 * The semantics for Reference types is defined by the DDS-PSM-Cxx class
 * dds::core::Reference. In the context of this specification the semantics implied
 * by the ReferenceType is mandatory, yet the implementation supplied as part of
 * this standard is provided to show one possible way of implementing this semantics.
 *
 * List of reference types:
 *
 * * Entity
 * * Condition
 * * GuardCondition
 * * ReadCondition
 * * QueryCondition
 * * Waitset
 * * DomainParticipant
 * * AnyDataWriter
 * * Publisher
 * * DataWriter
 * * AnyDataReader
 * * Subscriber
 * * DataReader
 * * SharedSamples
 * * AnyTopic
 * * Topic
 *
 * Instances of reference types are created using C++ constructors.
 * The trivial constructor is not defined for reference types; the only
 * alternative to properly constructing a reference is to initialize it to a
 * null reference by assigning dds::core::null.
 *
 * Resource management for some reference types might involve relatively
 * heavyweight operating-system resources (such as threads, mutexes,
 * and network sockets) in addition to memory.
 * These objects therefore provide a function close() that shall halt network
 * communication (in the case of entities) and dispose of any appropriate
 * operating-system resources.
 *
 * Users of this PSM are recommended to call close on objects of all reference
 * types once they are finished using them. In addition, implementations may
 * automatically close objects that they deem to be no longer in use,
 * subject to the following restrictions:
 *
 * * Any object to which the application has a direct reference
 *   (not including a WeakReference) is still in use.
 *
 * * Any entity with a non-null listener is still in use.
 *
 * * Any object that has been explicitly retained is still in use.
 *
 * * The creator of any object that is still in use is itself still in use.
 *
 */
template <typename DELEGATE>
class Reference
{
public:
    DECLARE_TYPE_TRAITS(DELEGATE)

    /**
     * Creates a "null" Reference.
     */
    explicit Reference(dds::core::null_type&);

public:
    /**
     * Creates a Reference from another.
     *
     * @param ref the other reference
     */
    explicit Reference(const Reference& ref);

    /**
     * Enables safe assignment from other Reference types.
     *
     * @param ref the other reference
     */
    template <typename D>
    explicit Reference(const Reference<D>& ref);

    explicit Reference(DELEGATE_T* p);

    explicit Reference(const DELEGATE_REF_T& p);

public:
    /**
     * Destroys a Reference.
     */
    ~Reference();

public:
    operator DELEGATE_REF_T() const;

    /**
     * Compares two Reference objects and returns true if they are equal.
     * Equality is based on the referential equality of the object being
     * pointed.
     *
     * @param ref the other Reference object
     */
    template <typename R>
    bool operator==(const R& ref) const;

    /**
     * Compares two Reference objects and returns true if they are not
     * equal.
     *
     * Inequality is based on the referential inequality of the object
	  * being pointed to.
     *
     * @param ref the other reference object
     */
    template <typename R>
    bool operator!=(const R& ref) const;

    template <typename D>
    Reference& operator=(const Reference<D>& that);

    template <typename R>
    Reference& operator=(const R& rhs);

    /**
     * Special assignment operators that takes care of assigning <i>null</i>
     * to this reference. When assigning null, there might be an associated
     * garbage collection activity.
     */
    Reference&
    operator=(const null_type);

    /**
     * Returns true if this reference object is nil, meaning pointing to null.
     */
    bool is_nil() const;

    /**
     * Special operator== used to check if this reference object
     * equals the null reference.
     * The null-check can be done like this:
     * @return true if this reference is null.
     */
    bool
    operator==(const null_type) const;

    /**
     * Special operator!= used to check if this reference object
     * does not equal the null reference.
     * The non-null-check can be done like this:
     *
     *    bool is_null = (r != dds::null);
     *
     * If r is a non-null reference the is_null variable will
     * have the <b>false</b> value.
     *
     * @return true if this reference is null.
     */
    bool operator!=(const null_type nil) const;

private:
    // -- disallow dynamic allocation for reference types
    void* operator new(size_t);



public:
    /**
     * Returns a reference to the underlying delegate. This can be used
     * to invoke non-standard extensions provided by the DDS implementor.
     *
     * @return a reference to delegate.
     */
    const DELEGATE_REF_T& delegate() const;

    /**
     * Returns a reference to the underlying delegate. This can be used
     * to invoke non-standard extensions provided by the DDS implementor.
     *
     * @return a reference to delegate.
     */
    DELEGATE_REF_T& delegate();

    /**
     * The operator->() is provided to be able to directly invoke
     * functions on the delegate. The decision to provide direct access to
     * the delegate was motivated by the need for providing a way that
     * was not invasive with respect to the CXXDDS API and yet would allow
     * for vendor-specific extension.
     * Thus vendor-specific extensions can be invoked on the Reference
     * and on all its subclasses as follows:
     *
     *
     *      my_dds_entity.standard_function();
     *      my_dds_entity->vendor_specific_extension();
     *
     *
     * @return a reference to delegate.
     */
    DELEGATE* operator->();

    /**
     * The operator->() is provided to be able to directly invoke
     * functions on the delegate. The decision to provide direct access to
     * the delegate was motivated by the need for providing a way that
     * was not invasive with respect to the CXXDDS API and yet would allow
     * for vendor-specific extension.
     * Thus vendor-specific extensions can be invoked on the Reference
     * and on all its subclasses as follows:
     *
     *
     *      my_dds_entity.standard_function();
     *      my_dds_entity->vendor_specific_extension();
     *
     *
     * @return a reference to delegate.
     */
    const DELEGATE* operator->() const;

    operator DELEGATE_REF_T& ();

    operator const DELEGATE_REF_T& () const;

protected:
    DELEGATE_REF_T impl_;
};


}
} /* namespace dds / namespace core */

template <class D> bool operator == (dds::core::null_type, const dds::core::Reference<D>& r);

template <class D> bool operator != (dds::core::null_type, const dds::core::Reference<D>& r);

#endif /* OMG_DDS_CORE_REFERENCE_HPP_ */
