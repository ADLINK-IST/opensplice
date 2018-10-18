/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef NB_SERVICE_H
#define NB_SERVICE_H

#include "vortex_os.h"

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"

#ifdef OSPL_BUILD_NWBRIDGE_SERVICE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif /* OSPL_BUILD_NWBRIDGE_SERVICE */

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API OPENSPLICE_ENTRYPOINT_DECL(ospl_nwbridge);

OS_API void ospl_nwbridgeAtExit(void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* NB_SERVICE_H */
