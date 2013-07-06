/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef UT__TRACE_H
#define UT__TRACE_H


#define UT_TRACE_ENABLED (0) /* to enable basic backtraceing set this to 1 */


/* enable extended version of the backtrace symbols function that can resolve function names and line numbers
 * This version requires the binutils develop library (-lbfd -liberty)
 * this extension can be enabled by doing 'make UT_TRACE_EXTENSION=1' in the OSPL_HOME/src/utilities directory.
 * WARNING this extended function may lead to more overhead in a running system due to extra lookup function calls
 * When enabling this option you also need to recompile the section in which you included this tracing.
 * */
#define UT_TRACE_EXTENSION (0) /* to enable extended tracing set this to 1 */


#if UT_TRACE_ENABLED

#include <execinfo.h>
#include <stdio.h>

#define UT_TRACE_MAX_STACK 64
static const char* UT_TRACE_FILE = "mem.log";


#if UT_TRACE_EXTENSION
#include "ut_backtrace-symbols.h"
#define BACKTRACE_SYMBOL(tr,s) backtrace_symbols_extension(tr, s)
#else
#define BACKTRACE_SYMBOL(tr,s)  backtrace_symbols(tr, s)
#endif


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

#endif /* UT__TRACE_H */
