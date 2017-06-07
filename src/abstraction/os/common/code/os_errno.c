#include "os_defs.h"
#include "os_errno.h"
#include "os_stdlib.h"
#include "os_thread.h"

const os_char *
os_strError (os_int err)
{
    os_char *mem;
    os_char *ptr, *str = NULL;
    os_size_t len = 0;
    os_size_t tot;
    os_int res = OS_ERRNO_TOO_SMALL;

    /* os_threadMem* does not support destructors, but does free the memory
       referred to by the index. To avoid memory leaks and extra work the
       length of the string is placed in front of the string. */
    mem = os_threadMemGet (OS_THREAD_STR_ERROR);
    if (mem == NULL) {
        len = OS_ERRNO_MIN_LEN;
        tot = sizeof (len) + len + 1;
        mem = os_threadMemMalloc (OS_THREAD_STR_ERROR, tot, NULL, NULL);
        if (mem != NULL) {
            memset (mem, 0, tot);
            memcpy (mem, &len, sizeof (len));
        }
    }
    if (mem != NULL) {
        str = mem + sizeof (len);
        memcpy (&len, mem, sizeof (len));
    }

    /* os_strerror_r returns OS_ERRNO_TOO_SMALL if the buffer is too small.
       Iteratively increase the buffer and retry until OS_ERRNO_MAX_LEN is
       reached, in which case, give up and hope that os_strerror_r copied
       something useful. */
    for (; str != NULL && res == OS_ERRNO_TOO_SMALL;) {
        assert (len != 0 && (len % OS_ERRNO_MIN_LEN) == 0);
        /* Solaris 10 does not populate buffer if it is too small */
        memset (str, '\0', len + 1);
        if ((res = os_strerror_r (err, str, len)) == OS_ERRNO_TOO_SMALL) {
            if (len < OS_ERRNO_MAX_LEN) {
                os_threadMemFree (OS_THREAD_STR_ERROR);
                len *= 2;
                tot = sizeof (len) + len + 1;
                mem = os_threadMemMalloc (OS_THREAD_STR_ERROR, tot, NULL, NULL);
                if (mem != NULL) {
                    memset (mem, 0, tot);
                    memcpy (mem, &len, sizeof (len));
                    str = mem + sizeof (len);
                } else {
                    res = OS_ERRNO_FAIL;
                    str = NULL;
                    len = 0;
                }
            } else {
                res = OS_ERRNO_FAIL;
            }
        }
    }

    if (res == OS_ERRNO_SUCCESS) {
        /* Strip newline and/or carriage return */
        for (ptr = str; ptr != NULL && *ptr != '\0'; ptr++) {
            if (*ptr == '\n' || *ptr == '\r') {
                *ptr = '\0';
            }
        }
    }

    if (str != NULL && str[0] == '\0') {
        (void)snprintf (str, len, "Unknown error (%d)", err);
    }

    return str;
}
