/* $Id$
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef __log4c_defs_h
#define __log4c_defs_h

/**
 * @file defs.h
 *
 * @brief types and declarations enclosures for C++.
 *
 **/

#ifdef  __cplusplus
# define __LOG4C_BEGIN_DECLS  extern "C" {
# define __LOG4C_END_DECLS    }
#else
# define __LOG4C_BEGIN_DECLS
# define __LOG4C_END_DECLS
#endif

#ifdef __SUNPRO_C
#undef LOG4C_INLINE
#define LOG4C_INLINE
#else
#define LOG4C_INLINE inline
#endif
#define LOG4C_API    extern
#define LOG4C_DATA    extern

#ifdef __HP_cc
#define inline __inline
#endif

#ifdef _WIN32
# include <log4c/config-win32.h>
#endif

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#endif /* GCC_VERSION */

#if GCC_VERSION < 2009
#define OLD_VARIADIC_MACRO 1
#endif

#endif /* __log4c_defs_h */
