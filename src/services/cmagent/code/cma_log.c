/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "cma__log.h"
#include "cma__thread.h"
#include "cma__configuration.h"

#include "vortex_os.h"
#include "os_report.h"

#define DATE_SIZE (OS_CTIME_R_BUFSIZE)
#define MAX_TIMESTAMP_LENGTH (10 + 1 + 6)
#define MAX_TID_LENGTH       (15)
#define MAX_HDR_LENGTH       (DATE_SIZE + 1 + MAX_TIMESTAMP_LENGTH + 1 + MAX_TID_LENGTH + 2)

#define BUF_OFFSET MAX_HDR_LENGTH

static void
cma__logbufFlushReal(
    cma_logbuf _this,
    cma_thread self) __nonnull_all__;

static void
cma__logbufFlush(
    cma_logbuf _this,
    cma_thread self) __nonnull_all__;

static void
cma_vlogb(
    cma_thread self,
    const char *fmt,
    va_list ap) __nonnull((1, 2)) __attribute_format__((printf, 2, 0));

cma_logbuf
cma_logbufNew(void)
{
    cma_logbuf lb = os_malloc(sizeof(*lb));

    cma_logbufInit(lb);

    return lb;
}

void
cma_logbufInit(
    cma_logbuf _this)
{
    assert(_this);

    _this->bufsz = sizeof(_this->buf);
    _this->pos = BUF_OFFSET;
    _this->tstamp = -1;
    _this->buf[_this->pos] = 0;
}

static void
cma__logbufFlushReal(
    cma_logbuf _this,
    cma_thread self)
{
    cma_logConfig gc;

    cma_objectIsValidKind(self, CMA_OBJECT_THREAD);
    assert(_this);

    gc = cma_threadLogConfig(self);

    if (gc->tracing.file) {
        char hdr[MAX_HDR_LENGTH + 1];
        char date[DATE_SIZE];
        int n, tsec, tusec;
        os_timeW time;

        if (_this->tstamp < 0) {
            _this->tstamp = cma_timeNow();
        }

        cma_time_sec_usec_conv(_this->tstamp, &tsec, &tusec);
        OS_TIMEW_SET_VALUE(time, _this->tstamp);
        os_ctimeW_r(&time, date, DATE_SIZE);

        _this->tstamp = -1;
        n = snprintf(hdr, sizeof(hdr), "%s %d.%06d/%*.*s: ",
            date, tsec, tusec, MAX_TID_LENGTH, MAX_TID_LENGTH, cma_threadName(self));
        assert(n > 0 && n <= BUF_OFFSET);
        memcpy(_this->buf + BUF_OFFSET - (size_t)n, hdr, (size_t)n);
        (void)fwrite(_this->buf + BUF_OFFSET - (size_t)n, 1, _this->pos - BUF_OFFSET + (size_t)n, gc->tracing.file);
        (void)fflush(gc->tracing.file);
    }
    _this->pos = BUF_OFFSET;
    _this->buf[_this->pos] = 0;
}

static void
cma__logbufFlush(
    cma_logbuf _this,
    cma_thread self)
{
    cma_objectIsValidKind(self, CMA_OBJECT_THREAD);
    assert(_this);

    if (_this->pos > BUF_OFFSET) {
        if (_this->pos < (int)sizeof(_this->buf)) {
            _this->buf[_this->pos++] = '\n';
        } else {
            _this->buf[sizeof(_this->buf) - 1] = '\n';
        }
        cma__logbufFlushReal(_this, self);
    }
}

void
cma_logbufFree(
    cma_logbuf _this,
    cma_thread self)
{
    cma_objectIsValidKind(self, CMA_OBJECT_THREAD);
    assert(_this);

    cma__logbufFlush(_this, self);
    os_free(_this);
}

static void
cma_vlogb(
    cma_thread self,
    const char *fmt,
    va_list ap)
{
    int n, trunc = 0;
    size_t nrem;
    cma_logbuf _this;

    assert(self);
    assert(fmt);

    if (*fmt == 0) {
        return;
    }
    _this = cma_threadLogbuf(self);

    /* Copy message to log buffer */
    nrem = _this->bufsz - _this->pos;
    if (nrem > 0) {
        n = os_vsnprintf(_this->buf + _this->pos, nrem, fmt, ap);
        if (n >= 0 && (size_t)n < nrem) {
            _this->pos += (size_t)n;
        } else {
            _this->pos += nrem;
            trunc = 1;
        }

        if (trunc) {
            static const char msg[] = "(trunc)\n";
            const size_t msglen = sizeof(msg) - 1;
            assert(_this->pos <= _this->bufsz);
            assert(_this->pos >= msglen);
            memcpy(_this->buf + _this->pos - msglen, msg, msglen);
        }
    }

    /* Flush on newline */
    if (fmt[strlen(fmt) - 1] == '\n') {
        cma__logbufFlushReal(_this, self);
    }
}

int
cma_log(
    cma_logcat cat,
    const char *fmt, ...)
{
    cma_thread self = cma_threadLookup();

    if (self) {
        cma_logConfig gc = cma_threadLogConfig(self);

        if (gc->tracing.categories & cat) {
            va_list ap;
            va_start(ap, fmt);
            cma_vlogb(self, fmt, ap);
            va_end(ap);
        }
    }
    return 0;
}

int
cma_trace(
    const char *fmt, ...)
{
    cma_thread self = cma_threadLookup();

    if (self) {
        cma_logConfig gc = cma_threadLogConfig(self);

        if (gc->tracing.categories & LOG_TRACE) {
            va_list ap;
            va_start(ap, fmt);
            cma_vlogb(self, fmt, ap);
            va_end(ap);
        }
    }

    return 0;
}
