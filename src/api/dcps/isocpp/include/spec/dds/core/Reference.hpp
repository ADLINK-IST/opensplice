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


namespace dds { namespace core {


  /**
   * The <code>Reference</code> class defines the abstract behavior for those
   * type defined as "reference-type" in the DDS-PSM-CXX specification.
   */
  template <typename DELEGATE>
  class Reference
  {
  public:
    DECLARE_TYPE_TRAITS(DELEGATE)

    /**
     * Creates a "null" reference.
     */
    explicit Reference(dds::core::null_type&);

  public:
    /**
     * Creates a reference from another.
     *
     * @param ref the other reference
     */
    explicit Reference(const Reference& ref);

    /**
     * Enables safe assignment from other reference types.
     *
     * @param ref the other reference
     */
    template <typename D>
    explicit Reference(const Reference<D>& ref);

    explicit Reference(DELEGATE_T* p);

  public:
    /**
     * Destroys a reference.
     */
    ~Reference();

  public:
    operator DELEGATE_REF_T() const;

    /**
     * Compares two reference objects and returns true if they are equal.
     * Equality is based on the referential equality of the object being
     * pointed.
     *
     * @param ref the other reference object
     */
    template <typename R>
    bool operator==(const R& ref) const;

    /**
     * Compares two reference objects and returns true if they are not-equal.
     * Inequality is based on the referential inequality of the object being
     * pointed.
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
     *
     * In essence this assignment operators takes care of allowing programmers
     * to write:
     *    MyRefType my_ref = //...
     *    my_ref = dds::null;
     */
    Reference&
    operator=(const null_type);

    /**
     * Returns true if this reference object is nil, meaning pointing to null.
     */
    bool is_nil() const;

    /**
     * Special <code>operator==</code> used to check if this reference object
     * equals the <code>null</code> reference.
     * The null-check can be done like this:
     * <code>
     *    bool is_null = (r == dds::null);
     * </code>
     * If r is a null reference the <code> is_null</code> variable will
     * have the <b>true</b> value.
     *
     * @return true if this reference is null.
     */
    bool
    operator==(const null_type) const;

    /**
     * Special <code>operator!=</code> used to check if this reference object
     * does not equals the <code>null</code> reference.
     * The non-null-check can be done like this:
     * <code>
     *    bool is_null = (r != dds::null);
     * </code>
     * If r is a non-null reference the <code> is_null</code> variable will
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
     * The <code>operator->()</code> is provided to be able to directly
     * invoke methods on the delegate. The choice to provide direct access to
     * the delegate was motivated by the need for providing a way that
     * was not invasive with respect to the CXXDDS API and yet would allow
     * for Vendor Specific extension.
     * Thus, vendor specific extensions can be invoked on the Reference
     * and on all its subclasses as follows:
     *
     *  <code>
     *      my_dds_entity.standard_method();
     *      my_dds_entity->vendor_specific_extension();
     *  </code>
     *
     * @return a reference to delegate.
     */
    DELEGATE* operator->();

    /**
     * The <code>operator->()</code> is provided to be able to directly
     * invoke methods on the delegate. The choice to provide direct access to
     * the delegate was motivated by the need for providing a way that
     * was not invasive with respect to the CXXDDS API and yet would allow
     * for Vendor Specific extension.
     * Thus, vendor specific extensions can be invocked on the Reference
     * and on all its subclasses as follows:
     *
     *  <code>
     *      my_dds_entity.standard_method();
     *      my_dds_entity->vendor_specific_extension();
     *  </code>
     *
     * @return a reference to delegate.
     */
    const DELEGATE* operator->() const;

    operator DELEGATE_REF_T& ();

    operator const DELEGATE_REF_T& () const;

  protected:
    DELEGATE_REF_T impl_;
  };


} } /* namespace dds / namespace core */

template <class D> bool operator == (dds::core::null_type, const dds::core::Reference<D> & r);

template <class D> bool operator != (dds::core::null_type, const dds::core::Reference<D> & r);

#endif /* OMG_DDS_CORE_REFERENCE_HPP_ */
