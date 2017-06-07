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
#ifndef S_MISC_H
#define S_MISC_H

#if defined (__cplusplus)
extern "C" {
#endif

#define S_THREAD_MAIN              "main"
#define S_THREAD_KERNELMANAGER     "OSPL Kernel Manager"
#define S_THREAD_RESENDMANAGER     "OSPL Builtin Resend Manager"
#define S_THREAD_GARBAGE_COLLECTOR "OSPL Garbage Collector"
#define S_THREAD_LEASE_RENEW_THREAD "OSPL Lease Renew Thread"
#define S_THREAD_HEARTBEAT_THREAD   "OSPL Heartbeat Thread"
#define S_THREAD_C_AND_M_COMMANDMANAGER "OSPL C&M Command Manager"
#define S_THREAD_DURABILITYCLIENT   "OSPL Durability Client"

#define S_THREAD_CNT_MAX            (16)

#if defined (__cplusplus)
}
#endif


#endif /* S_MISC_H */
