/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
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
