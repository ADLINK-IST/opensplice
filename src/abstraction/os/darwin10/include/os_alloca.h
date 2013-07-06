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

#include "alloca.h"

#ifdef NDEBUG

#define os_alloca(size) alloca(size)
#define os_freea(ptr)

#else

#define os_alloca(size) os_malloc(size)
#define os_freea(ptr) os_free(ptr)

#endif
