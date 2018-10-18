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
#ifndef Q_MTREADER_H
#define Q_MTREADER_H

#include "ut_avl.h"
#include "ut_fibheap.h"

struct mtreader;
struct u_topic_s;

enum mtr_sample_state {
  MTR_SST_NEW,
  MTR_SST_UPD,
  MTR_SST_DEL
};

struct mtr_iter {
  ut_avlIter_t it;
};

struct mtr_sample {
  ut_avlNode_t avlnode;
  ut_fibheapNode_t fhnode;
  v_gid gid;
  unsigned ntopics;
  enum mtr_sample_state state;
  nn_mtime_t texpire;
  unsigned flag;
#if __STDC_VERSION__ >= 199901L
  struct v_message_s *vmsg[/* ntopics */];
#else
  struct v_message_s *vmsg[1];
#endif
};

/* NOTE: Only for readers of built-in topics, that have the <systemId,localId> of a GID as key,
   and that should have the full GID as a key */

struct mtreader *new_mtreader (unsigned ntopics, struct u_topic_s **topics);
void delete_mtreader (struct mtreader *mtr);

int update_mtreader (struct mtreader *mtr, const struct mtr_sample *outputs[2], const struct u_topic_s *tp, v_state sample_state, struct v_message_s *vmsg);

/* Updates UDATA field in mtr_sample corresponding to KEY; returns ERR_INVALID_DATA if no such sample */
int update_mtreader_setflag (struct mtreader *mtr, const v_gid *key, unsigned flag);

/* Allocates and returns array of live, complete samples for which pred(&TP.FIELD,B) && !FLAG, return value is length. All returned samples are in state UPD. */
int query_mtreader (const struct mtreader *mtr, const struct mtr_sample ***result, const struct u_topic_s *tp, const char *field, int (*pred) (const void *a, const void *b), const void *b);

const struct mtr_sample *mtr_first (const struct mtreader *mtr, struct mtr_iter *it);
const struct mtr_sample *mtr_next (struct mtr_iter *it);

#endif
