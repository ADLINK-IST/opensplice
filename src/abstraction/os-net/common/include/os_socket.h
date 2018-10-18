/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef OS_COMMON_SOCKET_H
#define OS_COMMON_SOCKET_H

#include "os_errno.h"

/* List of socket error numbers */
#define os_sockEAGAIN       EAGAIN      /* Operation would block, or a timeout expired before operation succeeded */
#define os_sockEWOULDBLOCK  EWOULDBLOCK /* Operation would block */
#define os_sockEPERM        EPERM       /* Operation not permitted */
#define os_sockENOENT       ENOENT      /* No such file or directory */
#define os_sockEINTR        EINTR       /* Interrupted system call */
#define os_sockEBADF        EBADF       /* Bad file number */
#define os_sockENOMEM       ENOMEM      /* Out of memory */
#define os_sockEACCES       EACCES      /* Permission denied */
#define os_sockEINVAL       EINVAL      /* Invalid argument */
#define os_sockEMFILE       EMFILE          /* Too many open files */
#define os_sockENOSR        ENOSR       /* Out of streams resources */
#define os_sockENOTSOCK     ENOTSOCK    /* Socket operation on non-socket */
#define os_sockEMSGSIZE     EMSGSIZE    /* Message too long */
#define os_sockENOPROTOOPT  ENOPROTOOPT /* Protocol not available */
#define os_sockEPROTONOSUPPORT  EPROTONOSUPPORT /* Protocol not supported */
#define os_sockEADDRINUSE   EADDRINUSE  /* Address already in use */
#define os_sockEADDRNOTAVAIL    EADDRNOTAVAIL   /* Cannot assign requested address */
#define os_sockENETUNREACH  ENETUNREACH /* Network is unreachable */
#define os_sockENOBUFS      ENOBUFS     /* No buffer space available */
#define os_sockECONNRESET   ECONNRESET  /* Connection reset by peer */

#ifdef IP_ADD_SOURCE_MEMBERSHIP
#define OS_SOCKET_HAS_SSM 1
#else
#define OS_SOCKET_HAS_SSM 0
#endif

#endif /* OS_COMMON_SOCKET_H */
