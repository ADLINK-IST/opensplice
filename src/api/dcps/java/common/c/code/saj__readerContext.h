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

#ifndef SAJ__READERCONTEXT_H
#define SAJ__READERCONTEXT_H

#include "saj_copyCache.h"
#include "cmn_samplesList.h"

typedef struct {
    JNIEnv *javaEnv;
    saj_copyCache copyCache;
    jobject jreader;
    jlong uReader;
    jobject dataSeqHolder;
    jobject infoSeqHolder;
    jobject dataSeq;
    jobject infoSeq;
    unsigned int dataSeqLen;
    unsigned int infoSeqLen;
    unsigned int max_samples;
    cmn_samplesList samplesList;
    sajParDemContext pardemCtx;
    jlong CDRCopy;
} saj_readerContext;

#endif /* SAJ__READERCONTEXT_H */
