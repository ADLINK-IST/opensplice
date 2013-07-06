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
#include <stdio.h>
#include "DLRL_Util.h"

char *
DLRL_Util_userResultToString(
    u_result result)
{
   char *returnString = NULL;

    switch (result){
        case U_RESULT_UNDEFINED:
            returnString = "RESULT_UNDEFINED";
            break;
        case U_RESULT_OK:
            returnString = "RESULT_OK";
            break;
        case U_RESULT_INTERRUPTED:
            returnString = "RESULT_INTERRUPTED";
            break;
        case U_RESULT_NOT_INITIALISED:
            returnString = "RESULT_NOT_INITIALISED";
            break;
        case U_RESULT_OUT_OF_MEMORY:
            returnString = "RESULT_OUT_OF_MEMORY";
            break;
        case U_RESULT_INTERNAL_ERROR:
            returnString = "RESULT_INTERNAL_ERROR";
            break;
        case U_RESULT_ILL_PARAM:
            returnString = "RESULT_ILL_PARAM";
            break;
        case U_RESULT_CLASS_MISMATCH:
            returnString = "RESULT_CLASS_MISMATCH";
            break;
        case U_RESULT_DETACHING:
            returnString = "RESULT_DETACHING";
            break;
        case U_RESULT_TIMEOUT:
            returnString = "RESULT_TIMEOUT";
            break;
        case U_RESULT_INCONSISTENT_QOS:
            returnString = "RESULT_INCONSISTENT_QOS";
            break;
        case U_RESULT_IMMUTABLE_POLICY:
            returnString = "RESULT_IMMUTABLE_POLICY";
            break;
        case U_RESULT_PRECONDITION_NOT_MET:
            returnString = "RESULT_PRECONDITION_NOT_MET";
            break;
        case U_RESULT_UNSUPPORTED:
            returnString = "RESULT_UNSUPPORTED";
            break;
        default:
            returnString = "Unknown error";
            break;
    }
    return returnString;
}

char*
DLRL_Util_writeResultToString(
    v_writeResult result)
{
   char* returnString = NULL;

    switch (result){
        case V_WRITE_UNDEFINED:
            returnString = "WRITE_UNDEFINED";
            break;
        case V_WRITE_SUCCESS:
            returnString = "WRITE_SUCCESS";
            break;
        case V_WRITE_REGISTERED:
            returnString = "WRITE_REGISTERED";
            break;
        case V_WRITE_UNREGISTERED:
            returnString = "WRITE_UNREGISTERED";
            break;
        case V_WRITE_PRE_NOT_MET:
            returnString = "WRITE_PRECONDITION_NOT_MET";
            break;
        case V_WRITE_ERROR:
            returnString = "WRITE_ERROR";
            break;
        case V_WRITE_TIMEOUT:
            returnString = "WRITE_TIMEOUT";
            break;
        case V_WRITE_OUT_OF_RESOURCES:
            returnString = "WRITE_OUT_OF_RESOURCES";
            break;
        case V_WRITE_REJECTED:
            returnString = "WRITE_REJECTED";
            break;
        case V_WRITE_COUNT:
            returnString = "WRITE_COUNT";
            break;
        default:
            returnString = "Unknown error";
            break;
    }
    return returnString;
}
