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

#include <limits.h>
#include <stdio.h>
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

/* VxWorks has a non-compliant strerror_r in kernel mode which only takes a
   buffer and an error number. Providing a buffer smaller than NAME_MAX + 1
   (256) may result in a buffer overflow. See target/src/libc/strerror.c for
   details. NAME_MAX is defined in limits.h. */

#if defined(OS_ERRNO_MIN_LEN)
#undef OS_ERRNO_MIN_LEN
#endif

#define OS_ERRNO_MIN_LEN 256

static os_int
os_strerror_r (os_int err, os_char *str, os_size_t len)
{
    os_int res = OS_ERRNO_SUCCESS;

    assert (str != NULL);
    assert (len >= OS_ERRNO_MIN_LEN && (len % OS_ERRNO_MIN_LEN) == 0);

    if (len < (NAME_MAX + 1)) {
        res = OS_ERRNO_TOO_SMALL;
    } else if (strerror_r (err, str) != OK) {
        res = OS_ERRNO_FAIL;
    }

    return res;
}

#include "../common/code/os_errno.c"
