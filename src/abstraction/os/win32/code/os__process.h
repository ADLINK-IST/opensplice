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
#ifndef OS__PROCESS_H
#define OS__PROCESS_H

#include "os_process.h"

void os_processModuleInit(void);
void os_processModuleExit(void);
void os_procCallExitHandlers(void);
HANDLE os_procIdToHandle(os_procId);
os_procId os_handleToProcId(HANDLE);
#endif /*OS__PROCESS_H*/
