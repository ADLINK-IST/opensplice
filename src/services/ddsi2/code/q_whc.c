#include <assert.h>
#include <stddef.h>

#include "os_heap.h"

#include "q_unused.h"
#include "q_osplser.h"
#include "q_config.h"
#include "q_whc.h"

/* FIXME: instead of having a single hash table and rehashing once the
   hash table's load factor because too large it would be better to
   have multiple hash tables, s.t. HT0 stores seq numbers 0..N1, HT1
   seq numbers N1..N2, .. HTk seq numbers Nk..inf. Once the load
   factor of HTk becomes too large, create HT(k+1) starting at N(k+1)
   and stop adding entries in HTk.  HT(k+1) is twice as large as HTk.
   Once a hash table becomes empty, we can free it.

   That still doesn't allow reducing the memory use by the WHC once
   the number of entries in it drops significantly (but practically
   speaking, that probably means many unregisters, which is more
   likely at the end of its life. */

#define MIN_SEQHASH_SIZE_LG2 5 /* just a guess */
#define MIN_SEQHASH_SIZE (1 << MIN_SEQHASH_SIZE_LG2)

static void insert_whcn_in_hash (struct whc *whc, struct whc_node *whcn);
static int compare_seq (const void *va, const void *vb);

static const ut_avlTreedef_t whc_seq_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct whc_intvnode, avlnode), offsetof (struct whc_intvnode, min), compare_seq, 0);
static const ut_avlTreedef_t whc_tlidx_treedef =
  UT_AVL_TREEDEF_INITIALIZER_INDKEY (offsetof (struct whc_node, avlnode_tlidx), offsetof (struct whc_node, serdata), (int (*) (const void *, const void *)) serdata_cmp, 0);

static struct whc_node *whc_findmax_procedurally (const struct whc *whc)
{
  if (whc->seq_size == 0)
    return NULL;
  else if (whc->open_intv->first)
  {
    /* last is only valid iff first != NULL */
    return whc->open_intv->last;
  }
  else
  {
    struct whc_intvnode *intv = ut_avlFindPred (&whc_seq_treedef, &whc->seq, whc->open_intv);
    assert (intv->first);
    return intv->last;
  }
}

static void check_whc (const struct whc *whc)
{
  /* there's much more we can check, but it gets expensive quite
     quickly: all nodes but open_intv non-empty, non-overlapping and
     non-contiguous; min & maxp1 of intervals correct; each interval
     contiguous; all samples in seq & in seqhash; tlidx \subseteq seq;
     seq-number ordered list correct; &c. */
  assert (whc->open_intv != NULL);
  assert (whc->open_intv == ut_avlFindMax (&whc_seq_treedef, &whc->seq));
  assert (whc->seqhash_size >= MIN_SEQHASH_SIZE);
  assert (whc->seqhash_size == 1 << whc->seqhash_size_lg2);
#if 0 /* relation need not hold if we are out of memory */
  assert (whc->seq_size <= 3 * whc->seqhash_size / 4);
#endif
  assert (ut_avlFindSucc (&whc_seq_treedef, &whc->seq, whc->open_intv) == NULL);
  if (whc->maxseq_node)
  {
    assert (whc->maxseq_node->next_seq == NULL);
  }
  if (whc->open_intv->first)
  {
    assert (whc->open_intv->last);
    assert (whc->maxseq_node == whc->open_intv->last);
    assert (whc->open_intv->min < whc->open_intv->maxp1);
    assert (whc->maxseq_node->seq + 1 == whc->open_intv->maxp1);
  }
  else
  {
    assert (whc->open_intv->min == whc->open_intv->maxp1);
  }
  assert (whc->maxseq_node == whc_findmax_procedurally (whc));
}

static int hash_seq (os_int64 seq, int seqhash_size_lg2)
{
  /* we hash the lower 32 bits, on the assumption that with 4 billion
     samples in between there won't be significant correlation */
#define UINT64_CONST(x, y, z) (((os_uint64) (x) * 1000000 + (y)) * 1000000 + (z))
  const os_uint64 c = UINT64_CONST (16292676, 669999, 574021);
#undef UINT64_CONST
  const os_uint32 x = (os_uint32) seq;
  return (int) ((x * c) >> (64 - seqhash_size_lg2));
}

static void resize_seqhash (struct whc *whc, int newsize_lg2)
{
  const int newsize = 1 << newsize_lg2;
  struct whc_node **new;
  if ((new = os_malloc (newsize * sizeof (*new))) != NULL)
  {
    /* when out of memory, better to continue with a hash table that's
       too small or ridiculously large than it is to crash */
    struct whc_node *whcn;
    /* using insert_whcn_in_hash, so have to set all seqhash-related
       "globals" as if we have an empty hash */
    os_free (whc->seqhash);
    whc->seqhash_size_lg2 = newsize_lg2;
    whc->seqhash_size = newsize;
    whc->seqhash = new;
    memset (new, 0, newsize * sizeof (*new));
    assert (new[0] == NULL); /* strictly speaking, that needn't be true :) */
    for (whcn = whc->maxseq_node; whcn; whcn = whcn->prev_seq)
      insert_whcn_in_hash (whc, whcn);
  }
}

static void insert_whcn_in_hash (struct whc *whc, struct whc_node *whcn)
{
  /* by definition, seq is unique, so we simply compute the hash key &
     insert it */
  int idx = hash_seq (whcn->seq, whc->seqhash_size_lg2);
  whcn->next_hash = whc->seqhash[idx];
  whcn->prev_hash = NULL;
  if (whcn->next_hash)
    whcn->next_hash->prev_hash = whcn;
  whc->seqhash[idx] = whcn;
}

static void remove_whcn_from_hash (struct whc *whc, struct whc_node *whcn)
{
  /* precondition: whcn is in hash */
  int idx = hash_seq (whcn->seq, whc->seqhash_size_lg2);
  if (whcn->next_hash)
    whcn->next_hash->prev_hash = whcn->prev_hash;
  if (whcn->prev_hash)
    whcn->prev_hash->next_hash = whcn->next_hash;
  else
    whc->seqhash[idx] = whcn->next_hash;
}

struct whc_node *whc_findseq (const struct whc *whc, os_int64 seq)
{
  int idx = hash_seq (seq, whc->seqhash_size_lg2);
  struct whc_node *whcn = whc->seqhash[idx];
  while (whcn && whcn->seq != seq)
    whcn = whcn->next_hash;
  return whcn;
}

static int compare_seq (const void *va, const void *vb)
{
  const os_int64 *a = va;
  const os_int64 *b = vb;
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

struct whc *whc_new (int transient_local, int keep_last1)
{
  struct whc *whc;
  struct whc_intvnode *intv;

  if ((whc = os_malloc (sizeof (*whc))) == NULL)
    return NULL;
  whc->transient_local = transient_local ? 1 : 0;
  whc->keep_last1 = keep_last1 ? 1 : 0;
  whc->seq_size = 0;
  whc->tlidx_size = 0;
  whc->max_drop_seq = 0;

  /* hash table */
  whc->seqhash_size_lg2 = MIN_SEQHASH_SIZE_LG2;
  whc->seqhash_size = MIN_SEQHASH_SIZE;
  if ((whc->seqhash = os_malloc (whc->seqhash_size * sizeof (*whc->seqhash))) == NULL)
  {
    os_free (whc);
    return NULL;
  }
  memset (whc->seqhash, 0, whc->seqhash_size * sizeof (*whc->seqhash));

  /* seq interval tree: always has an "open" node */
  ut_avlInit (&whc_seq_treedef, &whc->seq);
  if ((intv = os_malloc (sizeof (*intv))) == NULL)
  {
    os_free (whc->seqhash);
    os_free (whc);
    return NULL;
  }
  intv->min = intv->maxp1 = 1;
  intv->first = intv->last = NULL;
  ut_avlInsert (&whc_seq_treedef, &whc->seq, intv);
  whc->open_intv = intv;
  whc->maxseq_node = NULL;

  /* hack */
  whc->freelist = NULL;

  /* transient-local tree */
  ut_avlInit (&whc_tlidx_treedef, &whc->tlidx);

  check_whc (whc);
  return whc;
}

static void free_whc_node (struct whc *whc, struct whc_node *whcn)
{
  serdata_unref (whcn->serdata);
  whcn->next_seq = whc->freelist;
  whc->freelist = whcn;
}

void whc_free (struct whc *whc)
{
  check_whc (whc);
  /* simply free all samples in the whc (without worrying about any
     datastructures) */
  {
    struct whc_node *whcn = whc->maxseq_node;
    while (whcn)
    {
      struct whc_node *tmp = whcn;
      whcn = whcn->prev_seq;
      free_whc_node (whc, tmp);
    }
  }
  /* we know all samples have been freed already -- now delete the
     interval tree using a fairly expensive technique to avoid having
     to change the AVL tree implementation and to avoid using the AVL
     tree's free function support (that I want to get rid of
     eventually) */
  {
    struct whc_intvnode *intv;
    while ((intv = ut_avlFindMin (&whc_seq_treedef, &whc->seq)) != NULL)
    {
      ut_avlDelete (&whc_seq_treedef, &whc->seq, intv);
      os_free (intv);
    }
  }

  while (whc->freelist)
  {
    struct whc_node *tmp = whc->freelist;
    whc->freelist = tmp->next_seq;
    os_free (tmp);
  }

  os_free (whc->seqhash);
  os_free (whc);
}

int whc_empty (const struct whc *whc)
{
  return whc->seq_size == 0;
}

os_int64 whc_min_seq (const struct whc *whc)
{
  /* precond: whc not empty */
  const struct whc_intvnode *intv;
  check_whc (whc);
  assert (!whc_empty (whc));
  intv = ut_avlFindMin (&whc_seq_treedef, &whc->seq);
  /* not empty, open node may be anything but is (by definition)
     findmax, and whc is claimed to be non-empty, so min interval
     can't be empty */
  assert (intv->maxp1 > intv->min);
  return intv->min;
}

struct whc_node *whc_findmax (const struct whc *whc)
{
  check_whc (whc);
  return (struct whc_node *) whc->maxseq_node;
}

os_int64 whc_max_seq (const struct whc *whc)
{
  /* precond: whc not empty */
  check_whc (whc);
  assert (!whc_empty (whc));
  assert (whc->maxseq_node != NULL);
  return whc->maxseq_node->seq;
}

static struct whc_node *find_nextseq_intv (struct whc_intvnode **p_intv, const struct whc *whc, os_int64 seq)
{
  struct whc_node *n;
  struct whc_intvnode *intv;
  if ((n = whc_findseq (whc, seq)) == NULL)
  {
    /* don't know seq => lookup interval with min > seq (intervals are
       contiguous, so if we don't know seq, an interval [X,Y) with X <
       SEQ < Y can't exist */
#ifndef NDEBUG
    {
      struct whc_intvnode *predintv = ut_avlLookupPredEq (&whc_seq_treedef, &whc->seq, &seq);
      assert (predintv == NULL || predintv->maxp1 <= seq);
    }
#endif
    if ((intv = ut_avlLookupSuccEq (&whc_seq_treedef, &whc->seq, &seq)) == NULL) {
      assert (ut_avlLookupPredEq (&whc_seq_treedef, &whc->seq, &seq) == whc->open_intv);
      return NULL;
    } else if (intv->min < intv->maxp1) { /* only if not empty interval */
      assert (intv->min > seq);
      *p_intv = intv;
      return intv->first;
    } else { /* but note: only open_intv may be empty */
      assert (intv == whc->open_intv);
      return NULL;
    }
  }
  else if (n->next_seq == NULL)
  {
    assert (n == whc->maxseq_node);
    return NULL;
  }
  else
  {
    assert (whc->maxseq_node != NULL);
    assert (n->seq < whc->maxseq_node->seq);
    n = n->next_seq;
    *p_intv = ut_avlLookupPredEq (&whc_seq_treedef, &whc->seq, &n->seq);
    return n;
  }
}

os_int64 whc_next_seq (const struct whc *whc, os_int64 seq)
{
  struct whc_node *n;
  struct whc_intvnode *intv;
  check_whc (whc);
  if ((n = find_nextseq_intv (&intv, whc, seq)) == NULL)
    return MAX_SEQ_NUMBER;
  else
    return n->seq;
}

static void delete_one_from_tlidx (struct whc *whc, struct whc_node *whcn)
{
  assert (whcn->in_tlidx);
  assert (whc->tlidx_size > 0);
  whcn->in_tlidx = 0;
  ut_avlDelete (&whc_tlidx_treedef, &whc->tlidx, whcn);
  whc->tlidx_size--;
}

static void swapnode_in_tlidx (struct whc *whc, struct whc_node *old, struct whc_node *new)
{
  assert (old->in_tlidx);
  assert (whc->tlidx_size > 0);
  old->in_tlidx = 0;
  new->in_tlidx = 1;
  ut_avlSwapNode (&whc_tlidx_treedef, &whc->tlidx, old, new);
}

static void whc_delete_tlidx_entry_helper (void *vnode)
{
  struct whc_node *node = vnode;
  assert (node->in_tlidx);
  node->in_tlidx = 0;
}

void whc_downgrade_to_volatile (struct whc *whc)
{
  os_int64 old_max_drop_seq;

  /* We only remove them from whc->tlidx: we don't remove them from
     whc->seq yet.  That'll happen eventually.  */
  check_whc (whc);

  /* Do things specific for this mode transition ... */
  if (whc->transient_local)
  {
    if (whc->keep_last1)
    {
      /* no need to do anything special: need to keep the tlidx for
         dropping overwritten samples, and remove_acked_messages will
         drop all samples no longer needed */
    }
    else
    {
      /* not maintaining a tlidx anymore; note that
         whc_delete_tlidx_entry_helper doesn't actually "free"
         anything, but avl_free is simply a synonym for a walk with
         callback after locating the next node, and a resetting of the
         root pointer. */
      ut_avlFree (&whc_tlidx_treedef, &whc->tlidx, whc_delete_tlidx_entry_helper);
      whc->tlidx_size = 0;
    }
  }

  /* ... then update the mode */
  whc->transient_local = 0;

  /* Immediately drop them from the WHC (used to delay it until the
     next ack); but need to make sure remove_acked_messages processes
     them all. */
  old_max_drop_seq = whc->max_drop_seq;
  whc->max_drop_seq = 0;
  whc_remove_acked_messages (whc, old_max_drop_seq);
  assert (whc->max_drop_seq == old_max_drop_seq);
}

static int whc_delete_one_intv (struct whc *whc, struct whc_intvnode **p_intv, struct whc_node **p_whcn)
{
  /* Removes *p_whcn, possibly deleting or splitting *p_intv, as the
     case may be.  Does *NOT* update whc->seq_size.  *p_intv must be
     the interval containing *p_whcn (&& both must actually exist).

     Returns:
     - 0 if delete failed (only possible cause is memory exhaustion),
     in which case *p_intv & *p_whcn are undefined;
     - 1 if successful, in which case *p_intv & *p_whcn are set
     correctly for the next sample in sequence number order */
  struct whc_intvnode *intv = *p_intv;
  struct whc_node *whcn = *p_whcn;
  assert (whcn->seq >= intv->min && whcn->seq < intv->maxp1);
  *p_whcn = whcn->next_seq;

  /* If it is in the tlidx, take it out.  Transient-local data never gets here */
  if (whcn->in_tlidx)
    delete_one_from_tlidx (whc, whcn);

  /* Take it out of seqhash and out of the list ordered on
     sequence numbers. */
  remove_whcn_from_hash (whc, whcn);
  if (whcn->prev_seq)
    whcn->prev_seq->next_seq = whcn->next_seq;
  if (whcn->next_seq)
    whcn->next_seq->prev_seq = whcn->prev_seq;

  /* We may have introduced a hole & have to split the interval
     node, or we may have nibbled of the first one, or even the
     last one. */
  if (whcn == intv->first)
  {
    if (whcn == intv->last && intv != whc->open_intv)
    {
      struct whc_intvnode *tmp = intv;
      *p_intv = ut_avlFindSucc (&whc_seq_treedef, &whc->seq, intv);
      /* only sample in interval and not the open interval => delete interval */
      ut_avlDelete (&whc_seq_treedef, &whc->seq, tmp);
      os_free (tmp);
    }
    else
    {
      intv->first = whcn->next_seq;
      intv->min++;
      assert (intv->first != NULL || intv == whc->open_intv);
      assert (intv->min < intv->maxp1 || intv == whc->open_intv);
      assert ((intv->first == NULL) == (intv->min == intv->maxp1));
    }
  }
  else if (whcn == intv->last)
  {
    /* well, at least it isn't the first one & so the interval is
       still non-empty and we don't have to drop the interval */
    assert (intv->min < whcn->seq);
    assert (whcn->prev_seq);
    assert (whcn->prev_seq->seq + 1 == whcn->seq);
    intv->last = whcn->prev_seq;
    intv->maxp1--;
    *p_intv = ut_avlFindSucc (&whc_seq_treedef, &whc->seq, intv);
  }
  else
  {
    /* somewhere in the middle => split the interval (ideally,
       would split it lazily, but it really is a transient-local
       issue only, and so we can (for now) get away with splitting
       it greedily */
    struct whc_intvnode *new_intv;
    ut_avlIPath_t path;

    if ((new_intv = os_malloc (sizeof (*new_intv))) == NULL)
    {
      /* Now what?  Re-insert and return seems most elegant.
         Re-inserting it doesn't require memory allocations (a
         possible hash resize will continue in case of a memory
         exhaustion) and will always succeed.  Note: whcn is in
         the middle of the interval, so prev & next must be both
         be non-NULL. */
      assert (whcn->prev_seq);
      assert (whcn->next_seq);
      whcn->prev_seq->next_seq = whcn;
      whcn->next_seq->prev_seq = whcn;
      insert_whcn_in_hash (whc, whcn);
      return 0;
    }

    /* new interval starts at the next node */
    assert (whcn->next_seq);
    assert (whcn->seq + 1 == whcn->next_seq->seq);
    new_intv->first = whcn->next_seq;
    new_intv->last = intv->last;
    new_intv->min = whcn->seq + 1;
    new_intv->maxp1 = intv->maxp1;
    intv->last = whcn->prev_seq;
    intv->maxp1 = whcn->seq;
    assert (intv->min < intv->maxp1);
    assert (new_intv->min < new_intv->maxp1);

    /* insert new node & continue the loop with intv set to the
       new interval */
    if (ut_avlLookupIPath (&whc_seq_treedef, &whc->seq, &new_intv->min, &path) != NULL)
      assert (0);
    ut_avlInsertIPath (&whc_seq_treedef, &whc->seq, new_intv, &path);

    if (intv == whc->open_intv)
      whc->open_intv = new_intv;
    *p_intv = new_intv;
  }

  /* free sample and continue with next -- note that *p_whcn and
     *p_intv hav been updated already (*p_intv conditionally, of
     course) */
  free_whc_node (whc, whcn);
  return 1;
}

static int whc_delete_one (struct whc *whc, struct whc_node *whcn)
{
  struct whc_intvnode *intv;
  intv = ut_avlLookupPredEq (&whc_seq_treedef, &whc->seq, &whcn->seq);
  assert (intv != NULL);
  return whc_delete_one_intv (whc, &intv, &whcn);
}

int whc_remove_acked_messages (struct whc *whc, os_int64 max_drop_seq)
{
  struct whc_intvnode *intv;
  struct whc_node *whcn;
  int ndropped = 0;
  assert (max_drop_seq < MAX_SEQ_NUMBER);
  assert (max_drop_seq >= whc->max_drop_seq);

  check_whc (whc);
  whcn = find_nextseq_intv (&intv, whc, whc->max_drop_seq);
  while (whcn && whcn->seq <= max_drop_seq)
  {
    if (whc->transient_local && whcn->in_tlidx)
    {
      /* quickly skip over samples in tlidx */
      if (whcn == intv->last)
        intv = ut_avlFindSucc (&whc_seq_treedef, &whc->seq, intv);
      whcn = whcn->next_seq;
    }
    else if (whc_delete_one_intv (whc, &intv, &whcn))
    {
      ndropped++;
    }
    else
    {
      break;
    }
  }

  assert (ndropped <= whc->seq_size);
  whc->seq_size -= ndropped;
  assert (whc->tlidx_size <= whc->seq_size);

  /* lazy people do it this way: */
  whc->maxseq_node = whc_findmax_procedurally (whc);
  whc->max_drop_seq = max_drop_seq;

  /* resize seqhash if the load factor becomes very low */
  if (0)
  {
    int newsize_lg2 = whc->seqhash_size_lg2;
    while (newsize_lg2 > MIN_SEQHASH_SIZE_LG2 && whc->seq_size < (1 << newsize_lg2) / 4)
      newsize_lg2--;
    if (newsize_lg2 < whc->seqhash_size_lg2)
      resize_seqhash (whc, whc->seqhash_size_lg2 - 1);
  }

  return ndropped;
}

struct whc_node *whc_findkey (const struct whc *whc, const struct serdata *serdata_key)
{
  check_whc (whc);
  return ut_avlLookup (&whc_tlidx_treedef, &whc->tlidx, serdata_key);
}

static struct whc_node *whc_insert_seq (struct whc *whc, os_int64 seq, serdata_t serdata)
{
  struct whc_node *newn = NULL;

  if ((newn = whc->freelist) == NULL)
    newn = os_malloc (sizeof (*newn));
  else
    whc->freelist = newn->next_seq;
  newn->seq = seq;
  newn->in_tlidx = 0; /* initial state, may be changed */
  newn->last_rexmit_ts = 0;
  newn->serdata = serdata_ref (serdata);
  newn->next_seq = NULL;
  newn->prev_seq = whc->maxseq_node;
  if (newn->prev_seq)
    newn->prev_seq->next_seq = newn;
  whc->maxseq_node = newn;

  insert_whcn_in_hash (whc, newn);

  if (whc->open_intv->first == NULL)
  {
    /* open_intv is empty => reset open_intv */
    whc->open_intv->min = seq;
    whc->open_intv->maxp1 = seq + 1;
    whc->open_intv->first = whc->open_intv->last = newn;
  }
  else if (whc->open_intv->maxp1 == seq)
  {
    /* no gap => append to open_intv */
    whc->open_intv->last = newn;
    whc->open_intv->maxp1++;
  }
  else
  {
    /* gap => need new open_intv */
    struct whc_intvnode *intv1;
    ut_avlIPath_t path;
    intv1 = os_malloc (sizeof (*intv1));
    intv1->min = seq;
    intv1->maxp1 = seq + 1;
    intv1->first = intv1->last = newn;
    if (ut_avlLookupIPath (&whc_seq_treedef, &whc->seq, &seq, &path) != NULL)
      assert (0);
    ut_avlInsertIPath (&whc_seq_treedef, &whc->seq, intv1, &path);
    whc->open_intv = intv1;
  }

  whc->seq_size++;

  /* maybe grow the hash table */
  if (whc->seq_size >= 3 * whc->seqhash_size / 4)
  {
    /* a load factor of 3/4 is a bit much for a hash table, but we
       don't really do lookups very often, so I guess it'll do */
    resize_seqhash (whc, whc->seqhash_size_lg2 + 1);
  }
  /*nn_log (LC_TRACE, "add_msg_to_whc: insert %p in seq\n", newn);*/
  return newn;
}

int whc_insert (struct whc *whc, os_int64 max_drop_seq, os_int64 seq, serdata_t serdata)
{
  struct whc_node *newn = NULL;
  check_whc (whc);

  assert (max_drop_seq < MAX_SEQ_NUMBER);
  assert (max_drop_seq >= whc->max_drop_seq);

  /* Seq must be greater than what is currently stored. Usually it'll
     be the next sequence number, but if there are no readers
     temporarily, a gap may be among the possibilities */
  assert (whc_empty (whc) || seq > whc_max_seq (whc));

  newn = whc_insert_seq (whc, seq, serdata);

  if (whc->keep_last1 || whc->transient_local) /* Maintaining a tlidx */
  {
    ut_avlIPath_t path;
    struct whc_node *oldtln;
    assert (newn != NULL);
    if ((oldtln = ut_avlLookupIPath (&whc_tlidx_treedef, &whc->tlidx, serdata, &path)) != NULL)
    {
      /* If unregister, simply delete oldtln (unregisters can't be
         kept around or the history keeps growing). Else, swap the new
         one in. */
      if (serdata->v.msginfo.statusinfo & NN_STATUSINFO_UNREGISTER)
        delete_one_from_tlidx (whc, oldtln);
      else
        swapnode_in_tlidx (whc, oldtln, newn);

      if (whc->keep_last1 || oldtln->seq <= max_drop_seq)
      {
        struct whc_node *newmax;
        newmax = (oldtln == whc->maxseq_node) ? oldtln->prev_seq : whc->maxseq_node;
        if (whc_delete_one (whc, oldtln))
        {
          whc->seq_size--;
          whc->maxseq_node = newmax;
        }
      }
    }
    else if (!(serdata->v.msginfo.statusinfo & NN_STATUSINFO_UNREGISTER))
    {
      /* Ignore unregisters, but insert anything else */
      newn->in_tlidx = 1;
      ut_avlInsertIPath (&whc_tlidx_treedef, &whc->tlidx, newn, &path);
      whc->tlidx_size++;
    }
  }

  return 0;
}


/* SHA1 not available (unoffical build.) */
