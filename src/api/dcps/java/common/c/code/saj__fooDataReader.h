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

#include "gapi.h"
#include "saj_utilities.h"

int
saj_fooDataReaderParallelDemarshallingMain(
    JNIEnv * env,
    sajParDemContext pdc);

int
saj_fooDataReaderParallelDemarshallingContextFinalize(
    JNIEnv * env,
    jobject jdatareader,
    sajParDemContext pdc);

int
saj_fooDataReaderSetParallelReadThreadCount(
    JNIEnv * env,
    jobject jdatareader,
    int value);

int
saj_fooDataReaderSetCDRCopy(
    JNIEnv * env,
    jobject jdatareader,
    int value);

void
saj_dataReaderCopy (
    gapi_dataSampleSeq *samples,
    gapi_readerInfo *arg
    );
