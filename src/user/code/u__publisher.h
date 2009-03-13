
#ifndef U__PUBLISHER_H
#define U__PUBLISHER_H

#include "u_publisher.h"

u_result
u_publisherInit (
    u_publisher _this);

u_result
u_publisherDeinit (
    u_publisher _this);

u_result
u_publisherClaim(
    u_publisher _this,
    v_publisher *publisher);

u_result
u_publisherRelease(
    u_publisher _this);

#endif
