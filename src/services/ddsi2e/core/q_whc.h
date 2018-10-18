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
#ifndef Q_WHC_H
#define Q_WHC_H

#include "ut_avl.h"
#include "ut_hopscotch.h"
#include "q_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define MAX_SEQ_NUMBER 0x7fffffffffffffffll

struct serdata;
struct nn_plist;
struct whc_idxnode;

struct whc_node {
  struct whc_node *next_seq; /* next in this interval */
  struct whc_node *prev_seq; /* prev in this interval */
  struct whc_idxnode *idxnode; /* NULL if not in index */
  unsigned idxnode_pos; /* index in idxnode.hist */
  os_int64 seq;
  struct nn_plist *plist; /* 0 if nothing special */
  unsigned unacked: 1; /* counted in whc::unacked_bytes iff 1 */
  nn_mtime_t last_rexmit_ts;
  unsigned rexmit_count;
  struct serdata *serdata;
};

struct whc_intvnode {
  ut_avlNode_t avlnode;
  os_int64 min;
  os_int64 maxp1;
  struct whc_node *first; /* linked list of seqs with contiguous sequence numbers [min,maxp1) */
  struct whc_node *last; /* valid iff first != NULL */
};

struct whc_idxnode {
  os_int64 prune_seq;
  unsigned headidx;
#if __STDC_VERSION__ >= 199901L
  struct whc_node *hist[];
#else
  struct whc_node *hist[1];
#endif
};
  
struct whc {
  unsigned seq_size;
  os_size_t unacked_bytes;
  os_size_t sample_overhead;
  unsigned is_transient_local: 1;
  unsigned hdepth; /* 0 = unlimited */
  unsigned tldepth; /* 0 = disabled/unlimited (no need to maintain an index if KEEP_ALL <=> is_transient_local + tldepth=0) */
  unsigned idxdepth; /* = max(hdepth, tldepth) */
  os_int64 max_drop_seq; /* samples in whc with seq <= max_drop_seq => transient-local */
  struct whc_intvnode *open_intv; /* interval where next sample will go (usually) */
  struct whc_node *maxseq_node; /* NULL if empty; if not in open_intv, open_intv is empty */
  struct whc_node *freelist; /* linked via whc_node::next_seq */
  struct ut_hh *seq_hash;
  struct ut_hh *idx_hash;
  ut_avlTree_t seq;
};

struct whc *whc_new (int is_transient_local, unsigned hdepth, unsigned tldepth, os_size_t sample_overhead);
void whc_free (struct whc *whc);
int whc_empty (const struct whc *whc);
os_int64 whc_min_seq (const struct whc *whc);
os_int64 whc_max_seq (const struct whc *whc);
os_int64 whc_next_seq (const struct whc *whc, os_int64 seq);
os_size_t whc_unacked_bytes (struct whc *whc);

struct whc_node *whc_findseq (const struct whc *whc, os_int64 seq);
struct whc_node *whc_findmax (const struct whc *whc);
struct whc_node *whc_findkey (const struct whc *whc, const struct serdata *serdata_key);

/* min_seq is lowest sequence number that must be retained because of
   reliable readers that have not acknowledged all data */
/* max_drop_seq must go soon, it's way too ugly. */
/* plist may be NULL or os_malloc'd, WHC takes ownership of plist */
int whc_insert (struct whc *whc, os_int64 max_drop_seq, os_int64 seq, struct nn_plist *plist, struct serdata *serdata);
void whc_downgrade_to_volatile (struct whc *whc);
unsigned whc_remove_acked_messages (struct whc *whc, os_int64 max_drop_seq);

#if defined (__cplusplus)
}
#endif

#endif /* Q_WHC_H */
