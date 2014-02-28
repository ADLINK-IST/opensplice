/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef OSPL_DDS_CORE_DETAIL_REF_TRAITS_HPP_
#define OSPL_DDS_CORE_DETAIL_REF_TRAITS_HPP_

/**
 * @file
 */

// Implementation

#include <dds/core/types.hpp>
#include <dds/core/Exception.hpp>

/* Compiling explicitly w/ C++ 11 support */
#if defined(OSPL_USE_CXX11)
#  include <memory>
#  include <type_traits>
#  include <array>
#  define OSPL_CXX11_STD_MODULE ::std

/* Compiling to use Tech Report 1 headers */
#elif defined(OSPL_USE_TR1)
#  ifdef _MSC_VER
#    include <memory>
#    include <type_traits>
#    include <array>
#  else
#    include <tr1/memory>
#    include <tr1/type_traits>
#    include <tr1/array>
#  endif
#  define OSPL_CXX11_STD_MODULE ::std::tr1

/* Compiling with boost */
#elif defined(OSPL_USE_BOOST)
#  include <boost/shared_ptr.hpp>
#  include <boost/weak_ptr.hpp>
#  include <boost/type_traits.hpp>
#  include <boost/array.hpp>
#  define OSPL_CXX11_STD_MODULE ::boost

#else
#  error "macros.hpp header not included or other unexpected misconfiguration"
#endif

template <typename T1, typename T2>
struct dds::core::is_base_of :
#if defined (OSPL_USE_BOOST) && ! defined (BOOST_TT_IS_BASE_OF_HPP_INCLUDED)
#   include "boost/type_traits/is_base_and_derived.hpp"
        ::boost::detail::is_base_and_derived_impl<T1, T2> { }; // Really old boost had no is_base_of
#else
    public OSPL_CXX11_STD_MODULE::is_base_of<T1, T2> { };
#endif

template <typename T1, typename T2>
struct dds::core::is_same : public OSPL_CXX11_STD_MODULE::is_same<T1, T1> { };

template <typename T>
struct dds::core::smart_ptr_traits
{
    typedef OSPL_CXX11_STD_MODULE::shared_ptr<T> ref_type;
    typedef OSPL_CXX11_STD_MODULE::weak_ptr<T>   weak_ref_type;
};

template <typename TO, typename FROM>
TO dds::core::polymorphic_cast(FROM& from)
{
    typename TO::DELEGATE_REF_T dr =
        OSPL_CXX11_STD_MODULE::dynamic_pointer_cast< typename TO::DELEGATE_T>(from.delegate());
    TO to(dr);

    if(to == dds::core::null)
    {
        throw dds::core::InvalidDowncastError("Attempted invalid downcast.");
    }
    return to;
}

/* We need a 'std' array. TIMTOWDI */
#if !defined (OSPL_VANILLA_USING_TEMPLATE_IMPORT_BUST)
namespace dds
{
namespace core
{
/* We're not renaming, or instantiating, so a vanilla using
 * should surely suffice. We don't need C++11 so this is the
 * default always unless: OSPL_VANILLA_USING_TEMPLATE_IMPORT_BUST
 * is defined */
using OSPL_CXX11_STD_MODULE::array;
}
}
#elif defined (OSPL_DDS_CXX11)
/* C++11 template using syntax available. Go team! */
namespace dds
{
namespace core
{
template <class T, std::size_t N > using array = OSPL_CXX11_STD_MODULE::array<T, N>;
}
}
#elif !defined(OSPL_DONT_INHERIT_ARRAY)
/* Workaround - this is not usually a GoodThing at all. AFAIK it
 * should cause no issues as all the ::array impls I've looked at
 * have no destructor to worry about. If I'm wrong you have a get
 * out in: OSPL_DONT_INHERIT_ARRAY */
namespace dds
{
namespace core
{
template <class T, std::size_t N > struct array : public OSPL_CXX11_STD_MODULE::array<T, N> {};
}
}
#else
/* The below code was "lifted", to put it politely, from the GCC implementation
 * at some unknown point in time and included in the 'spec'.
 * It will never be used unless all else fails and the OSPL_DONT_INHERIT_ARRAY
 * is defined by you the user. */
#include <dds/core/array.hpp>
#endif

// End of implementation

#endif /* OSPL_DDS_CORE_DETAIL_REF_TRAITS_HPP_ */
