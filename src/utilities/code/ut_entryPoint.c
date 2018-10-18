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

#include "ut_entryPoint.h"
#include "vortex_os.h"
#include "os_defs.h"

void *
ut_entryPointWrapper(void *arg)
{
    struct ut_entryPointWrapperArg *mwa = (struct ut_entryPointWrapperArg *)arg;
    os_address result;

    result = (os_address) mwa->entryPoint(mwa->argc,mwa->argv);

    os_free(mwa->argv);
    os_free(mwa);

    return (void *) ((os_address) result);
}
