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

#include "os_defs.h"
#include "os_errno.h"
#include "os_stdlib.h"
#include "os_thread.h"

#include <windows.h>

#include "../common/include/os_errno.h"

/* WSAGetLastError, GetLastError and errno

   Windows supports errno (The Microsoft c Run-Time Library for Windows CE
   does so since version 15 (Visual Studio 2008)). Error codes set by the
   Windows Sockets implementation, however, are NOT made available via the
   errno variable.

   WSAGetLastError used to be the thread-safe version of GetLastError, but
   nowadays is just an an alias for GetLastError as intended by Microsoft:
   http://www.sockets.com/winsock.htm#Deviation_ErrorCodes
   */

os_int
os_getErrno (void)
{
    return GetLastError();
}

void
os_setErrno (os_int err)
{
    SetLastError (err);
}

static os_int
os_strerror_r (os_int err, os_char *str, os_size_t len)
{
    LPVOID buf = NULL;
    os_int cnt, res = OS_ERRNO_SUCCESS;

    assert (str != NULL);
    assert (len >= OS_ERRNO_MIN_LEN && (len % OS_ERRNO_MIN_LEN) == 0);

    (void)FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS |
                        FORMAT_MESSAGE_MAX_WIDTH_MASK,
                        NULL,
                        err,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR) &buf,
                        0,
                        NULL);

    if (cnt != 0) {
        /* Calculate buffer size required to store message */
        cnt = WideCharToMultiByte (CP_ACP, 0, buf, -1, NULL, 0, NULL, NULL);
        if (cnt < len) {
            (void)WideCharToMultiByte (
                CP_ACP, 0, buf, -1, str, (os_size_t)len, NULL, NULL);
            /* null terminate, just to be sure */
            str[cnt] = '\0';
        } else {
            res = OS_ERRNO_TOO_SMALL;
        }
    } else {
        res = OS_ERRNO_FAIL;
    }

    if (buf != NULL) {
        LocalFree (buf);
    }

    return res;
}

#include "../common/code/os_errno.c"
