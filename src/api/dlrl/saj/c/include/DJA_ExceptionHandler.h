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
#ifndef DJA_EXCEPTION_HANDLER_H
#define DJA_EXCEPTION_HANDLER_H

#include <jni.h>
#include "dds_dcps.h"
#include "gapi.h"
#include "DLRL_Exception.h"

typedef DDS_ReturnCode_t LOC_ReturnCode_t;

void DJA_ExceptionHandler_handleException(JNIEnv * env, DLRL_Exception* exception);

/* if an exception occured in a java callback then this exception must be transformed into the exception  */
/* structure used by the kernel, so that the kernel can detect exceptions that occured in the java part of this  */
/* callback. */
void DJA_ExceptionHandler_forwardJavaExceptionToKernel(JNIEnv * env, DLRL_Exception* exception);
/* NOT IN DESIGN */
char *DJA_ExceptionHandler_returnCodeDCPSToString(LOC_ReturnCode_t returnCode);

/* NOT IN DESIGN */
char *DJA_ExceptionHandler_returnCodeGAPIToString(gapi_returnCode_t returnCode);

LOC_boolean DJA_ExceptionHandler_hasJavaExceptionOccurred(JNIEnv *env);

#define DLRL_JavaException_PROPAGATE(env, exception) \
  DJA_ExceptionHandler_forwardJavaExceptionToKernel(env, exception); \
  DLRL_Exception_PROPAGATE(exception)

#define DLRL_DcpsException_PROPAGATE(_this, returnCode, ...) \
    do { \
        if(returnCode != LOC_RETCODE_OK){\
            DLRL_Exception_THROW(_this, DLRL_DCPS_ERROR, "%s: %s. Check DCPS error log file for (possibly) more information.", DJA_ExceptionHandler_returnCodeDCPSToString((returnCode)), __VA_ARGS__);\
        } \
    } while(0)

/* NOT IN DESIGN */
#define DLRL_Exception_PROPAGATE_GAPI_RESULT(_this, returnCode, message) \
    do { \
        if(returnCode != GAPI_RETCODE_OK){ \
            DLRL_Exception_THROW(_this, DLRL_DCPS_ERROR, "%s: %s. Check DCPS error log file for (possibly) more information.", DJA_ExceptionHandler_returnCodeGAPIToString((returnCode)), message); \
        } \
    } while(0)


/* NOT IN DESIGN - all */
#define LOC_RETCODE_OK                                  0
#define LOC_RETCODE_ERROR                               1
#define LOC_RETCODE_UNSUPPORTED                         2
#define LOC_RETCODE_BAD_PARAMETER                       3
#define LOC_RETCODE_PRECONDITION_NOT_MET                4
#define LOC_RETCODE_OUT_OF_RESOURCES                    5
#define LOC_RETCODE_NOT_ENABLED                         6
#define LOC_RETCODE_IMMUTABLE_POLICY                    7
#define LOC_RETCODE_INCONSISTENT_POLICY                 8
#define LOC_RETCODE_ALREADY_DELETED                     9
#define LOC_RETCODE_TIMEOUT                             10
#define LOC_RETCODE_NO_DATA                             11

#endif
