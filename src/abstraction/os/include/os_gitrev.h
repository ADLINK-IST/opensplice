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
#ifndef OS_GITREV_H
#define OS_GITREV_H

#include "os_macrostringify.h"

#ifdef OSPL_INNER_REV
#define OSPL_INNER_REV_STR OSPL_STRINGIFY(OSPL_INNER_REV)
#else
#define OSPL_INNER_REV_STR ""
#endif

#ifdef OSPL_OUTER_REV
#define OSPL_OUTER_REV_STR OSPL_STRINGIFY(OSPL_OUTER_REV)

#ifndef OSPL_INNER_REV
#error OSPL_OUTER_REV is defined but OSPL_INNER_REV is not set.
#endif

#else
#define OSPL_OUTER_REV_STR ""
#endif

#endif
