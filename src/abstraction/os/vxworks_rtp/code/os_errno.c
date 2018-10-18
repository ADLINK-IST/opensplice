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

#include <string.h>

#include "os_defs.h"
#include "os_errno.h"
#include "os_stdlib.h"
#include "os_thread.h"

#include "../common/include/os_errno.h"

os_int
os_getErrno (void)
{
    return errno;
}

void
os_setErrno (os_int err)
{
    errno = err;
}

static os_int
os_strerror_r (os_int err, os_char *str, os_size_t len)
{
    os_char chr;
    os_int res = OS_ERRNO_SUCCESS;

    assert (str != NULL);
    assert (len >= OS_ERRNO_MIN_LEN && (len % OS_ERRNO_MIN_LEN) == 0);

    /* VxWorks's strerror_r always returns 0 (zero), so the only way to decide
       if the error was truncated is to check if the last position in the
       buffer is overwritten by strerror_r. */
    chr = str[len - 1];
    str[len - 1] = 'x';

    (void)strerror_r (err, str, len);

    if (str[len - 1] != 'x') {
        res = OS_ERRNO_TOO_SMALL;
    }

    str[len - 1] = '\0'; /* always null terminate, just to be safe */

    return res;
}

#include "../common/code/os_errno.c"
