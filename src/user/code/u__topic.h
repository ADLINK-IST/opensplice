
#ifndef U__TOPIC_H
#define U__TOPIC_H

#include "u_topic.h"

u_result
u_topicInit (
    u_topic _this);

u_result
u_topicDeinit (
    u_topic _this);

u_result
u_topicClaim(
    u_topic _this,
    v_topic *topic);

u_result
u_topicRelease(
    u_topic _this);

#endif
