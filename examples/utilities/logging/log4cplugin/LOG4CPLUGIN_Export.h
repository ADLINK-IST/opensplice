/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
/* $Id$ */
/* Definition for Win32/GCC Export directives.
 * This file is generated automatically by generate_export_file.pl -n LIB4CPLUGIN
 * ------------------------------ */

#ifndef LIB4CPLUGIN_EXPORT_H
#define LIB4CPLUGIN_EXPORT_H

#include /* $(OSPL_HOME)/include/sys */ "os_if.h"

#if !defined (LIB4CPLUGIN_HAS_DLL)
#  define LIB4CPLUGIN_HAS_DLL 1
#endif /* ! LIB4CPLUGIN_HAS_DLL */

#if defined (LIB4CPLUGIN_HAS_DLL) && (LIB4CPLUGIN_HAS_DLL == 1)
#  if defined (LOG4CPLUGIN_BUILD_DLL)
#    define LIB4CPLUGIN_Export OS_API_EXPORT
#  else /* ! LIB4CPLUGIN_BUILD_DLL */
#    define LIB4CPLUGIN_Export OS_API_IMPORT
#  endif /* ! LIB4CPLUGIN_BUILD_DLL */
#else /* ! LIB4CPLUGIN_HAS_DLL == 1 */
#  define LIB4CPLUGIN_Export
#endif /* ! LIB4CPLUGIN_HAS_DLL == 1 */

#endif /* LIB4CPLUGIN_EXPORT_H */

// End of auto generated file.
