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
#ifndef U__HANDLE_H
#define U__HANDLE_H

#include "u_handle.h"
/* exporting some functions from this header file is only needed,
 * since cmxml uses these functions
 */
#include "v_handle.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u__handleResult(result) \
        ((result == V_HANDLE_OK) ? U_RESULT_OK : \
         (result == V_HANDLE_EXPIRED) ? U_RESULT_ALREADY_DELETED : \
                                        U_RESULT_ILL_PARAM)

#define u__handleIsEqual(h1,h2) v_handleIsEqual((h1),(h2))
#define u__handleIsNil(handle)  v_handleIsNil(handle)
#define u__handleSetNil(handle) v_handleSetNil(handle)

#undef OS_API

#endif

