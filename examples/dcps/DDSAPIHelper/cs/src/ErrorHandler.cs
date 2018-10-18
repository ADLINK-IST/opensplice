/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

/********************************************************************************
 * LOGICAL_NAME:    ErrorHandler.cs
 * FUNCTION:        OpenSplice ContentFilteredTopic example code.
 * MODULE:          ContentFilteredTopic example for the C# programming language.
 * DATE             October 2010.
 *********************************************************************************
 *
 * This file contains the implementation for the error handling operations.
 *
 ***/
using DDS;

namespace DDSAPIHelper {

    /// <summary>
    /// Utility class to check the status of various Vortex OpenSplice operations.
    /// </summary>
public sealed class ErrorHandler {

    public const int NR_ERROR_CODES = 13;

	/* Array to hold the names for all ReturnCodes. */
	public static string[] RetCodeName =  new string[NR_ERROR_CODES];
	public static int t;

	static ErrorHandler() {
		RetCodeName[0] 	= "DDS_RETCODE_OK";
		RetCodeName[1] 	= "DDS_RETCODE_ERROR";
		RetCodeName[2] 	= "DDS_RETCODE_UNSUPPORTED";
		RetCodeName[3] 	= "DDS_RETCODE_BAD_PARAMETER";
		RetCodeName[4] 	= "DDS_RETCODE_PRECONDITION_NOT_MET";
		RetCodeName[5] 	= "DDS_RETCODE_OUT_OF_RESOURCES";
		RetCodeName[6] 	= "DDS_RETCODE_NOT_ENABLED";
		RetCodeName[7] 	= "DDS_RETCODE_IMMUTABLE_POLICY";
		RetCodeName[8] 	= "DDS_RETCODE_INCONSISTENT_POLICY";
		RetCodeName[9] 	= "DDS_RETCODE_ALREADY_DELETED";
		RetCodeName[10] = "DDS_RETCODE_TIMEOUT";
		RetCodeName[11] = "DDS_RETCODE_NO_DATA";
        RetCodeName[12] = "DDS_RETCODE_ILLEGAL_OPERATION";
	}

	/**
	 * Returns the name of an error code.
	 **/
	public static string getErrorName(ReturnCode status) {
		return RetCodeName[(int)status];
	}

	/**
	 * Check the return status for errors. If there is an error,
     * then terminate.
	 **/
	public static void checkStatus(ReturnCode status, string info) {
        if (status != ReturnCode.Ok &&
             status != ReturnCode.NoData)
        {
            System.Console.WriteLine(
                "Error in " + info + ": " + getErrorName(status));
            System.Environment.Exit(-1);
        }
	}

	/**
	 * Check whether a valid handle has been returned. If not, then terminate.
	 **/
	public static void checkHandle(object handle, string info) {
		if (handle == null) {
	        System.Console.WriteLine (
                "Error in " + info + ": Creation failed: invalid handle");
            System.Environment.Exit(-1);
	     }
	}

}

}
