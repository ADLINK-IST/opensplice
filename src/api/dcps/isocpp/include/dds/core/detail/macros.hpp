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
#ifndef OSPL_DDS_CORE_DETAIL_MACROS_HPP_
#define OSPL_DDS_CORE_DETAIL_MACROS_HPP_

/**
 * @file
 */

// Implementation

#include <iostream>
#include <string.h>

// == Constants
#define OMG_DDS_DEFAULT_STATE_BIT_COUNT_DETAIL (size_t)16
#define OMG_DDS_DEFAULT_STATUS_COUNT_DETAIL    (size_t)16
// ==========================================================================

#ifdef DOXYGEN_FOR_ISOCPP
/* The above macro is never (and must never) be defined in normal compilation. The
 below macros may be defined individually for the effect described in the documentation */
/** @internal If this macro is defined C++11 features will be assumed to be available and
 * should be used wherever possible in the API
 * @see OSPL_USE_TR1
 * @see OSPL_USE_BOOST */
#define OSPL_USE_CXX11
/** @internal If this macro is defined Technical Report 1 features will be assumed to be available and
 * this macro will direct that they be used in the API
 * @see OSPL_USE_CXX11
 * @see OSPL_USE_BOOST */
#define OSPL_USE_TR1
/** @internal If this macro is defined Boost will be assumed to be available and that its features should be
 * be used throughout the API
 * @see OSPL_USE_CXX11
 * @see OSPL_USE_TR1 */
#define OSPL_USE_BOOST
#endif

// == C++ 11 / Techical Report 1 / Boost availability
/* Determine if C++ 11 compile flag is 'on', or is usually on by default */
#if (defined __GXX_EXPERIMENTAL_CXX0X || \
       __cplusplus >= 201103L || \
       (_MSC_VER >= 1600 && _HAS_CPP0X))
# define OSPL_DEFAULT_TO_CXX11
#endif

#if (_MSC_VER >= 1500) || ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 0)))
# define OSPL_DEFAULT_TO_TR1
#endif

/* Compiling explicitly w/ C++ 11 e.g. -std=cxx0x; Studio 2010; &c... */
#if (defined(OSPL_DEFAULT_TO_CXX11) && !defined (OSPL_USE_BOOST) &&  !defined (OSPL_USE_TR1)) \
        || defined(OSPL_USE_CXX11)
#  ifndef OSPL_USE_CXX11
#    define OSPL_USE_CXX11
#  endif

/* Tech Report 1 headers are known available. Use unless we specifically ask for boost */
#elif (defined (OSPL_DEFAULT_TO_TR1) && !defined (OSPL_USE_BOOST)) \
        || defined(OSPL_USE_TR1)
#  ifndef OSPL_USE_TR1
#    define OSPL_USE_TR1
#  endif

#else /* Not C++11, no TR1, or explicit OSPL_USE_BOOST - use boost */
#  ifndef OSPL_USE_BOOST
#    define OSPL_USE_BOOST
#  endif

#endif

// == Static Assert
#if defined (OSPL_USE_CXX11) /* static_assert keyword supported */
#  define OMG_DDS_STATIC_ASSERT_DETAIL(condition) static_assert(condition, #condition)
#else
#  if defined (OSPL_USE_BOOST)
#    include <boost/static_assert.hpp>
#    define OMG_DDS_STATIC_ASSERT_DETAIL BOOST_STATIC_ASSERT
#  else /* Create a compilation error by creating a negative sized array type when condition not true */
#    define OSPL_MACRO_CONCAT_(a, b) a ## b
#    define OSPL_MACRO_CONCAT(a, b) OSPL_MACRO_CONCAT_(a, b)
#    if (__GNUC__ >= 4) /* Have seen 'attribute unused' on types in gcc 3.1 docs but we're only worried re 4+ anyway... */
#      define OSPL_GCC_ATTRIBUTE_UNUSED __attribute__ ((unused))
/* If the above addition does not silence warnings like:
 * typedef ‘OMG_DDS_STATIC_ASSERT_FAILED_nn’ locally defined but not used [-Wunused-local-typedefs]
 * then you may wish to consider also/instead uncommenting the below line.
 * @see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=54372 and OSPL-3329 */
//#      pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#    else
#      define OSPL_GCC_ATTRIBUTE_UNUSED
#    endif
#    define OMG_DDS_STATIC_ASSERT_DETAIL(condition) typedef int OSPL_MACRO_CONCAT(OMG_DDS_STATIC_ASSERT_FAILED_, __LINE__)[(condition) ? 1 : -1] OSPL_GCC_ATTRIBUTE_UNUSED
#  endif
#endif
// ==========================================================================

// DLL Export Macros
#ifdef BUILD_OMG_DDS_API
#  ifdef _WIN32 // This is defined for 32/64 bit Windows
#    define OMG_DDS_API_DETAIL __declspec(dllexport)
#  else
#    define OMG_DDS_API_DETAIL
#  endif
#else
#  ifdef _WIN32 // This is defined for 32/64 bit Windows
#    define OMG_DDS_API_DETAIL __declspec(dllimport)
#  else
#    define OMG_DDS_API_DETAIL
#  endif
#endif

// ==========================================================================

// Logging Macros
#include <dds/core/detail/maplog.hpp>
#define OMG_DDS_LOG_DETAIL(kind, msg) \
    if (dds::core::detail::maplog(kind) >= os_reportVerbosity) os_report(dds::core::detail::maplog(kind),"isocpp-OMG_DDS_LOG",__FILE__,__LINE__,0,msg)
//  std::cout << "[" << kind << "]: " << msg << std::endl;
// ==========================================================================

/**
 * @internal
 * @bug OSPL-2893 SunCC template compilation issue.
 * Partial specialization to 'hardcode' the delegate required.
 */
#ifdef __SUNPRO_CC
#  define OSPL_2893_COMPILER_BUG
/**
 * @internal
 * @bug OSPL-3323 Old SunCC has poor template support. SunCC 5.9/12.0 is known not to work.
 * SunCC 5.11/12.2 works. For now, let 5.10/12.1 try their luck...
 */
#  if (__SUNPRO_CC < 0x5100)
#    error The ISO C++ API is not supported on Solaris Studio 12.0 or earlier. Please upgrade or export INCLUDE_API_DCPS_ISOCPP=no.
#  endif
#endif

// C++ 11 features
// Slightly pathological - we could (for example) want to use boost traits
// and 'STL' implementations  but compile with -std=c++11, so set a macro for
// C++11 compile being on. This way we can always use language features
#if defined (OSPL_DEFAULT_TO_CXX11) || defined(OSPL_USE_CXX11)
#  define OSPL_DDS_CXX11
#  include <cstring>
#endif

#if !defined(OSPL_DDS_FINAL) && defined (OSPL_DDS_CXX11)
#  if defined (_MSC_VER) && ( _MSC_VER < 1700)
// http://msdn.microsoft.com/en-us/library/vstudio/hh567368.aspx
// 'Visual C++ in Visual Studio 2010 ... "final" was ... supported, but
// under the different spelling "sealed". The Standard spelling and
// semantics of "override" and "final" are now completely supported.'
#    define OSPL_DDS_FINAL sealed
#  else
#    define OSPL_DDS_FINAL final
#endif
#else
#  define OSPL_DDS_FINAL
#endif

#if defined(OSPL_DDS_CXX11)
#  if (_MSC_VER <= 1800)
// See: http://msdn.microsoft.com/en-us/library/vstudio/hh567368.aspx
// "These are now supported, but with this exception: For defaulted functions,
// the use of = default to request member-wise move constructors and move
// assignment operators is not supported."
// ('now' is currently VS 2013 - _MSC_VER == 1800).
#    define OSPL_CXX11_NO_FUNCTION_DEFAULTS
#  endif
#endif

// End of implementation

#endif /* OSPL_DDS_CORE_DETAIL_MACROS_HPP_ */
