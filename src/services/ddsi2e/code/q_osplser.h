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
#ifndef NN_OSPLSER_H
#define NN_OSPLSER_H

#include "kernelModuleI.h"
#include "ddsi_ser.h"

extern sertopic_t osplser_topic4u;
extern sertopic_t osplser_topicpmd;
extern c_type osplser_topicpmd_type;
extern c_type osplser_topicpmd_value_type;

int osplser_init (void);
void osplser_fini (void);

sertopic_t deftopic (C_STRUCT(v_topic) const * const ospl_topic);
c_type topic_type (const struct sertopic * tp);
v_topic topic_ospl_topic (const struct sertopic * tp);
void freetopic (sertopic_t tp);

serdata_t serialize_raw (serstatepool_t pool, const sertopic_t tp, const void *data, unsigned statusinfo, nn_wctime_t timestamp, const struct nn_prismtech_writer_info *wri);
serdata_t serialize (serstatepool_t pool, const struct sertopic * tp, C_STRUCT (v_message) const *msg);
serdata_t serialize_key (serstatepool_t pool, const struct sertopic * tp, C_STRUCT (v_message) const *msg);
serdata_t serialize_empty (serstatepool_t pool, unsigned statusinfo, C_STRUCT (v_message) const *msg);

v_message deserialize (const struct sertopic * topic, const void *vsrc, size_t vsrcsize);
v_message deserialize_from_key (const struct sertopic * topic, const void *vsrc, size_t vsrcsize);
v_message deserialize_from_keyhash (const struct sertopic * topic, const void *vsrc, size_t vsrcsize);

int prettyprint_serdata (char *dst, const int dstsize, const struct serdata *serdata);
int prettyprint_raw (char *dst, const int dstsize, const struct sertopic * topic, const void *vsrc, size_t vsrcsize);

unsigned serdata_hash (const struct serdata *a);
int serdata_cmp (const struct serdata * a, const struct serdata * b);
void serdata_keyhash (const struct serdata * d, char keyhash[16]);
int serdata_verify (serdata_t serdata, C_STRUCT (v_message) const *data);

void serstate_set_key (serstate_t st, int justkey, const void *key);
void serstate_init (serstate_t st, const struct sertopic * topic);
void serstate_free (serstate_t st);

#endif /* NN_OSPLSER_H */
