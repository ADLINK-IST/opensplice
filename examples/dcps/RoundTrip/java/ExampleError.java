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

/**
 * This class defines some simple error handling functions for use in the OpenSplice Standalone Java examples.
 */
class ExampleError
{
    public static String[] ReturnCodeName =
    {
        "DDS_RETCODE_OK", "DDS_RETCODE_ERROR", "DDS_RETCODE_UNSUPPORTED",
        "DDS_RETCODE_BAD_PARAMETER", "DDS_RETCODE_PRECONDITION_NOT_MET",
        "DDS_RETCODE_OUT_OF_RESOURCES", "DDS_RETCODE_NOT_ENABLED",
        "DDS_RETCODE_IMMUTABLE_POLICY", "DDS_RETCODE_INCONSISTENT_POLICY",
        "DDS_RETCODE_ALREADY_DELETED", "DDS_RETCODE_TIMEOUT", "DDS_RETCODE_NO_DATA",
        "DDS_RETCODE_ILLEGAL_OPERATION"
    };

    /**
    * Function to convert DDS return codes into an exception with meaningful output.
    * @param returnCode DDS return code
    * @param where A string detailing where the error occurred
    */
    public static void CheckStatus(int returnCode, String where) throws Exception
    {
        if (returnCode != DDS.RETCODE_OK.value && returnCode != DDS.RETCODE_NO_DATA.value)
        {
            throw new Exception(ReturnCodeName[returnCode] + " thrown at: " + where);
        }
    }

    /**
    * Function to check for a null handle and throw an exception with meaningful output.
    * @param handle to check for null
    * @param where A string detailing where the error occured
    */
    public static void CheckHandle(Object handle, String where) throws NullPointerException
    {
        if (handle == null)
        {
            throw new NullPointerException("Null pointer at: " + where);
        }
    }
}
