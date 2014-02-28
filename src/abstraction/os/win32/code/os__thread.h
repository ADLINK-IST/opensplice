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
#ifndef OS_WIN32__THREAD_H
#define OS_WIN32__THREAD_H

#if defined (__cplusplus)
extern "C" {
#endif

/** \brief Initialize thread module
 */
void
os_threadModuleInit (
    void);


/** \brief Deinitialize thread module
 */
void
os_threadModuleExit (
    void);


void os_threadSetThreadName (DWORD dwThreadID, char* threadName);

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32__THREAD_H */
