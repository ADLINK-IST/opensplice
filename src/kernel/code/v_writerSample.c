

#include "v_writerSample.h"
#include "v_writer.h"
#include "os_report.h"
#define _EXTENT_
#ifdef _EXTENT_
#include "c_extent.h"
#endif

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

#ifdef _EXTENT_
    sample = v_writerSample(c_extentCreate(writer->sampleExtent));
#else
    sample = v_writerSample(c_new(writer->sampleField->type));
#endif
    v_writerSampleTemplate(sample)->message = c_keep(message);
    sample->next = NULL;
    sample->prev = NULL;
    sample->sequenceNumber = 1;
    v_writerSampleClear(sample);

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
    v_writerSample sample)
{
    assert(sample);
    assert(C_TYPECHECK(sample,v_writerSample));

    sample->resend = TRUE;
}    

