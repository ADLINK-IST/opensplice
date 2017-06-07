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
#include "os_uniqueNodeId.h"
#include "os__linuxEntropyHook.h"

void os_linuxEntropyHook( struct _SHA256_CTX *ctx )
{
    FILE *rndfile = fopen("/dev/urandom", "r");
    if (rndfile != NULL)
    {
        os_uchar buf[4];
        size_t n = fread(buf, sizeof (buf), 1, rndfile);
        if (n > 0) {
            os_SHA256Update(ctx, (os_uchar *)buf, n);
        }
        fclose(rndfile);
    }
}
