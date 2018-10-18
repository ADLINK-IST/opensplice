/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef Q_OSPLBUILTIN_H
#define Q_OSPLBUILTIN_H

struct v_subscriptionInfo;
struct v_publicationInfo;
struct v_publisherCMInfo;
struct v_subscriberCMInfo;
struct v_topicInfo;
struct u_participant_s;
struct v_topicQos_s;
struct nn_xqos;
struct nn_plist;

int init_reader_qos (const struct v_subscriptionInfo *data, const struct v_dataReaderCMInfo *cmdata, struct nn_xqos *xqos, int interpret_user_data);
int init_writer_qos (const struct v_publicationInfo *data, const struct v_dataWriterCMInfo *cmdata, v_topic ospl_topic, struct nn_xqos *xqos, int interpret_user_data);
int init_reader_qos_from_topicQos (const struct v_topicQos_s *data, struct nn_xqos *xqos);
int init_topic_plist (const struct v_topicInfo *data, struct nn_plist *ps);
int init_cm_publisher_plist (const struct v_publisherCMInfo *data, const nn_guid_t *guid, struct nn_plist *ps);
int init_cm_subscriber_plist (const struct v_subscriberCMInfo *data, const nn_guid_t *guid, struct nn_plist *ps);
int init_participant_plist (const struct v_participantInfo *data, const struct v_participantCMInfo *cmdata, struct nn_plist *ps);
u_result create_builtin_topic_writers (struct u_participant_s *p);
int is_builtin_topic_writer (const struct v_gid_s *gid);
void destroy_builtin_topic_writers (void);
int is_topic_discoverable (const char *topic);

#endif
