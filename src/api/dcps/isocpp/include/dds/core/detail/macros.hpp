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
/** If this macro is defined C++11 features will be assumed to be available and
 * should be used wherever possible in the API
 * @see OSPL_USE_TR1
 * @see OSPL_USE_BOOST */
#define OSPL_USE_CXX11
/** If this macro is defined Technical Report 1 features will be assumed to be available and
 * this macro will direct that they be used in the API
 * @see OSPL_USE_CXX11
 * @see OSPL_USE_BOOST */
#define OSPL_USE_TR1
/** If this macro is defined Boost will be assumed to be available and its features will be
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
#    define OMG_DDS_STATIC_ASSERT_DETAIL(condition) typedef int OSPL_MACRO_CONCAT(OMG_DDS_STATIC_ASSERT_FAILED_, __LINE__)[(condition) ? 1 : -1]
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


// End of implementation

#endif /* OSPL_DDS_CORE_DETAIL_MACROS_HPP_ */
