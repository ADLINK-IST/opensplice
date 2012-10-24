/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef OS_WIN32_ALLOCA_H
#define OS_WIN32_ALLOCA_H

#define os_alloca(size) os_malloc(size)
#define os_freea(ptr)  os_free(ptr)

#endif /* OS_WIN32_ALLOCA_H */
