/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef OS_WIN32_PROCESS_H
#define OS_WIN32_PROCESS_H

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h> /* for HANDLE */

#if defined (__cplusplus)
extern "C" {
#endif

  typedef HANDLE os_os_procId;

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_PROCESS_H */
