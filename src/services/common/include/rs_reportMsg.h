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
#ifndef RS_REPORTMSG_H
#define RS_REPORTMSG_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_report.h"
#include "c_typebase.h"
#include "c_time.h"
#include "os_if.h"

#ifdef OSPL_BUILD_REPORTSERVICE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define rs_reportMsg(o)	((rs_reportMsg)(o))

typedef os_reportType rs_reportType;

C_CLASS(rs_reportMsg);

/* Construct a new message */
OS_API rs_reportMsg
rs_reportMsgNew	(
    const rs_reportType reportType,
    const c_char       *reportContext,
    const c_char       *fileName,
    c_long              lineNo,
    c_long              reportCode,
    const c_char       *description,
    const c_char       *domain,
    const c_char       *node,
    const c_char       *process,
    const c_char       *thread,
    c_time              dateTime);

/* Free the constructed message */
OS_API void 		rs_reportMsgFree 		(rs_reportMsg reportMsg);

/* Process the constructed message */
OS_API void		rs_reportMsgReport 		(rs_reportMsg reportMsg);

/* Getter methods */
OS_API rs_reportType 	rs_reportMsgGetReportType 	(rs_reportMsg reportMsg);
OS_API c_char * 	rs_reportMsgGetReportContext 	(rs_reportMsg reportMsg);
OS_API c_char * 	rs_reportMsgGetFileName 	(rs_reportMsg reportMsg);
OS_API c_long	 	rs_reportMsgGetLineNo 		(rs_reportMsg reportMsg);
OS_API c_long 		rs_reportMsgGetReportCode 	(rs_reportMsg reportMsg);
OS_API c_char * 	rs_reportMsgGetDescription 	(rs_reportMsg reportMsg);
OS_API c_char * 	rs_reportMsgGetDomain 		(rs_reportMsg reportMsg);
OS_API c_char * 	rs_reportMsgGetNode 		(rs_reportMsg reportMsg);
OS_API c_char * 	rs_reportMsgGetProcess 		(rs_reportMsg reportMsg);
OS_API c_char * 	rs_reportMsgGetThread 		(rs_reportMsg reportMsg);
OS_API c_time 		rs_reportMsgGetDateTime 	(rs_reportMsg reportMsg);

/* Setter methods */
OS_API void 		rs_reportMsgSetReportType 	(rs_reportMsg reportMsg, const rs_reportType reportType);
OS_API void 		rs_reportMsgSetReportContext 	(rs_reportMsg reportMsg, const c_char *reportContext);
OS_API void 		rs_reportMsgSetFileName 	(rs_reportMsg reportMsg, const c_char *fileName);
OS_API void	 	rs_reportMsgSetLineNo 		(rs_reportMsg reportMsg, c_long lineNo);
OS_API void 		rs_reportMsgSetReportCode 	(rs_reportMsg reportMsg, c_long reportCode);
OS_API void 		rs_reportMsgSetDescription 	(rs_reportMsg reportMsg, const c_char *description);
OS_API void 		rs_reportMsgSetDomain 		(rs_reportMsg reportMsg, const c_char *spliceSystem);
OS_API void 		rs_reportMsgSetNode	 	(rs_reportMsg reportMsg, const c_char *node);
OS_API void 		rs_reportMsgSetProcess 		(rs_reportMsg reportMsg, const c_char *process);
OS_API void 		rs_reportMsgSetThread 		(rs_reportMsg reportMsg, const c_char *thread);
OS_API void 		rs_reportMsgSetDateTime 	(rs_reportMsg reportMsg, c_time dateTime);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* RS_REPORTMSG_H */
