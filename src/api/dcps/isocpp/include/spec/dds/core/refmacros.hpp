#ifndef OMG_DDS_CORE_REFMACROS_HPP_
#define OMG_DDS_CORE_REFMACROS_HPP_

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

#include <dds/core/macros.hpp>
#include <dds/core/ref_traits.hpp>

////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////

#define DECLARE_TYPE_TRAITS(TYPE) \
    typedef TYPE  TYPE##_T;\
    typedef typename ::dds::core::smart_ptr_traits<TYPE>::ref_type       TYPE##_REF_T;\
    typedef typename ::dds::core::smart_ptr_traits<TYPE>::weak_ref_type  TYPE##_WEAK_REF_T;


/*
 * This macro defines all the functions that Reference Types have to implement.
 */
/*
 *
 *
 */
////////////////////////////////////////////////////////////////////////////////
// Defines all the types, functions and attributes required for a Reference type
// without default ctor.
//

#define OMG_DDS_REF_TYPE_BASE(TYPE, BASE, DELEGATE)     \
    public: \
    typedef BASE< DELEGATE >                                                  BASE_T;\
    protected:\
    explicit TYPE(DELEGATE_T* p)\
        : BASE< DELEGATE_T >(p) \
    {  }\
    public:\
    template <typename H__> \
    TYPE(const H__& h)    \
        : BASE< DELEGATE_T >(h) \
    { \
        OMG_DDS_STATIC_ASSERT((::dds::core::is_base_of<typename TYPE::DELEGATE_T, typename H__::DELEGATE_T>::value)); \
        this->::dds::core::Reference<DELEGATE>::impl_ = h.delegate(); \
    }

#define OMG_DDS_REF_TYPE_BASE_T(TYPE, BASE, T_PARAM, DELEGATE) \
    public: \
    typedef BASE< T_PARAM, DELEGATE > BASE_T; \
    protected: \
    explicit TYPE(DELEGATE_T* p) \
        : BASE< T_PARAM, DELEGATE >(p) \
    {  } \
    public: \
    template <typename H__> \
    TYPE(const H__& h) \
        : BASE< T_PARAM, DELEGATE >(h) \
    { \
        OMG_DDS_STATIC_ASSERT((::dds::core::is_base_of<typename TYPE::DELEGATE_T, typename H__::DELEGATE_T>::value)); \
        this->::dds::core::Reference< DELEGATE< T_PARAM > >::impl_ = h.delegate(); \
    }


#define OMG_DDS_REF_TYPE_DELEGATE(TYPE, BASE, DELEGATE)     \
    public: \
    typedef DELEGATE                                                          DELEGATE_T;   \
    typedef typename ::dds::core::smart_ptr_traits< DELEGATE >::ref_type      DELEGATE_REF_T; \
    typedef typename ::dds::core::smart_ptr_traits< DELEGATE >::weak_ref_type DELEGATE_WEAK_REF_T; \
    \
    private:\
    const typename ::dds::core::Reference< DELEGATE >::DELEGATE_REF_T& impl() const \
    { return ::dds::core::Reference< DELEGATE >::impl_; }\
    typename ::dds::core::Reference< DELEGATE >::DELEGATE_REF_T& impl() \
    { return ::dds::core::Reference< DELEGATE >::impl_; }\
    \
    public:\
    template <typename T__> \
    TYPE& \
    operator=(const T__& rhs) {\
        OMG_DDS_STATIC_ASSERT((::dds::core::is_base_of<typename TYPE::DELEGATE_T, typename T__::DELEGATE_T>::value));\
        if (this != (TYPE*)&rhs) \
            *this = TYPE(rhs); \
        return *this; \
    }


#define OMG_DDS_REF_TYPE_DELEGATE_T(TYPE, BASE, T_PARAM, DELEGATE)     \
    public: \
    typedef DELEGATE< T_PARAM >                                               DELEGATE_T;   \
    typedef typename ::dds::core::smart_ptr_traits< DELEGATE< T_PARAM > >::ref_type \
    DELEGATE_REF_T; \
    typedef typename ::dds::core::smart_ptr_traits< DELEGATE< T_PARAM > >::weak_ref_type \
    DELEGATE_WEAK_REF_T; \
    \
    private:\
    const typename ::dds::core::Reference< DELEGATE< T_PARAM > >::DELEGATE_REF_T& impl() const \
    { return ::dds::core::Reference< DELEGATE< T_PARAM > >::impl_; }\
    typename ::dds::core::Reference< DELEGATE< T_PARAM > >::DELEGATE_REF_T& impl() \
    { return ::dds::core::Reference< DELEGATE< T_PARAM > >::impl_; }\
    \
    public:\
    template <typename T__> \
    TYPE& \
    operator=(const T__& rhs) {\
        OMG_DDS_STATIC_ASSERT((::dds::core::is_base_of<typename TYPE::DELEGATE_T, typename T__::DELEGATE_T>::value));\
        if (this != (TYPE*)&rhs) \
            *this = TYPE(rhs); \
        return *this; \
    }


#define OMG_DDS_REF_TYPE_NODC(TYPE, BASE, DELEGATE) \
    OMG_DDS_REF_TYPE_DELEGATE(TYPE, BASE, DELEGATE)     \
    OMG_DDS_REF_TYPE_BASE(TYPE, BASE, DELEGATE)         \
    public:\
    explicit TYPE(const DELEGATE_REF_T& ref) \
        : BASE<DELEGATE_T>(ref)\
    { }

#define OMG_DDS_REF_TYPE_NODC_T(TYPE, BASE, T_PARAM, DELEGATE) \
    OMG_DDS_REF_TYPE_DELEGATE_T(TYPE, BASE, T_PARAM, DELEGATE)     \
    OMG_DDS_REF_TYPE_BASE_T(TYPE, BASE, T_PARAM, DELEGATE)         \
    public:\
    explicit TYPE(const DELEGATE_REF_T& ref) \
        : BASE<T, DELEGATE>(ref)\
    { }

#define DDS_PTR_CTOR_REF_TYPE(TYPE, BASE, DELEGATE)     \
    public: \
    typedef BASE< DELEGATE >                                                  BASE_T;\
    typedef DELEGATE                                                          DELEGATE_T;   \
    typedef typename ::dds::core::smart_ptr_traits< DELEGATE >::ref_type      DELEGATE_REF_T; \
    typedef typename ::dds::core::smart_ptr_traits< DELEGATE >::weak_ref_type DELEGATE_WEAK_REF_T; \
    \
    private:\
    typename ::dds::core::Reference< DELEGATE >::DELEGATE_REF_T& impl() \
    { return ::dds::core::Reference< DELEGATE >::impl_; }\
    public:\
    explicit TYPE(DELEGATE_T* p)\
        : BASE< DELEGATE_T >(p) \
    {  }\
    template <typename PTR>\
    explicit TYPE(PTR* ptr) {\
        OMG_DDS_STATIC_ASSERT((::dds::core::is_base_of<typename TYPE::DELEGATE_T, PTR>::value)); \
        this->::dds::core::Reference< DELEGATE >::impl_.reset(ptr);\
    }\
    public:\
    explicit TYPE(const DELEGATE_REF_T& ref) \
        : BASE<DELEGATE_T>(ref)\
    { }\
    \
    template <typename H__> \
    explicit TYPE(const H__& h)    \
        : BASE< DELEGATE_T >(h) \
    { \
        OMG_DDS_STATIC_ASSERT((::dds::core::is_base_of<typename TYPE::DELEGATE_T, typename H__::DELEGATE_T>::value)); \
        this->::dds::core::Reference<DELEGATE>::impl_ = h.delegate();\
    } \
    public:\
    template <typename T__> \
    TYPE& \
    operator=(const T__& rhs) {\
        OMG_DDS_STATIC_ASSERT((::dds::core::is_base_of<typename TYPE::DELEGATE_T, typename T__::DELEGATE_T>::value));\
        if (this != (TYPE*)&rhs) \
            *this = TYPE(rhs); \
        return *this; \
    } \
    public:


////////////////////////////////////////////////////////////////////////////////
// Declares a reference type equipped with a default ctor.
//
#define OMG_DDS_REF_TYPE(TYPE, BASE, DELEGATE)      \
    OMG_DDS_REF_TYPE_NODC(TYPE, BASE, DELEGATE) \
    private: \
    TYPE(); \
    public: \
    TYPE(const dds::core::null_type&) : BASE< DELEGATE >(static_cast<DELEGATE*>(NULL)) { }

#define OMG_DDS_REF_TYPE_PUBLIC(TYPE, BASE, DELEGATE)      \
    OMG_DDS_REF_TYPE_NODC(TYPE, BASE, DELEGATE) \
    public: \
    TYPE() : BASE< DELEGATE >(new DELEGATE()) { } \
    TYPE(const dds::core::null_type&) : BASE< DELEGATE >(static_cast<DELEGATE*>(NULL)) { }

#define OMG_DDS_REF_TYPE_T(TYPE, BASE, T_PARAM, DELEGATE) \
    OMG_DDS_REF_TYPE_NODC_T(TYPE, BASE, T_PARAM, DELEGATE) \
    private: \
    TYPE(); \
    public: \
    TYPE(const dds::core::null_type&) : BASE<T_PARAM, DELEGATE>(static_cast<DELEGATE<T_PARAM>* >(NULL)) { }

#define OMG_DDS_BASIC_REF_TYPE(TYPE, BASE, DELEGATE)      \
    private: \
    TYPE(); \
    public: \
    typedef DELEGATE                                                          DELEGATE_T;   \
    typedef typename ::dds::core::smart_ptr_traits< DELEGATE >::ref_type      DELEGATE_REF_T; \
    typedef typename ::dds::core::smart_ptr_traits< DELEGATE >::weak_ref_type DELEGATE_WEAK_REF_T; \
    TYPE(const dds::core::null_type&) : BASE< DELEGATE >(static_cast<DELEGATE*>(NULL)) { } \
    explicit TYPE(const DELEGATE_REF_T& ref) \
        : BASE<DELEGATE_T>(ref)\
    { }

#endif /* OMG_DDS_CORE_REFMACROS_HPP_ */
