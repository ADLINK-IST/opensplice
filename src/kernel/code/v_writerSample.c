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


#include "v_writerSample.h"
#include "v_writer.h"
#include "os_report.h"

#define v_sample(s) ((v_sample)(s))

void
_v_writerSampleClear(
    v_writerSample sample)
{
    assert(sample);
    assert(C_TYPECHECK(sample,v_writerSample));

    sample->resend = FALSE;
    sample->decayCount = 0;
}


v_writerSample
_v_writerSampleNew(
    v_writer writer,
    v_message message)
{
    v_writerSample sample;

    assert(writer != NULL);
    assert(C_TYPECHECK(writer,v_writer));
    assert(message);
    assert(C_TYPECHECK(message,v_message));

    sample = v_writerSample(c_new(writer->sampleType));

    if (sample) {
        v_writerSampleTemplate(sample)->message = c_keep(message);
        sample->next = NULL;
        sample->prev = NULL;
        sample->sequenceNumber = 1;
        sample->sentBefore = FALSE;
        v_writerSampleClear(sample);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_writerSampleNew",0,
                  "Failed to allocate sample.");
        assert(FALSE);
    }

    assert(C_TYPECHECK(sample,v_writerSample));

    return sample;
}


/* Precondition: protect the sample yourself */
v_writerSampleStatus
_v_writerSampleGetStatus(
    v_writerSample sample)
{
    v_writerSampleStatus result;

    assert(sample);
    assert(C_TYPECHECK(sample,v_writerSample));

    if ((c_long)sample->resend == TRUE) {
        /* Someone has rejected the sample, resend it */
        result = V_SAMPLE_RESEND;
    } else {
        if ((c_long)sample->decayCount > 0) {
            result = V_SAMPLE_KEEP;
        } else {
            result = V_SAMPLE_RELEASE;
        }
    }

    return result;
}


/* Precondition: protect the sample yourself */
void
_v_writerSampleRelease(
    v_writerSample sample)
{
    assert(sample);
    assert(C_TYPECHECK(sample,v_writerSample));

    if (sample->decayCount > 0) {
       sample->decayCount--;
    }
}


/* Precondition: protect the sample yourself */
void
_v_writerSampleKeep (
    v_writerSample sample,
    c_long count)
{
    assert(sample);
    assert(C_TYPECHECK(sample,v_writerSample));
    assert(count >= 0);

    sample->decayCount = count;
}


/* Precondition: protect the sample yourself */
void
_v_writerSampleResend (
    v_writerSample sample,
    v_resendScope resendScope)
{
    assert(sample);
    assert(C_TYPECHECK(sample,v_writerSample));

    sample->resend = TRUE;
    sample->resendScope = resendScope;
}

void
_v_writerSampleSetSentBefore(
    v_writerSample _this,
    c_bool sentBefore)
{
    assert(_this);
    assert(C_TYPECHECK(_this,v_writerSample));

    _this->sentBefore = sentBefore;
}

c_bool
_v_writerSampleHasBeenSentBefore(
    v_writerSample _this)
{
    assert(_this);
    assert(C_TYPECHECK(_this,v_writerSample));

    return _this->sentBefore;
}
