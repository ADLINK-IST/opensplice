#ifndef Q_FILL_MSG_QOS_H
#define Q_FILL_MSG_QOS_H

#include "os_defs.h"
#include "c_base.h"

struct v_message;

struct nn_xqos;
struct proxy_writer;
struct nn_prismtech_writer_info;

c_array new_v_message_qos (const struct nn_xqos *xqos);

#endif /* Q_FILL_MSG_QOS_H */

/* SHA1 not available (unoffical build.) */
