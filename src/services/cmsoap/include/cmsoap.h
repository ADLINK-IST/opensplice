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

/**
 * @file services/cmsoap/include/cmsoap.h
 *
 * Main class for the Control & Monitoring SOAP service.
 */
#ifndef CMSOAP_H
#define CMSOAP_H

#include "os.h"

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"

#ifdef OSPL_BUILD_CMSOAP
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * Starts the Control & Monitoring SOAP service.
 *
 * @param argc The number of arguments.
 * @param argv The list of arguments. The service expects one argument; the
 *             uri where to attach to.
 * @return If the execution was successfull 0 is returned, if not
 *         error is returned.
 */

OS_API OPENSPLICE_ENTRYPOINT_DECL(ospl_cmsoap);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
