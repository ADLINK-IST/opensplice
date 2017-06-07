/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/************************************************************************
 *
 * Copyright (c) 2007
 * PrismTech Ltd.
 * All rights Reserved.
 *
 * LOGICAL_NAME:    ErrorHandler.java
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             june 2007.
 ************************************************************************
 *
 * This file contains the implementation for the error handling operations.
 *
 ***/

package chatroom;

import DDS.*;

public class ErrorHandler {

    public static final int NR_ERROR_CODES = 13;

    /* Array to hold the names for all ReturnCodes. */
    public static String[] RetCodeName = new String[NR_ERROR_CODES];

    static {
        RetCodeName[0]     = new String("DDS_RETCODE_OK");
        RetCodeName[1]     = new String("DDS_RETCODE_ERROR");
        RetCodeName[2]     = new String("DDS_RETCODE_UNSUPPORTED");
        RetCodeName[3]     = new String("DDS_RETCODE_BAD_PARAMETER");
        RetCodeName[4]     = new String("DDS_RETCODE_PRECONDITION_NOT_MET");
        RetCodeName[5]     = new String("DDS_RETCODE_OUT_OF_RESOURCES");
        RetCodeName[6]     = new String("DDS_RETCODE_NOT_ENABLED");
        RetCodeName[7]     = new String("DDS_RETCODE_IMMUTABLE_POLICY");
        RetCodeName[8]     = new String("DDS_RETCODE_INCONSISTENT_POLICY");
        RetCodeName[9]     = new String("DDS_RETCODE_ALREADY_DELETED");
        RetCodeName[10] = new String("DDS_RETCODE_TIMEOUT");
        RetCodeName[11] = new String("DDS_RETCODE_NO_DATA");
        RetCodeName[12] = new String("DDS_RETCODE_ILLEGAL_OPERATION");
    }

    /**
     * Returns the name of an error code.
     **/
    public static String getErrorName(int status) {
        return RetCodeName[status];
    }

    /**
     * Check the return status for errors. If there is an error,
     * then terminate.
     **/
    public static void checkStatus(int status, String info) {
        if ( status != RETCODE_OK.value &&
             status != RETCODE_NO_DATA.value) {
            System.err.println(
                "Error in " + info + ": " + getErrorName(status) );
            System.exit(-1);
        }
    }

    /**
     * Check whether a valid handle has been returned. If not, then terminate.
     **/
    public static void checkHandle(Object handle, String info) {
        if (handle == null) {
            System.out.println(
                "Error in " + info + ": Creation failed: invalid handle");
            System.exit(-1);
         }
    }

}
