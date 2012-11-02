/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef UT__TRACE_H
#define UT__TRACE_H

#ifdef UT_TRACE_ENABLED
#include <execinfo.h>
#include <stdio.h>

#define UT_TRACE_MAX_STACK 64

#define UT_TRACE(msgFormat, ...) do { \
    void *opt = os_threadMemGet(OS_THREAD_UT_TRACE);\
    void *tr[UT_TRACE_MAX_STACK];\
    char **strs;\
    size_t s,i;	\
    FILE *stream = ut_traceGetStream();\
    \
    if (opt && (*(int *)opt != 0)) {\
      s = backtrace(tr, UT_TRACE_MAX_STACK);\
      strs = backtrace_symbols(tr, s);\
      fprintf(stream, msgFormat, __VA_ARGS__);				\
      for (i=0;i<s;i++) fprintf(stream, "%s\n", strs[i]);\
      free(strs);\
    }\
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
