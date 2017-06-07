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

#ifndef OS_WIN32_THREAD_H
#define OS_WIN32_THREAD_H


#if defined (__cplusplus)
extern "C" {
#endif

typedef unsigned long DWORD;
typedef void * HANDLE;

typedef struct os_threadInfo_s
{
    DWORD threadId;
    HANDLE handle;
} os_threadInfo;

typedef os_threadInfo os_os_threadId;

OS_API extern os_os_threadId id_none;

#define OS_THREAD_ID_NONE id_none


#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_THREAD_H */
