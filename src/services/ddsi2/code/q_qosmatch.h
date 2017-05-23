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
#ifndef Q_QOSMATCH_H
#define Q_QOSMATCH_H

#if defined (__cplusplus)
extern "C" {
#endif

struct nn_xqos;

int partition_match_based_on_wildcard_in_left_operand (const struct nn_xqos *a, const struct nn_xqos *b, const char **realname);
int partitions_match_p (const struct nn_xqos *a, const struct nn_xqos *b);
int is_wildcard_partition (const char *str);

/* Returns -1 on success, or QoS id of first missmatch (>=0) */

os_int32 qos_match_p (const struct nn_xqos *rd, const struct nn_xqos *wr);

#if defined (__cplusplus)
}
#endif

#endif /* Q_QOSMATCH_H */

/* SHA1 not available (unoffical build.) */
