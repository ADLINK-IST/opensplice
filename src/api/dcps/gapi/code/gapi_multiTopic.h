#ifndef GAPI_MULTITOPIC_H
#define GAPI_MULTITOPIC_H

#include "gapi_common.h"

#define _MultiTopic(o) ((_MultiTopic)(o))

#define gapi_multiTopicClaim(h,r) \
        (_MultiTopic(gapi_objectClaim(h,OBJECT_KIND_MULTITOPIC,r)))

#define gapi_multiTopicClaimNB(h,r) \
        (_MultiTopic(gapi_objectClaimNB(h,OBJECT_KIND_MULTITOPIC,r)))

_MultiTopic
_MultiTopicNew (
    void);

void
_MultiTopicFree (
    _MultiTopic _this);

#endif
