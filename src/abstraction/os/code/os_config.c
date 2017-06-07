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

#include "os_config.h"
#include "os_stdlib.h"

/**
* Determines if the passed string holds a 'true' value.
* @param configString A string holding something that indiocates true or false.
* @param resultOut Pointer to a os_boolean that should be set to true or false
* depending on the value of configString.
* Will be set OS_TRUE iff string holds "1", yes, or true. OS_FALSE if it holds
* "0", no, or false.
* @return os_resultFail if the string contains neither of the above;
* os_resultSuccess otherwise.
*/
os_result
os_configIsTrue(
    const char* configString,
    os_boolean* resultOut)
{
    os_result result = os_resultSuccess;

    if (os_strcasecmp(configString, "FALSE") == 0 ||
        os_strcasecmp(configString, "0")     == 0 ||
        os_strcasecmp(configString, "NO")    == 0)
    {
        *resultOut = OS_FALSE;
    }
    else
    {
        if (os_strcasecmp(configString, "TRUE") == 0 ||
            os_strcasecmp(configString, "1")    == 0 ||
            os_strcasecmp(configString, "YES")  == 0)
        {
            *resultOut = OS_TRUE;
        }
        else
        {
            result = os_resultFail;
        }
    }
    return result;
}
