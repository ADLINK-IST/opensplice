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
#ifndef NN_DDSI2_H
#define NN_DDSI2_H

#include "os.h"
#include "kernelModule.h"
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

OS_API OPENSPLICE_ENTRYPOINT_DECL(ospl_ddsi2);

OS_API void
ospl_ddsi2AtExit(
    void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* NN_DDSI2_H */

/* SHA1 not available (unoffical build.) */
