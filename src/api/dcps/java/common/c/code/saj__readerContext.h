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

#ifndef SAJ__READERCONTEXT_H
#define SAJ__READERCONTEXT_H

typedef struct {
    JNIEnv *javaEnv;
    saj_copyCache copyCache;
    jobject jreader;
    jobject dataSeqHolder;
    jobject infoSeqHolder;
    jobject dataSeq;
    jobject infoSeq;
    unsigned int dataSeqLen;
    unsigned int infoSeqLen;
    unsigned int max_samples;
    sajParDemContext pardemCtx;
    jlong CDRCopy;
} saj_readerContext;

#endif /* SAJ__READERCONTEXT_H */
