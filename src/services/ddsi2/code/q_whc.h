#ifndef Q_WHC_H
#define Q_WHC_H

#include "ut_avl.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define MAX_SEQ_NUMBER 0x7fffffffffffffffll

struct serdata;

struct whc_node {
  ut_avlNode_t avlnode_tlidx;
  struct whc_node *next_seq; /* next in this interval */
  struct whc_node *prev_seq; /* prev in this interval */
  struct whc_node *next_hash; /* next in hash chain */
  struct whc_node *prev_hash; /* prev in hash chain */
  os_int64 seq;
  int in_tlidx;
  os_int64 last_rexmit_ts;
  struct serdata *serdata;
};

struct whc_intvnode {
  ut_avlNode_t avlnode;
  os_int64 min;
  os_int64 maxp1;
  struct whc_node *first; /* linked list of seqs with contiguous sequence numbers [min,maxp1) */
  struct whc_node *last; /* valid iff first != NULL */
};

struct whc {
  int seq_size;
  int tlidx_size;
  unsigned transient_local: 1;
  unsigned keep_last1: 1;
  os_int64 max_drop_seq; /* samples in whc with seq <= max_drop_seq => transient-local */
  struct whc_intvnode *open_intv; /* interval where next sample will go (usually) */
  struct whc_node *maxseq_node; /* NULL if empty; if not in open_intv, open_intv is empty */
  int seqhash_size_lg2;
  int seqhash_size;
  struct whc_node **seqhash;
  struct whc_node *freelist; /* linked via whc_node::next_seq */
  ut_avlTree_t seq; /* reliable|transient_local */
  ut_avlTree_t tlidx; /* transient_local */
};

struct whc *whc_new (int transient_local, int keep_last1);
void whc_free (struct whc *whc);
int whc_empty (const struct whc *whc);
os_int64 whc_min_seq (const struct whc *whc);
os_int64 whc_max_seq (const struct whc *whc);
int whc_number_of_unacked_samples (const struct whc *whc);
os_int64 whc_next_seq (const struct whc *whc, os_int64 seq);

struct whc_node *whc_findseq (const struct whc *whc, os_int64 seq);
struct whc_node *whc_findmax (const struct whc *whc);
struct whc_node *whc_findkey (const struct whc *whc, const struct serdata *serdata_key);

/* min_seq is lowest sequence number that must be retained because of
   reliable readers that have not acknowledged all data */
/* max_drop_seq must go soon, it's way too ugly. */
int whc_insert (struct whc *whc, os_int64 max_drop_seq, os_int64 seq, struct serdata *serdata);
void whc_downgrade_to_volatile (struct whc *whc);
int whc_remove_acked_messages (struct whc *whc, os_int64 max_drop_seq);

#if defined (__cplusplus)
}
#endif

#endif /* Q_WHC_H */

/* SHA1 not available (unoffical build.) */
