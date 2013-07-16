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

#ifndef OS_COMMON_SOCKET_H
#define OS_COMMON_SOCKET_H

#include <errno.h>

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

#endif /* OS_COMMON_SOCKET_H */
