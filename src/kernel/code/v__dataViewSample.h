
#ifndef V__DATAVIEWSAMPLE_H
#define V__DATAVIEWSAMPLE_H

#include "v_kernel.h"

v_dataReaderSample
v_dataReaderSampleNew (
    v_dataReaderInstance instance,
    v_message message);

void
v_dataReaderSampleFree (
    v_dataReaderSample _this);

#endif
