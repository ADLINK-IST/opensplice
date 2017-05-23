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

#include <string.h>

#include "os_uniqueNodeId.h"
#include "os_sha2.h"
#include "os_time.h"
#include "os_stdlib.h"
#include "os_process.h"
#include "os_abstract.h"
#include "os_atomics.h"

static pa_uint32_t os_uniqueNodeIdGetCounter = PA_UINT32_INIT (0);
static os_platformEntropyHook *os_addPlatformEntropy = 0;

void os_uniqueIdSetEntropyHook(os_platformEntropyHook peh)
{
    os_addPlatformEntropy = peh;
}

os_uint32 os_uniqueNodeIdGet(os_uint32 min, os_uint32 max, size_t entropySize, const void *entropy)
{
   /* DNS name is max 253 Chars */
    char hostnamebuf[256];
    const os_timeW currenttime = os_timeWGet();
    const int pid = os_procIdSelf();
    char digest[SHA256_DIGEST_STRING_LENGTH];
    SHA256_CTX context, save;
    unsigned mask;
    unsigned x;
    int first;

    assert (1 <= min && min <= max && max <= 0x7fffffff);
    os_gethostname(hostnamebuf, sizeof(hostnamebuf));

    /* Mask so we get smallest power of two >= the requested range, max mask is therefore 2^31-1.
 *        We then draw numbers from our "random" generator limited by the mask until we get one within
 *               the desired range. Given a uniform generator on [0,2^N-1], this yields a uniform random
 *                      generator on [0,M <= 2^N-1] with a retry probability that is halved with each attempt.
 *
 *                             This depends on the (unvalidated) assumption that the output of SHA256 is very close to a
 *                                    uniform random generator. */
    mask = 1u;
    while (mask < max-min+1) {
        mask *= 2u;
    }
    mask--;

    os_SHA256Init(&context);
    os_SHA256Update(&context, (unsigned char *)&currenttime, sizeof(currenttime));
    os_SHA256Update(&context, (unsigned char *)&pid, sizeof(pid));
    os_SHA256Update(&context, (unsigned char *)hostnamebuf, strlen(hostnamebuf));
    if (entropy != NULL && entropySize != 0) {
        os_SHA256Update(&context, entropy, entropySize);
    }
    if (os_addPlatformEntropy != 0) {
        os_addPlatformEntropy(&context);
    }

    first = 1;
    do {
        const os_uint32 cnt = pa_inc32_nv(&os_uniqueNodeIdGetCounter);
        int i;
        if (first) {
            memcpy (&save, &context, sizeof (SHA256_CTX));
            first = 0;
        } else {
            memcpy (&context, &save, sizeof (SHA256_CTX));
        }
        os_SHA256Update(&context, (unsigned char *)&cnt, sizeof(cnt));
        os_SHA256End(&context, digest);
        for (i = 0, x = 0; i < 8; i++)
        {
            unsigned c;
            if (digest[i] >= '0' && digest[i] <= '9') {
                c = (unsigned) ((unsigned char) digest[i] - '0');
            } else if (digest[i] >= 'a' && digest[i] <= 'f') {
                c = (unsigned) ((unsigned char) digest[i] - 'a' + 10);
            } else {
                assert (digest[i] >= 'A' && digest[i] <= 'F');
                c = (unsigned) ((unsigned char) digest[i] - 'A' + 10);
            }
            x = (x << 4) | c;
        }
        x &= mask;
    } while (x > max - min);
    return min + x;
}
