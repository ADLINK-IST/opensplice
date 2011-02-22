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

#include "ccpp_ErrorInfo.h"
#include "ccpp_Utils.h"
#include "os_report.h"

DDS::ErrorInfo::ErrorInfo( void )
{
    DDS::ccpp_UserData_ptr myUD;
    _gapi_self = gapi_errorInfo__alloc();
    if (_gapi_self) {
        myUD = new DDS::ccpp_UserData(this);
        if (myUD) {
            gapi_object_set_user_data(_gapi_self, (CORBA::Object *)myUD,
                                      DDS::ccpp_CallBack_DeleteUserData,NULL);
        } else {
            OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
        }
        CORBA::release(this);
    }
}

DDS::ErrorInfo::~ErrorInfo( void )
{
    DDS::ccpp_UserData_ptr myUD;
    myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(_gapi_self));
    if (myUD) {
        /* avoid another last release of the reference to this ErrorInfo */
        myUD->ccpp_object = NULL;
        delete myUD;
    } else {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
    }
    gapi__free(_gapi_self);
}

DDS::ReturnCode_t DDS::ErrorInfo::update( 
    void
) THROW_ORB_EXCEPTIONS
{
    return (DDS::ReturnCode_t)gapi_errorInfo_update(_gapi_self);
}

DDS::ReturnCode_t DDS::ErrorInfo::get_code(
    DDS::ErrorCode_t & code
) THROW_ORB_EXCEPTIONS
{
    gapi_returnCode_t result;
    gapi_errorCode_t gapi_code = (gapi_errorCode_t)code;

    result = gapi_errorInfo_get_code(_gapi_self,&gapi_code);
    code = (DDS::ErrorCode_t)gapi_code;

    return (DDS::ReturnCode_t)result;
}

DDS::ReturnCode_t DDS::ErrorInfo::get_message(
    char * & message
) THROW_ORB_EXCEPTIONS
{
    gapi_returnCode_t result;
    gapi_string gapi_message = NULL;
    
    result = gapi_errorInfo_get_message(_gapi_self, &gapi_message);
    if (result == GAPI_RETCODE_OK){
        if (gapi_message != NULL) {
            if (message != NULL) {
                CORBA::string_free(message);
            }
            message = CORBA::string_dup(gapi_message);
            gapi__free(gapi_message);
        } else {
            CORBA::string_free(message);
            message = NULL;
        }
    }
    return (DDS::ReturnCode_t)result;
}

DDS::ReturnCode_t DDS::ErrorInfo::get_location(
    char * & location
) THROW_ORB_EXCEPTIONS
{
    gapi_returnCode_t result;
    gapi_string gapi_location = NULL;
    
    result = gapi_errorInfo_get_location(_gapi_self, &gapi_location);
    if (result == GAPI_RETCODE_OK){
        if (gapi_location != NULL) {
            if (location != NULL) {
                CORBA::string_free(location);
            }
            location = CORBA::string_dup(gapi_location);
            gapi__free(gapi_location);
        } else {
            CORBA::string_free(location);
            location = NULL;
        }
    }
    return (DDS::ReturnCode_t)result;
}

DDS::ReturnCode_t DDS::ErrorInfo::get_source_line(
    char * & source_line
) THROW_ORB_EXCEPTIONS
{
    gapi_returnCode_t result;
    gapi_string gapi_source_line = NULL;
    
    result = gapi_errorInfo_get_source_line(_gapi_self, &gapi_source_line);
    if (result == GAPI_RETCODE_OK){
        if (gapi_source_line != NULL) {
            if (source_line != NULL) {
                CORBA::string_free(source_line);
            }
            source_line = CORBA::string_dup(gapi_source_line);
            gapi__free(gapi_source_line);
        } else {
            CORBA::string_free(source_line);
            source_line = NULL;
        }
    }
    return (DDS::ReturnCode_t)result;
}

DDS::ReturnCode_t DDS::ErrorInfo::get_stack_trace(
    char * & stack_trace
) THROW_ORB_EXCEPTIONS
{
    gapi_returnCode_t result;
    gapi_string gapi_stack_trace = NULL;
    
    result = gapi_errorInfo_get_stack_trace(_gapi_self, &gapi_stack_trace);
    if (result == GAPI_RETCODE_OK){
        if (gapi_stack_trace != NULL) {
            if (stack_trace != NULL) {
                CORBA::string_free(stack_trace);
            }
            stack_trace = CORBA::string_dup(gapi_stack_trace);
            gapi__free(gapi_stack_trace);
        } else {
            CORBA::string_free(stack_trace);
            stack_trace = NULL;
        }
    }
    return (DDS::ReturnCode_t)result;
}






