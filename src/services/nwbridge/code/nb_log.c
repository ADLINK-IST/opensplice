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
#include "nb__log.h"
#include "nb__thread.h"
#include "nb__configuration.h"

#include "vortex_os.h"
#include "os_report.h"

#define DATE_SIZE (OS_CTIME_R_BUFSIZE)
#define MAX_TIMESTAMP_LENGTH (10 + 1 + 6)
#define MAX_TID_LENGTH 15
#define MAX_HDR_LENGTH (DATE_SIZE + 1 + MAX_TIMESTAMP_LENGTH + 1 + MAX_TID_LENGTH + 2)

#define BUF_OFFSET MAX_HDR_LENGTH

static void         nb__logbufFlushReal(nb_thread self,
                                        nb_logbuf lb) __nonnull_all__;

static void         nb__logbufFlush(nb_thread self,
                                    nb_logbuf lb) __nonnull_all__;


static void         nb_vlogb(nb_thread self,
                             const char *fmt,
                             va_list ap) __nonnull((1, 2))
                                         __attribute_format__((printf, 2, 0));

nb_logbuf
nb_logbufNew(void)
{
    nb_logbuf lb = os_malloc(sizeof *lb);

    nb_logbufInit(lb);

    return lb;
}

void
nb_logbufInit(
    nb_logbuf lb)
{
    assert(lb);

    lb->bufsz = sizeof(lb->buf);
    lb->pos = BUF_OFFSET;
    lb->tstamp = OS_TIMEW_INVALID;
    lb->buf[lb->pos] = 0;
}

static void
nb__logbufFlushReal(
    nb_thread self,
    nb_logbuf lb)
{
    nb_logConfig gc;

    nb_objectIsValidKind(self, NB_OBJECT_THREAD);
    assert(lb);

    gc = nb_threadLogConfig(self);

    if (gc->tracing.file) {
        char hdr[MAX_HDR_LENGTH + 1];
        char date[DATE_SIZE];
        os_int32 n;
        os_uint32 tsec, tusec;

        if (OS_TIMEW_ISINVALID(lb->tstamp)) {
            lb->tstamp = os_timeWGet();
        }

        tsec = (os_uint32)OS_TIMEW_GET_SECONDS(lb->tstamp);
        tusec = (os_uint32)OS_TIMEW_GET_NANOSECONDS(lb->tstamp) / 1000;
        lb->tstamp = OS_TIMEW_INVALID;
        os_ctimeW_r(&lb->tstamp, date, DATE_SIZE);
        n = snprintf(hdr, sizeof(hdr), "%s %u.%06u/%*.*s: ", date, tsec, tusec, MAX_TID_LENGTH, MAX_TID_LENGTH, nb_threadName(self));
        assert(0 < n && n <= BUF_OFFSET);
        memcpy(lb->buf + BUF_OFFSET - (size_t)n, hdr, (size_t)n);
        fwrite(lb->buf + BUF_OFFSET - (size_t)n, 1, lb->pos - BUF_OFFSET + (size_t)n, gc->tracing.file);
        fflush(gc->tracing.file);
    }
    lb->pos = BUF_OFFSET;
    lb->buf[lb->pos] = 0;
}


static void
nb__logbufFlush(
    nb_thread self,
    nb_logbuf lb)
{
    nb_objectIsValidKind(self, NB_OBJECT_THREAD);
    assert(lb);

    if (lb->pos > BUF_OFFSET) {
        if (lb->pos < (int)sizeof(lb->buf)) {
            lb->buf[lb->pos++] = '\n';
        } else {
            lb->buf[sizeof(lb->buf)-1] = '\n';
        }
        nb__logbufFlushReal(self, lb);
    }
}

void
nb_logbufFree(
    nb_thread thread,
    nb_logbuf lb)
{
    nb_objectIsValidKind(thread, NB_OBJECT_THREAD);
    assert(lb);

    nb__logbufFlush(thread, lb);
    os_free(lb);
}

/* LOGGING ROUTINES */
static void
nb_vlogb(
    nb_thread self,
    const char *fmt,
    va_list ap)
{
    int n, trunc = 0;
    size_t nrem;
    nb_logbuf lb;

    assert(self);
    assert(fmt);

    if (*fmt == 0) {
        return;
    }
    lb = nb_threadLogbuf(self);

    /* Copy message to log buffer */
    nrem = lb->bufsz - lb->pos;
    if (nrem > 0) {
        n = os_vsnprintf(lb->buf + lb->pos, nrem, fmt, ap);
        if (n >= 0 && (size_t) n < nrem) {
            lb->pos += (size_t) n;
        } else {
            lb->pos += nrem;
            trunc = 1;
        }
        if (trunc) {
            static const char msg[] = "(trunc)\n";
            const size_t msglen = sizeof(msg) - 1;
            assert(lb->pos <= lb->bufsz);
            assert(lb->pos >= msglen);
            memcpy(lb->buf + lb->pos - msglen, msg, msglen);
        }
    }

    /* Flush on newline */
    if (fmt[strlen(fmt) - 1] == '\n') {
        nb__logbufFlushReal(self, lb);
    }
}

int
nb_log(
    nb_logcat cat,
    const char *fmt, ...)
{
    nb_thread self = nb_threadLookup();

    if(self){
        nb_logConfig gc = nb_threadLogConfig(self);

        if (gc->tracing.categories & cat) {
            va_list ap;
            va_start(ap, fmt);
            nb_vlogb(self, fmt, ap);
            va_end(ap);
        }
    }
    return 0;
}

int
nb_trace (const char *fmt, ...)
{
    nb_thread self = nb_threadLookup();

    if(self){
        nb_logConfig gc = nb_threadLogConfig(self);

        if (gc->tracing.categories & LOG_TRACE) {
            va_list ap;
            va_start(ap, fmt);
            nb_vlogb(self, fmt, ap);
            va_end(ap);
        }
    }

    return 0;
}
