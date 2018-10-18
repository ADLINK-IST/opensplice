/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "os/integrity/include/os_gethostname_cfg.h"

os_result
os_gethostname(
    char *hostname,
    os_uint32 buffersize)
{
    os_result result;
    char hostnamebuf[MAXHOSTNAMELEN];

    if (os_gethostname_ptr (hostnamebuf, MAXHOSTNAMELEN) == 0) 
    {
        if ((strlen(hostnamebuf)+1) > (size_t)buffersize) 
        {
            result = os_resultFail;
        } 
        else 
        {
            os_strncpy (hostname, hostnamebuf, (size_t)buffersize);
            result = os_resultSuccess;
	    }
    } 
    else 
    {
        result = os_resultFail;
    }
    return result;
}
