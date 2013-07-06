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

#include "gapi.h"

#include "dds_dcps.h"

/*     ReturnCode_t
 *     update( );
 */
DDS_ReturnCode_t
DDS_ErrorInfo_update (
    DDS_ErrorInfo _this
    )
{
    return (DDS_ReturnCode_t)
        gapi_errorInfo_update( 
            (gapi_errorInfo)_this
        );
}

/*     ReturnCode_t
 *     get_code(
 *         out ErrorCode code);
 */
DDS_ReturnCode_t
DDS_ErrorInfo_get_code (
    DDS_ErrorInfo _this,
    DDS_ErrorCode_t * code
    )
{
    return (DDS_ReturnCode_t)
        gapi_errorInfo_get_code(
            (gapi_errorInfo)_this,
            (gapi_errorCode_t*)code
        );
}

/*     ReturnCode_t
 *     get_location(
 *         out String location);
 */
DDS_ReturnCode_t
DDS_ErrorInfo_get_location(
    DDS_ErrorInfo _this,
    DDS_string * location
    )
{
    return (DDS_ReturnCode_t)
        gapi_errorInfo_get_location(
            (gapi_errorInfo)_this,
            (gapi_string*)location
        );
}

/*     ReturnCode_t
 *     get_source_line(
 *         out String source_line);
 */
DDS_ReturnCode_t
DDS_ErrorInfo_get_source_line(
    DDS_ErrorInfo _this,
    DDS_string * source_line
    )
{
    return (DDS_ReturnCode_t)
        gapi_errorInfo_get_source_line(
            (gapi_errorInfo)_this,
            (gapi_string*)source_line
        );
}

/*     ReturnCode_t
 *     get_stack_trace(
 *         out String stack_trace);
 */
DDS_ReturnCode_t
DDS_ErrorInfo_get_stack_trace(
    DDS_ErrorInfo _this,
    DDS_string * stack_trace
    )
{
    return (DDS_ReturnCode_t)
        gapi_errorInfo_get_stack_trace(
            (gapi_errorInfo)_this,
            (gapi_string*)stack_trace
        );    
}

/*     ReturnCode_t
 *     get_message(
 *         out String message);
 */
DDS_ReturnCode_t
DDS_ErrorInfo_get_message(
    DDS_ErrorInfo _this,
    DDS_string * message
    )
{
    return (DDS_ReturnCode_t)
        gapi_errorInfo_get_message(
            (gapi_errorInfo)_this,
            (gapi_string*)message
        );    
}


/*     ErrorInfo
 *     ErrorInfo__alloc (
 *         void);
 */
DDS_ErrorInfo
DDS_ErrorInfo__alloc (
    void
    )
{
    gapi_errorInfo _this;

    _this = gapi_errorInfo__alloc();

    return _this;
}



