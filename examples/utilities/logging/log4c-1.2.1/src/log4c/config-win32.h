/* $Id$
 *
 * See the COPYING file for the terms of usage and distribution.
 */

/* This file defines some labels as required for
   compiling with Microsoft Visual C++ 6
*/

#ifndef __log4c_config_win32_h
#define __log4c_config_win32_h

#include <time.h>
#include <windows.h>
#include <winsock.h>

#undef LOG4C_API
#ifdef LOG4C_EXPORTS
#    define LOG4C_API         __declspec(dllexport)
#else
#    define LOG4C_API       extern __declspec(dllimport)
#endif

#undef LOG4C_DATA
#ifdef LOG4C_EXPORTS
#    define LOG4C_DATA        __declspec(dllexport)
#else
#    define LOG4C_DATA       extern __declspec(dllimport)
#endif


/* This is defined to be 'inline' by default,
   but with msvc6 undef it so that inlined
   functions are just normal functions.
*/
#undef LOG4C_INLINE
#define LOG4C_INLINE


#endif /* __log4c_config_win32_h */
