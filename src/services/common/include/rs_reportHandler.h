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
#ifndef RS_REPORTHANDLER_H
#define RS_REPORTHANDLER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "rs_reportMsg.h"
#include "os_if.h"

#ifdef OSPL_BUILD_REPORTSERVICE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(rs_reportHandler);

OS_API rs_reportHandler rs_reportHandlerNew (c_char *handlerId, c_ushort port);

OS_API void rs_reportHandlerFree (rs_reportHandler reportHandler);

OS_API void
rs_reportHandlerReport (
    rs_reportHandler reportHandler,
    rs_reportMsg message);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* RS_REPORTHANDLER_H */
