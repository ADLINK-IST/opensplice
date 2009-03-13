
#ifndef GAPI_CONTENTFILTEREDTOPIC_H
#define GAPI_CONTENTFILTEREDTOPIC_H

#include "gapi_common.h"

#define _ContentFilteredTopic(o) ((_ContentFilteredTopic)(o))

#define gapi_contentFilteredTopicClaim(h,r) \
        (_ContentFilteredTopic(gapi_objectClaim(h,OBJECT_KIND_CONTENTFILTEREDTOPIC,r)))

#define gapi_contentFilteredTopicClaimNB(h,r) \
        (_ContentFilteredTopic(gapi_objectClaimNB(h,OBJECT_KIND_CONTENTFILTEREDTOPIC,r)))

#define _ContentFilteredTopicAlloc() \
        (_ContentFilteredTopic(_ObjectAlloc(OBJECT_KIND_CONTENTFILTEREDTOPIC, \
                                            C_SIZEOF(_ContentFilteredTopic), \
                                            NULL)))

_ContentFilteredTopic
_ContentFilteredTopicNew (
    const gapi_char      *topicName,
    _Topic                relatedTopic,
    const gapi_char      *expression,
    const gapi_stringSeq *parameters,
    _DomainParticipant    participant);

void
_ContentFilteredTopicFree (
    _ContentFilteredTopic _this);

gapi_boolean
_ContentFilteredTopicPrepareDelete (
    _ContentFilteredTopic _this);

_Topic
_ContentFilteredTopicGetRelatedTopic (
    _ContentFilteredTopic _this);

c_value *
_ContentFilteredTopicParameters (
    _ContentFilteredTopic topic
    );

#endif
