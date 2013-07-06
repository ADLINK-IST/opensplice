#ifndef GENERICREADER_H
#define GENERICREADER_H

#include "c_typebase.h"
#include "u_domain.h"
#include "topicSacDcps.h"

#if defined (__cplusplus)
extern "C" {
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(readerConfig);
C_STRUCT(readerConfig) {
    DDS_SubscriberQos   *subscriberQos;
    DDS_TopicQos        *topicQos;
    char                *topicName;
    int                 maxSamples;
    int                 retryCount;
    os_time             retryDelay;
    DDS_boolean         waitHistoricalData;
    DDS_Duration_t      historyTimeout;
    DDS_boolean         takeSamples;
    FILE                *output;
};

C_CLASS(refSample);
C_STRUCT(refSample) {
    simpleTopic_Type1   sample;
    DDS_SampleInfo      info;
    int                 count;
};

C_CLASS(sampleStore);
C_STRUCT(sampleStore) {
    refSample           *samples;
    int                 size;
};

#define readerConfig(p) ((readerConfig)(p))
#define sampleStore(p)  ((sampleStore)(p))
#define refSample(p)    ((refSample)(p))

#if defined (__cplusplus)
}
#endif

#endif /* GENERICREADER_H */
