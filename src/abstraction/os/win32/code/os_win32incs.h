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
#ifndef OS_WIN32_INCS_H
#define OS_WIN32_INCS_H

#if defined (__cplusplus)
extern "C" {
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0502
/* Minimum Windows Server 2003 SP1, Windows XP SP2 == _WIN32_WINNT_WS03 (0x0502) */
#error _WIN32_WINNT should be at least _WIN32_WINNT_WS03 (0x0502)
#endif

#include <winsock2.h>
#include <Windows.h>
#include <iphlpapi.h>

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_INCS_H */
