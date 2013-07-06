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
#include "os_if.h"

#ifdef OS_EXAMPLE_API
#undef OS_EXAMPLE_API
#endif

#ifdef OSPL_BUILDEXAMPLE_LIB
#define OS_EXAMPLE_API OS_API_EXPORT
#else
#define OS_EXAMPLE_API OS_API_IMPORT
#endif

/* Unlikely to need more than two levels of linked lib in an example */

#ifdef OS_EXAMPLE_API2
#undef OS_EXAMPLE_API2
#endif

#ifdef OSPL_BUILDEXAMPLE_LIB2
#define OS_EXAMPLE_API2 OS_API_EXPORT
#else
#define OS_EXAMPLE_API2 OS_API_IMPORT
#endif
