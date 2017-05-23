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
#include "os__win32EntropyHook.h"
#include "os_report.h"
#include "os_errno.h"
#include <Windows.h>
#include <Wincrypt.h>

#define RANDOMSIZE 16

void os_win32EntropyHook( struct _SHA256_CTX *ctx )
{
    char randombuf[RANDOMSIZE];
    HCRYPTPROV cp;
    int res;

    res = CryptAcquireContext(&cp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
    if (!res) {
        OS_REPORT(OS_ERROR,"os_win32EntropyHook",0,"CryptAcquireContext: %d %x\n", (int) res, (unsigned) os_getErrno());
    } else {
        memset(randombuf, 0, sizeof randombuf);
        res = CryptGenRandom(cp, RANDOMSIZE, randombuf);
        if (res) {
            os_SHA256Update(ctx, randombuf, RANDOMSIZE);
        }
        CryptReleaseContext(cp, 0);
    }
}
