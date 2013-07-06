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
