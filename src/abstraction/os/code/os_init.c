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

/****************************************************************
 * Initialization / Deinitialization                            *
 ****************************************************************/

/** \file os/code/os_init.c
 *  \brief Initialization / Deinitialization
 *
 * Initialization / Deinitialization provides routines for
 * initializing the OS layer claiming required resources
 * and routines to deinitialize the OS layer, releasing
 * all resources still claimed.
 */

#include "os_init.h"
#include "os_version.h"

/* include OS specific initialization implementation 		*/
#include "code/os_init.c"

const char *
os_versionString(void)
{
    return OSPL_VERSION_STR;
}
