#ifndef OSPL_TESTLIBS_API_EXPORT_H
#define OSPL_TESTLIBS_API_EXPORT_H

#if !defined(OSPL_TESTLIBS_API_HAS_DLL)
# if defined(OSPL_TESTLIBS_API_AS_STATIC_LIBS)
#   define OSPL_TESTLIBS_API_HAS_DLL 0
# else
#   define OSPL_TESTLIBS_API_HAS_DLL 1
# endif
#endif

#if (OSPL_TESTLIBS_API_HAS_DLL == 1)
#  if defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x550)
#    if defined(OSPL_TESTLIBS_API_BUILD_DLL)
#      define OSPL_TESTLIBS_API_Export __symbolic
#    else
#      define OSPL_TESTLIBS_API_Export __global
#    endif
#  elif defined(WIN32) || defined(UNDER_CE) || defined(__CYGWIN__)
#    if defined(OSPL_TESTLIBS_API_BUILD_DLL)
#      define OSPL_TESTLIBS_API_Export __declspec(dllexport)
#    else
#      define OSPL_TESTLIBS_API_Export __declspec(dllimport)
#    endif
#  elif (defined(__GNUC__) && (__GNUC__ >= 4))
#    if defined(OSPL_TESTLIBS_API_BUILD_DLL)
#      define OSPL_TESTLIBS_API_Export __attribute__((visibility("default")))
#    else
#      define OSPL_TESTLIBS_API_Export
#    endif
#  else
#    define OSPL_TESTLIBS_API_Export
#  endif
#else
#  define OSPL_TESTLIBS_API_Export
#endif

#endif
