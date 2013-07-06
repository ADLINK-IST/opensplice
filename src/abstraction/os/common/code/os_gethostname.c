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
#include <string.h>

os_result
os_gethostname(
    char *hostname,
    os_uint32 buffersize)
{
    os_result result;
    char hostnamebuf[MAXHOSTNAMELEN];

    if (gethostname (hostnamebuf, MAXHOSTNAMELEN) == 0) {
        if ((strlen(hostnamebuf)+1) > (size_t)buffersize) {
            result = os_resultFail;
        } else {
            os_strncpy (hostname, hostnamebuf, (size_t)buffersize);
            result = os_resultSuccess;
	    }
    } else {
        result = os_resultFail;
    }

    return result;
}
