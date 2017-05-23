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

/** \file os/linux/code/os_rwlock.c
 *  \brief Linux multiple reader writer lock
 *
 * Implements multiple reader writer lock for Linux
 * by including the POSIX implementation
 */

#include <os_defs.h>
#include "os_rwlock.h"
#include "os_mutex.h"

#include "../posix/code/os_rwlock.c"
#include "../common/code/os_rwlock_attr.c"

