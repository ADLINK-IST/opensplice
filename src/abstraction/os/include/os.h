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
/*******************************************************************
 * Interface definition for all of SPLICE-DDS OS layer functions   *
 *******************************************************************/

/** \file os.h
 *  \brief Aggregate OS layer include files
 *
 * os.h aggregates all OS layer include files. By including
 * this file, all type definitions and services are available.
 */

#ifndef OS_OS_H
#define OS_OS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"
#include "os_init.h"
#include "os_abstract.h"
#include "os_mutex.h"
#include "os_rwlock.h"
#include "os_cond.h"
#include "os_time.h"
#include "os_thread.h"
#include "os_process.h"
#include "os_signal.h"
#include "os_sharedmem.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_library.h"
/* #include "os_if.h"
 * May NEVER be included by this file!
 */

#ifndef OPENSPLICE_MAIN
#define OPENSPLICE_MAIN(n) int main (int argc, char ** argv)
#endif

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_OS_H */
