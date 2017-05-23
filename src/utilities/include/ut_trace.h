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
#ifndef UT_TRACE_H
#define UT_TRACE_H


#define UT_TRACE_ENABLED (0) /* to enable basic backtraceing set this to 1 */

/* removed UT_TRACE_EXTENSION See OSPL-5656 and OSPL-987 */

#if UT_TRACE_ENABLED

#define UT_TRACE_MAX_STACK 64
static const char* UT_TRACE_FILE = "mem.log";

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>

#define UT_TRACE_NAME_LENGTH (128)

#define SYM_INITIALIZE(hProcess) do { \
    static BOOL bInit = FALSE;\
    if (bInit == FALSE) {\
        SymInitialize(hProcess,NULL,TRUE);\
        bInit = TRUE;\
    }\
} while(0);

#define UT_TRACE_TO_FILE(msgFormat, ...) do { \
    void *opt = os_threadMemGet(OS_THREAD_UT_TRACE);\
    HANDLE hProcess;\
    char buffer[sizeof(SYMBOL_INFO) + UT_TRACE_NAME_LENGTH * sizeof(TCHAR)];\
    PSYMBOL_INFO pSymbol;\
    void *tr[UT_TRACE_MAX_STACK];\
    USHORT usFrames, i;\
    DWORD64 dwDisplacement;\
    FILE *stream = ut_traceGetStream();\
    \
    if (opt && (*(int *)opt != 0)) {\
        hProcess = GetCurrentProcess();\
        SYM_INITIALIZE(hProcess);\
        /*SymInitialize(hProcess,NULL,TRUE);*/\
        usFrames = CaptureStackBackTrace(0, UT_TRACE_MAX_STACK, tr, NULL);\
        pSymbol = (PSYMBOL_INFO)buffer;\
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);\
        pSymbol->MaxNameLen = UT_TRACE_NAME_LENGTH;\
        fprintf(stream, msgFormat, __VA_ARGS__);\
        for (i=0;i<usFrames;i++) {\
            SymFromAddr(hProcess, (DWORD64)tr[i], &dwDisplacement, pSymbol);\
            fprintf(stream, "(%s+0x%0x) [0x%0x]\n", pSymbol->Name, dwDisplacement, pSymbol->Address);\
        }\
        fprintf(stream, "\n\n");\
        /*SymCleanup(hProcess);*/\
    }\
} while (0)

#define UT_TRACE(msgFormat, ...) do { \
    HANDLE hProcess;\
    char buffer[sizeof(SYMBOL_INFO) + UT_TRACE_NAME_LENGTH * sizeof(TCHAR)];\
    PSYMBOL_INFO pSymbol;\
    void *tr[UT_TRACE_MAX_STACK];\
    USHORT usFrames, i;\
    DWORD64 dwDisplacement;\
    FILE* stream; \
    \
    hProcess = GetCurrentProcess();\
    SYM_INITIALIZE(hProcess);\
    /*SymInitialize(hProcess,NULL,TRUE);*/\
    usFrames = CaptureStackBackTrace(0, UT_TRACE_MAX_STACK, tr, NULL);\
    pSymbol = (PSYMBOL_INFO)buffer;\
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);\
    pSymbol->MaxNameLen = UT_TRACE_NAME_LENGTH;\
    stream = fopen(UT_TRACE_FILE, "a");\
    fprintf(stream, msgFormat, __VA_ARGS__);\
    for (i=0;i<usFrames;i++) {\
        SymFromAddr(hProcess, (DWORD64)tr[i], &dwDisplacement, pSymbol);\
        fprintf(stream, "(%s+0x%0x) [0x%0x]\n", pSymbol->Name, dwDisplacement, pSymbol->Address);\
    }\
    fprintf(stream, "\n\n");\
    fflush(stream);\
    fclose(stream);\
    /*SymCleanup(hProcess);*/\
} while (0)

#else

#include <execinfo.h>
#include <stdio.h>


#define BACKTRACE_SYMBOL(tr,s)  backtrace_symbols(tr, s)

#define UT_TRACE_TO_FILE(msgFormat, ...) do { \
    void *opt = os_threadMemGet(OS_THREAD_UT_TRACE);\
    void *tr[UT_TRACE_MAX_STACK];\
    char **strs;\
    size_t s,i;\
    FILE *stream = ut_traceGetStream();\
    \
    if (opt && (*(int *)opt != 0)) {\
      s = backtrace(tr, UT_TRACE_MAX_STACK);\
      strs = BACKTRACE_SYMBOL(tr, s);\
      fprintf(stream, msgFormat, __VA_ARGS__);\
      for (i=0;i<s;i++) fprintf(stream, "%s\n", strs[i]);\
      free(strs);\
    }\
  } while (0)

#define UT_TRACE(msgFormat, ...) do { \
    void *tr[UT_TRACE_MAX_STACK];\
    char **strs;\
    size_t s,i; \
    FILE* stream; \
    s = backtrace(tr, UT_TRACE_MAX_STACK);\
    strs = BACKTRACE_SYMBOL(tr,s);\
    stream = fopen(UT_TRACE_FILE, "a");\
    fprintf(stream, msgFormat, __VA_ARGS__);\
    for (i=0;i<s;i++) fprintf(stream, "%s\n", strs[i]);\
    free(strs);\
    fflush(stream);\
    fclose(stream);\
  } while (0)
#endif

/**
 * parameters:
 * - outputPathName: defines where the trace should be written
 *               special values are <stderr> and <stdout>
 */

int
ut_traceInitialize(
    char *outputPathName);
int
ut_traceFinalize();

FILE *
ut_traceGetStream();

#else

#define UT_TRACE(msgFormat, ...)
#define ut_traceInitialize(outputPathName)
#define ut_tracefinalize()
#define ut_traceGetStream()

#endif

#endif /* UT_TRACE_H */
