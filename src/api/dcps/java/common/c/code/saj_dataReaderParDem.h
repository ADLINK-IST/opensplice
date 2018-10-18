/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#ifndef SAJ_DATAREADERPARDEM_H
#define SAJ_DATAREADERPARDEM_H

#include "saj__readerContext.h"

struct sajParDemContext_s{
    os_ushort       nrofWorkers; /* Number of worker threads. This is configured #threads - 1 (application thread participates as well) */
    struct {
        os_ushort threshold; /* Number of samples threshold at which parallelization should be performed */
        os_ushort block;     /* Number of samples to be read without inter-thread sync */
    } heuristics;
    struct {
        pa_uint32_t     nextIndex; /* Next index to be read by worker thread */
        unsigned int    len;       /* The number of samples to be copied */
        jobject         dataSeq;   /* Weak-ref to the data-sqeuence to copy-out to */
        jobject         infoSeq;   /* Weak-ref to the info-sqeuence to copy-out to */
        saj_copyCache   copyCache; /* Reference to the copyCache */
        void (*copyOut)(void *, void *);   /* Copy-out function pointer */
        cmn_samplesList samplesList;
        jobject         jreader;   /* Weak-ref to the reader Object */
        jlong           uEntity;   /* The user entity */
    } copy_info;
    os_mutex        superMtx;  /* Mutex that should be held by the 'main' read thread in order to serialize concurrent reads on a parralelized reader */
    os_mutex        mtx;
    os_cond         workerCnd; /* Condition for notifying spawning thread that worker is operational */
    os_cond         startCnd;  /* Condition for notifying worker threads that work is to be done */
    os_cond         readyCnd;  /* Condition for notifying application thread that all work is done */
    os_ushort       readyCnt;  /* Counter for readyCnd broadcast */
    /* The parity is used for the loop-condition for readyCnd. Members of the
     * copy_info struct may be reused, so don't provide a safe loop-condition.
     * The parity toggles every loop and since it is not changed until all threads
     * are waiting on the next loop, it is a safe loop condition. */
    os_boolean      parity;
    os_ushort       terminate; /* Flag used to signal blocked threads to stop. */
};

saj_returnCode
saj_dataReaderParDemStack_copy_out(
    saj_readerContext *ctx);

int
saj_fooDataReaderParallelDemarshallingMain(
    JNIEnv * env,
    sajParDemContext pdc);

saj_returnCode
saj_fooDataReaderSetParallelReadThreadCount(
    JNIEnv * env,
    jobject jdatareader,
    int value);

#endif
