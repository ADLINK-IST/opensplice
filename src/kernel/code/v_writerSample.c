/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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


#include "v_writerSample.h"
#include "v_writer.h"
#include "v_group.h"
#include "os_report.h"

#define v_sample(s) ((v_sample)(s))

v_writerSample
v_writerSampleNew(
    v_writer writer,
    v_message message)
{
    v_writerSample sample;

    assert(writer != NULL);
    assert(C_TYPECHECK(writer,v_writer));
    assert(message);
    assert(C_TYPECHECK(message,v_message));

    sample = c_new(writer->sampleType);
    v_writerResendItem(sample)->kind = V_RESENDITEM_WRITERSAMPLE;
    assert(C_TYPECHECK(sample,v_writerSample));

    v_writerSampleTemplate(sample)->message = c_keep(message);

    /* c_new(...) does a memset(0), so the rest of the fields are all OK */
    assert(v_writerResendItem(sample)->scope == V_RESEND_NONE);
    return sample;
}

/* Precondition: protect the sample yourself */
void
v_writerSampleSetResendScope (
    v_writerSample sample,
    v_resendScope resendScope)
{
    assert(sample);
    assert(C_TYPECHECK(sample,v_writerSample));

    v_writerResendItem(sample)->scope = resendScope;
}

c_bool
v_writerSampleHasBeenSentBefore(
    v_writerSample _this)
{
    assert(_this);
    assert(C_TYPECHECK(_this,v_writerSample));

    return v_writerResendItem(_this)->scope != V_RESEND_NONE;
}
