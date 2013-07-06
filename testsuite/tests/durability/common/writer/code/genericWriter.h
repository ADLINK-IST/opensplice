#ifndef GENERICWRITER_H
#define GENERICWRITER_H

#include "c_typebase.h"
#include "topicSacDcps.h"
#include "u_domain.h"
#if defined (__cplusplus)
extern "C" {
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(writerConfig);
C_STRUCT(writerConfig) {
    DDS_PublisherQos    *publisherQos;
    DDS_TopicQos        *topicQos;
    char                *topicName;
    int                 sampleCount;
    os_time             writeDelay;
};

#define writerConfig(p)                  ((writerConfig)(p))

#if defined (__cplusplus)
}
#endif

#endif /* GENERICWRITER_H */
