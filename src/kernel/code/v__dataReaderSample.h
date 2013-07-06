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

#ifndef V__DATAREADERSAMPLE_H
#define V__DATAREADERSAMPLE_H

#include "v_kernel.h"
#include "v_dataReaderSample.h"

v_dataReaderSample
v_dataReaderSampleNew(
    v_dataReaderInstance instance,
    v_message message);

void
v_dataReaderSampleFree(
    v_dataReaderSample _this);

void
v_dataReaderSampleRemoveFromLifespanAdmin(
    v_dataReaderSample _this);

void
v_dataReaderSampleWipeViews(
    v_dataReaderSample _this);
void
v_dataReaderSampleEmptyViews(
    v_dataReaderSample _this);

#endif
