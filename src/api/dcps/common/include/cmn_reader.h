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
#ifndef CMN_READER_H
#define CMN_READER_H

#include "v_readerSample.h"
#include "c_base.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API v_actionResult cmn_reader_action(
    c_object o,
    void *arg);

OS_API v_actionResult cmn_reader_nextInstanceAction(
    c_object o,
    void *arg);

/*
 * TODO: when OSPL-3588 is finished, remove this function and replace
 * the calls with cmn_reader_nextInstanceAction.
 */
OS_API v_actionResult cmn_reader_nextInstanceAction_OSPL3588(
    c_object o,
    void *arg);

#undef OS_API
#if defined (__cplusplus)
}
#endif

#endif /* CMN_READER_H */
