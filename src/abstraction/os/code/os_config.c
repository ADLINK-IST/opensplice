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
