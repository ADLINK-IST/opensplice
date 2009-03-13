
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
v_dataReaderSampleWipeViews(
    v_dataReaderSample _this);
void
v_dataReaderSampleEmptyViews(
    v_dataReaderSample _this);

#endif
