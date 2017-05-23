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

   There is no relationship between GetLastError and errno.
   GetLastError gets the last error that occurred in a Windows API function (for the current thread).
   errno contains the last error that occurred in the C runtime library
   So if you call a winapi function like CreateFile you check GetLastError
   (assuming that the function call failed), while if you call a C runtime library function
   like fopen you check errno (again assuming that the call failed).
   */

os_int
os_getErrno (void)
{
    DWORD err = GetLastError();
    if (err != 0) {
        errno = (int) err;
    }
    return errno;
}

void
os_setErrno (os_int err)
{
    SetLastError (err);
    errno = err;
}

static os_int
os_strerror_r (os_int err, os_char *str, os_size_t len)
{
    DWORD cnt;
    os_int res = OS_ERRNO_SUCCESS;

    assert (str != NULL);
    assert ((len % OS_ERRNO_MIN_LEN) == 0);

    cnt = FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_IGNORE_INSERTS |
                         FORMAT_MESSAGE_MAX_WIDTH_MASK,
                         NULL,
                         err,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         (LPTSTR)str,
                         (DWORD)len,
                         NULL);

    assert (cnt < (len - 1));
    if (cnt == 0) {
        if (os_getErrno () == ERROR_INSUFFICIENT_BUFFER) {
            res = OS_ERRNO_TOO_SMALL;
        } else {
            res = OS_ERRNO_FAIL;
        }
    }

    return res;
}

#include "../common/code/os_errno.c"
