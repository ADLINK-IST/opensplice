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
 */
#include <assert.h>
#include <stddef.h>

#include "ut_hex.h"

int
ut_hexenc(
    char *xbuf,
    unsigned int xlen,
    const unsigned char *buf,
    unsigned int len)
{
    int xdec, cnt = -1;
    unsigned int pos;
    char xchr;

    assert(xbuf != NULL);
    assert(buf != NULL);

    if ((xlen / 2) >= len) {
        cnt = 0;
        for (pos = 0; pos < len; pos++) {
            if (((unsigned int)cnt) < xlen) {
                xdec = (buf[pos] & 0xf0) >> 4;
                xchr = (char)(xdec < 10 ? xdec + '0' : (xdec - 10) + 'a');
                xbuf[cnt] = xchr;
            }
            cnt++;
            if (((unsigned int)cnt) < xlen) {
                xdec = (buf[pos] & 0x0f);
                xchr = (char)(xdec < 10 ? xdec + '0' : (xdec - 10) + 'a');
                xbuf[cnt] = xchr;
            }
            cnt++;
        }

        /* null terminate if space is available */
        if (((unsigned int)cnt) < xlen) {
            xbuf[cnt] = '\0';
        }
    }

    return cnt;
}
