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
#ifndef NN_DDSI2_H
#define NN_DDSI2_H

#include "vortex_os.h"
#include "kernelModuleI.h"
#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"

#ifdef OSPL_BUILD_DDSI2
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#ifdef DDSI2E_OR_NOT2E
OS_API OPENSPLICE_ENTRYPOINT_DECL(ospl_ddsi2e);
#else
OS_API OPENSPLICE_ENTRYPOINT_DECL(ospl_ddsi2);
#endif

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* NN_DDSI2_H */
