#ifndef _REENTRANT
#define _REENTRANT 1
#endif

#include <ctype.h>
#include <stddef.h>

#if HAVE_VALGRIND && ! defined (NDEBUG)
#include <memcheck.h>
#define USE_VALGRIND 1
#else
#define USE_VALGRIND 0
#endif

#include "os_heap.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_thread.h"
#include "os_abstract.h"
#include "os_if.h"

#include "nn_avl.h"
#include "nn_osplser.h"
#include "nn_protocol.h"
#include "nn_rtps.h"
#include "nn_misc.h"

#include "nn_config.h"
#include "nn_log.h"

#include "nn_mlv.h"
#include "nn_align.h"
#include "nn_plist.h"
#include "nn_unused.h"
#include "nn_radmin.h"
#include "nn_bitset.h"

#include "sysdeps.h"

extern nn_sequence_number_t nn_toSN (os_int64 n);

/* Arguably, this upper bound is overkill. But this is copy-pasteable
   to wherever we need it. */
#define MAX_TREEHEIGHT (3 * CHAR_BIT * sizeof (void *) / 2)

/* OVERVIEW ------------------------------------------------------------

   The receive path of DDSI2 has any number of receive threads that
   accept data from sockets and (synchronously) push it up the
   protocol stack, potentially offloading processing to other threads
   at some point.  In particular, delivery of data can safely be
   offloaded.

   Each receive thread MUST process each message synchronously to the
   point where all additional indexing and other administrative data
   derived from the message has been stored in memory.  This storage
   is _always_ adjacent to the message that caused it.  Also, once it
   finishes processing a message, the reference count of that message
   may not be incremented anymore.

   In practice that means the receive thread can do everything by
   itself (handling acks and heartbeats, handling discovery,
   delivering data to the kernel), or it can offload everything but
   defragmentation and reordering.

   The data structures and functions in this file are all concerned
   with the storage of messages in buffers, organising their parts
   into ordered chains of fragments of (DDS) samples, reordering them
   into chains of consecutive samples, and queueing these chains for
   further processing.

   Storage is organised in the following hierarchy; rdata is included
   because it is is very intimately involved with the reference
   counting.  For the indexing structures for defragmenting and
   reordering messages, see RDATA, DEFRAG and REORDER below.

   nn_rbufpool

                One or more rbufs. Currently, an rbufpool is owned by
                a single receive thread, and only this thread may
                allocate memory from the rbufs contained in the pool
                and increment reference counts to the messages in it,
                while all threads may decrement these reference counts
                / release memory from it.

                (It is probably better to share the pool amongst all
                threads and make the rbuf the thing owned by this
                thread; and in fact the buffer pool isn't really
                necessary 'cos they handle multiple messages and
                therefore the malloc/free overhead is negligible.  It
                does provide a convenient location for storing some
                constant data.)

   nn_rbuf

                Largish buffer for receiving several UDP packets and
                for storing partially decoded and indexing information
                directly following the packet.

   nn_rmsg

                One message in an rbuf; the layout for one message is
                rmsg, raw udp packet, decoder stuff mixed with rdata,
                defragmentation and message reordering state.  One
                rbuf can contain many messages.

   nn_rdata

                Represents one Data/DataFrag submessage.  These
                contain some administrative data & point to the
                corresponding part of the message, and are referenced
                by the defragmentation and reordering (defrag, reorder)
                tables and the delivery queues.

   Each rmsg contains a reference count tracking all references to all
   rdatas contained in that message.  All data for one message in the
   rbuf (raw data, decoder info, &c.) is dependent on the refcount of
   the rmsg: once that reference count goes to zero _all_ dependent
   stuff becomes invalid immediately.

   As noted, the receive thread that owns the rbuf is the only one
   allowed to add data to it, which implies that this thread must do
   all defragmenting and reordering synchronously.  Delivery can be
   offloaded to another thread, and it remains to be seen which thread
   is best used for deserializing the data.

   The main advantage of restricting the adding of data to the buffer
   to the buffer's owning thread is that it allows us to simply append
   decoding information to the message as it becomes available while
   processing the message, without risking interference from another
   thread.  This includes decoded parameter lists/inline QoS settings,
   defragmenting information, &c.

   Once the synchronous processing of a message (a UDP packet) is
   completed, every adminstrative thing related to that message is
   contained in a single block of memory, and can be released very
   easily, regardless of whether the rbuf is a circular buffer, has a
   minimalistic heap inside it, or is simply discarded when the end is
   reached.

   Each rdata (submessage) that has been delivered (or need never be
   delivered) is not referenced anywhere and will therefore not
   contribute to rmsg::refcount, so once all rdatas of an rmsg have
   been delivered, rmsg::refcount will drop to 0.  If all submessages
   are processed by the receive thread, or delivery is delegated to
   other threads that happen to finish doing so before the receive
   thread is done processing the message, the message can be discarded
   trivially by not even updating the memory allocation info in the
   rbuf.

   Just creating an rdata is not sufficient reason for the reference
   count in the corresponding rmsg to be incremented: that happens
   once the defragmenter decides to not throw it away (either because
   it stores it or because it returns it for forwarding to reordering
   or delivery).  (Which is possible because both defragmentation and
   reordering are synchronous.)

   While synchronously processing the message, the reference count is
   biased by 2**31 just so we can detect some illegal activities.
   Furthermore, while still synchronous, each rdata contributes the
   number of actual references to the message plus 2**20 to the
   refcount.  This second bias allows delaying accounting for the
   actual references until after processing all reorder admins, saving
   us from having to update them potentially many times.

   The space needed for processing a message is limited: a UDP packet
   is never larger than 64kB (and it seems very unwise to actually use
   such large packets!), and there is only a finite amount of data
   that gets added to it while interpreting the message.  Although the
   exact amount is not yet known, it seems very unlikely that the
   decoding data for one packet would exceed 64kB size, though one had
   better be careful just in case.  So a maximum RMSG size of 128kB
   and an RBUF size of 1MB should be quite reasonable.

   Sequence of operations:

     receive_thread ()
     {
       ...
       rbpool = nn_rbufpool_new (1MB, 128kB)
       ...

       while ...
         rmsg = nn_rmsg_new (rbpool)
         actualsize = recvfrom (rmsg.payload, 64kB)
         nn_rmsg_setsize (rmsg, actualsize)
         process (rmsg)
         nn_rmsg_commit (rmsg)

       ... ensure no references to any buffer in rbpool exist ...
       nn_rbufpool_free (rbpool)
       ...
     }

   If there are no outstanding references to the message, commit()
   simply discards it and new() returns the same address next time
   round.

   Processing of a single message in process() is roughly as follows:

     for rdata in each Data/DataFrag submessage in rmsg
       sampleinfo.seq = XX;
       sampleinfo.fragsize = XX;
       sampleinfo.size = XX;
       sampleinfo.(others) = XX if first fragment, else not important
       sample = nn_defrag_rsample (pwr->defrag, rdata, &sampleinfo)
       if sample
         fragchain = nn_rsample_fragchain (sample)
         refcount_adjust = 0;

         if send-to-proxy-writer-reorder
           if nn_reorder_rsample (&sc, pwr->reorder, sample, &refcount_adjust)
              == DELIVER
             deliver-to-group (pwr, sc)
         else
           for (m in out-of-sync-reader-matches)
             sample' = nn_reorder_rsample_dup (rmsg, sample)
             if nn_reorder_rsample (&sc, m->reorder, sample, &refcount_adjust)
                == DELIVER
               deliver-to-reader (m->reader, sc)

         nn_fragchain_adjust_refcount (fragchain, refcount_adjust)
       fi
     rof

   Where deliver-to-x() must of course decrement refcounts after
   delivery when done, using nn_fragchain_unref().  See also REORDER
   for the subtleties of the refcount game.

   Note that there is an alternative to all this trickery with
   fragment chains and deserializing off these fragments chains:
   allocating sufficient memory upon reception of the first fragment,
   and then just memcpy'ing the bytes in, with a simple bitmask to
   keep track of which fragments have been received and which have not
   yet been.

   _The_ argument against that is a very unreliable network with huge
   messages: the way we do it here never needs more than a constant
   factor over what is actually received, whereas the simple
   alternative would blow up nearly instantaneously.  Maybe not if you
   drop samples halfway through defragmenting aggressively, but then
   you can't get anything through anymore if there are multiple
   writers.

   Gaps and Heartbeats prune the defragmenting index and are (when
   needed) stored as intervals of specially marked rdatas in the
   reordering indices.

   The procedure for a Gap is:

     for a Gap [a,b] in rmsg
       defrag_notegap (a, b+1)
       refcount_adjust = 0
       gap = nn_rdata_newgap (rmsg);
       if nn_reorder_gap (&sc, reorder, gap, a, b+1, &refcount_adjust)
         deliver-to-group (pwr, sc)
       for (m in out-of-sync-reader-matches)
         if nn_reorder_gap (&sc, m->reorder, gap, a, b+1, &refcount_adjust)
           deliver-to-reader (m->reader, sc)
       nn_fragchain_adjust_refcount (gap, refcount_adjust)

   Note that a Gap always gets processed both by the primary and by
   the secondary reorder admins.  This is because it covers a range.

   A heartbeat is similar, except that a heartbeat [a,b] results in a
   gap [1,a-1]. */

/* RBUFPOOL ------------------------------------------------------------ */

struct nn_rbufpool {
  /* An rbuf pool is owned by a receive thread, and that thread is the
     only allocating rmsgs from the rbufs in the pool. Any thread may
     be releasing buffers to the pool as they become empty.

     Currently, we only have maintain a current rbuf, which gets
     replaced when allocating a new one from it fails. Any rbufs that
     are released are freed completely if different from the current
     one.

     Could trivially be done lockless, except that it requires
     compare-and-swap, and we don't have that. But it hardly ever
     happens anyway. */
  os_mutex lock;
  struct nn_rbuf *current;
  int rbuf_size;
  os_uint32 max_rmsg_size;
#ifndef NDEBUG
  /* Thread that owns this pool, so we can check that no other thread
     is calling functions only the owner may use. */
  os_threadId owner_tid;
#endif
};

static struct nn_rbuf *nn_rbuf_new (struct nn_rbufpool *rbufpool);
static void nn_rbuf_release (struct nn_rbuf *rbuf);

#ifndef NDEBUG
#define ASSERT_RBUFPOOL_OWNER(rbp) (assert (os_threadEqual (os_threadIdSelf (), (rbp)->owner_tid)))
#else
#define ASSERT_RBUFPOOL_OWNER(rbp) ((void) (0))
#endif

struct nn_rbufpool *nn_rbufpool_new (int rbuf_size, int max_rmsg_size)
{
  struct nn_rbufpool *rbp;
  os_mutexAttr mattr;

  assert (max_rmsg_size > 0);

  if ((rbp = os_malloc (sizeof (*rbp))) == NULL)
    goto fail_rbp;
#ifndef NDEBUG
  rbp->owner_tid = os_threadIdSelf ();
#endif

  os_mutexAttrInit (&mattr);
  mattr.scopeAttr = OS_SCOPE_PRIVATE;
  if (os_mutexInit (&rbp->lock, &mattr) != os_resultSuccess)
    goto fail_lock;

  rbp->rbuf_size = rbuf_size;
  rbp->max_rmsg_size = (os_uint32) max_rmsg_size;

#if USE_VALGRIND
  VALGRIND_CREATE_MEMPOOL (rbp, 0, 0);
#endif

  /* current must be valid or NULL on entry */
  rbp->current = NULL;
  rbp->current = nn_rbuf_new (rbp);
  return rbp;

 fail_lock:
  os_free (rbp);
 fail_rbp:
  return NULL;
}

void nn_rbufpool_free (struct nn_rbufpool *rbp)
{
#if 0
  /* Anyone may free it: I want to be able to stop the receive
     threads, then stop all other asynchronous processing, then clear
     out the buffers.  That's is the only way to verify that the
     reference counts are all 0, as they should be. */
  ASSERT_RBUFPOOL_OWNER (rbp);
#endif
  nn_rbuf_release (rbp->current);
#if USE_VALGRIND
  VALGRIND_DESTROY_MEMPOOL (rbp);
#endif
  os_mutexDestroy (&rbp->lock);
  os_free (rbp);
}

/* RBUF ---------------------------------------------------------------- */

struct nn_rbuf {
  os_uint32 n_live_rmsg_chunks;
  os_uint32 size;
  os_uint32 max_rmsg_size;
  struct nn_rbufpool *rbufpool;

  /* Allocating sequentially, releasing in random order, not bothering
     to reuse memory as soon as it becomes available again. I think
     this will have to change eventually, but this is the easiest
     approach.  Changes would be confined rmsg_new and rmsg_free. */
  char *freeptr;

  union {
    /* raw data array, nn_rbuf::size bytes long in reality */
    char raw[1];

    /* to ensure reasonable alignment of raw[] */
    os_int64 l;
    double d;
    void *p;
  } u;
};

static struct nn_rbuf *nn_rbuf_new (struct nn_rbufpool *rbufpool)
{
  /* note: releases rbufpool->current if not null */
  struct nn_rbuf *rb;
  ASSERT_RBUFPOOL_OWNER (rbufpool);

  if ((rb = os_malloc (offsetof (struct nn_rbuf, u.raw) + rbufpool->rbuf_size)) == NULL)
    return NULL;
#if USE_VALGRIND
  VALGRIND_MAKE_MEM_NOACCESS (rb->u.raw, rbufpool->rbuf_size);
#endif

  rb->rbufpool = rbufpool;
  rb->n_live_rmsg_chunks = 1;
  rb->size = rbufpool->rbuf_size;
  rb->max_rmsg_size = rbufpool->max_rmsg_size;
  rb->freeptr = rb->u.raw;

  os_mutexLock (&rbufpool->lock);
  if (rbufpool->current)
    nn_rbuf_release (rbufpool->current);
  rbufpool->current = rb;
  os_mutexUnlock (&rbufpool->lock);
  nn_log (LC_RADMIN, "rbuf_new(%p) = %p\n", rbufpool, rb);
  return rb;
}

static void nn_rbuf_release (struct nn_rbuf *rbuf)
{
  struct nn_rbufpool *rbp = rbuf->rbufpool;
  nn_log (LC_RADMIN, "rbuf_release(%p) pool %p current %p\n", rbuf, rbp, rbp->current);
  if (atomic_dec_u32_nv (&rbuf->n_live_rmsg_chunks) == 0)
  {
    nn_log (LC_RADMIN, "rbuf_release(%p) free\n", rbuf);
    os_free (rbuf);
  }
}

/* RMSG ---------------------------------------------------------------- */

/* There are at most 64kB / 32B = 2**11 rdatas in one rmsg, because an
   rmsg is limited to 64kB and a Data submessage is at least 32B bytes
   in size.  With 1 bit taken for committed/uncommitted (needed for
   debugging purposes only), there's room for up to 2**20 out-of-sync
   readers matched to one proxy writer.  I believe it sufficiently
   unlikely that anyone will ever attempt to have 1 million readers on
   one node to one topic/partition ... */
#define RMSG_REFCOUNT_UNCOMMITTED_BIAS (1u << 31)
#define RMSG_REFCOUNT_RDATA_BIAS (1u << 20)
#ifndef NDEBUG
#define ASSERT_RMSG_UNCOMMITTED(rmsg) (assert ((rmsg)->refcount >= RMSG_REFCOUNT_UNCOMMITTED_BIAS))
#else
#define ASSERT_RMSG_UNCOMMITTED(rmsg) ((void) 0)
#endif

static void *nn_rbuf_alloc (struct nn_rbufpool *rbufpool, os_uint32 asize)
{
  /* Note: only one thread calls nn_rmsg_new on a pool */
  struct nn_rbuf *rb;
  nn_log (LC_RADMIN, "rmsg_rbuf_alloc(%p, %u)\n", (void *) rbufpool, asize);
  ASSERT_RBUFPOOL_OWNER (rbufpool);
  rb = rbufpool->current;
  assert (rb != NULL);
  assert (rb->freeptr >= rb->u.raw);
  assert (rb->freeptr <= rb->u.raw + rb->size);

  if ((os_uint32) (rb->u.raw + rb->size - rb->freeptr) < asize)
  {
    /* not enough space left for new rmsg */
    if ((rb = nn_rbuf_new (rbufpool)) == NULL)
      return NULL;

    /* a new one should have plenty of space */
    assert ((os_uint32) (rb->u.raw + rb->size - rb->freeptr) >= asize);
  }

  nn_log (LC_RADMIN, "rmsg_rbuf_alloc(%p, %u) = %p\n", (void *) rbufpool, asize, (void *) rb->freeptr);
#if USE_VALGRIND
  VALGRIND_MEMPOOL_ALLOC (rbufpool, rb->freeptr, asize);
#endif
  return rb->freeptr;
}

static void init_rmsg_chunk (struct nn_rmsg_chunk *chunk, struct nn_rbuf *rbuf)
{
  chunk->rbuf = rbuf;
  chunk->next = NULL;
  chunk->size = 0;
  atomic_inc_u32_nv (&rbuf->n_live_rmsg_chunks);
}

struct nn_rmsg *nn_rmsg_new (struct nn_rbufpool *rbufpool)
{
  /* Note: only one thread calls nn_rmsg_new on a pool */
  struct nn_rmsg *rmsg;
  nn_log (LC_RADMIN, "rmsg_new(%p)\n", rbufpool);

  rmsg = nn_rbuf_alloc (rbufpool, offsetof (struct nn_rmsg, chunk.u.payload) + rbufpool->max_rmsg_size);
  if (rmsg == NULL)
    return NULL;

  /* Reference to this rmsg, undone by rmsg_commit(). */
  rmsg->refcount = RMSG_REFCOUNT_UNCOMMITTED_BIAS;
  /* Initial chunk */
  init_rmsg_chunk (&rmsg->chunk, rbufpool->current);
  rmsg->lastchunk = &rmsg->chunk;
  /* Incrementing freeptr happens in commit(), so that discarding the
     message is really simple. */
  nn_log (LC_RADMIN, "rmsg_new(%p) = %p\n", rbufpool, rmsg);
  return rmsg;
}

void nn_rmsg_setsize (struct nn_rmsg *rmsg, os_uint32 size)
{
  os_uint32 size8 = ALIGN8 (size);
  nn_log (LC_RADMIN, "rmsg_setsize(%p, %u => %u)\n", rmsg, size, size8);
  ASSERT_RBUFPOOL_OWNER (rmsg->chunk.rbuf->rbufpool);
  ASSERT_RMSG_UNCOMMITTED (rmsg);
  assert (rmsg->refcount == RMSG_REFCOUNT_UNCOMMITTED_BIAS);
  assert (rmsg->chunk.size == 0);
  assert (size8 <= rmsg->chunk.rbuf->max_rmsg_size);
  assert (rmsg->lastchunk == &rmsg->chunk);
  rmsg->chunk.size = size8;
#if USE_VALGRIND
  VALGRIND_MEMPOOL_CHANGE (rmsg->chunk.rbuf->rbufpool, rmsg, rmsg, offsetof (struct nn_rmsg, chunk.u.payload) + rmsg->chunk.size);
#endif
}

void nn_rmsg_free (struct nn_rmsg *rmsg)
{
  /* Note: any thread may call rmsg_free.

     FIXME: note that we could optimise by moving rbuf->freeptr back
     in (the likely to be fairly normal) case free space follows this
     rmsg.  Except that that would require synchronising new() and
     free() which we don't do currently.  And ideally, you'd use
     compare-and-swap for this. */
  struct nn_rmsg_chunk *c;
  nn_log (LC_RADMIN, "rmsg_free(%p)\n", rmsg);
  assert (rmsg->refcount == 0);
  c = &rmsg->chunk;
  while (c)
  {
    struct nn_rbuf *rbuf = c->rbuf;
    struct nn_rmsg_chunk *c1 = c->next;
#if USE_VALGRIND
    if (c == &rmsg->chunk) {
      VALGRIND_MEMPOOL_FREE (rbuf->rbufpool, rmsg);
    } else {
      VALGRIND_MEMPOOL_FREE (rbuf->rbufpool, c);
    }
#endif
    assert (rbuf->n_live_rmsg_chunks > 0);
    nn_rbuf_release (rbuf);
    c = c1;
  }
}

static void commit_rmsg_chunk (struct nn_rmsg_chunk *chunk)
{
  struct nn_rbuf *rbuf = chunk->rbuf;
  nn_log (LC_RADMIN, "commit_rmsg_chunk(%p)\n", chunk);
  rbuf->freeptr = chunk->u.payload + chunk->size;
}

void nn_rmsg_commit (struct nn_rmsg *rmsg)
{
  /* Note: only one thread calls rmsg_commit -- the one that created
     it in the first place.

     If there are no outstanding references, we can simply reuse the
     memory.  This happens, e.g., when the message is invalid, doesn't
     contain anything processed asynchronously, or the scheduling
     happens to be such that any asynchronous activities have
     completed before we got to commit. */
  struct nn_rmsg_chunk *chunk = rmsg->lastchunk;
  nn_log (LC_RADMIN, "rmsg_commit(%p) refcount 0x%x last-chunk-size %u\n",
          rmsg, rmsg->refcount, chunk->size);
  ASSERT_RBUFPOOL_OWNER (chunk->rbuf->rbufpool);
  ASSERT_RMSG_UNCOMMITTED (rmsg);
  assert (chunk->size <= chunk->rbuf->max_rmsg_size);
  assert ((chunk->size % 8) == 0);
  assert (rmsg->refcount >= RMSG_REFCOUNT_UNCOMMITTED_BIAS);
  assert (rmsg->chunk.rbuf->n_live_rmsg_chunks > 0);
  assert (chunk->rbuf->n_live_rmsg_chunks > 0);
  assert (chunk->rbuf->rbufpool->current == chunk->rbuf);
  if (atomic_sub_u32_nv (&rmsg->refcount, RMSG_REFCOUNT_UNCOMMITTED_BIAS) == 0)
    nn_rmsg_free (rmsg);
  else
  {
    /* Other references exist, so either stored in defrag, reorder
       and/or delivery queue */
    nn_log (LC_RADMIN, "rmsg_commit(%p) => keep\n", rmsg);
    commit_rmsg_chunk (chunk);
  }
}

static void nn_rmsg_addbias (struct nn_rmsg *rmsg)
{
  /* Note: only the receive thread that owns the receive pool may
     increase the reference count, and only while it is still
     uncommitted.

     However, other threads (e.g., delivery threads) may have been
     triggered already, so the increment must be done atomically. */
  nn_log (LC_RADMIN, "rmsg_addbias(%p)\n", rmsg);
  ASSERT_RBUFPOOL_OWNER (rmsg->chunk.rbuf->rbufpool);
  ASSERT_RMSG_UNCOMMITTED (rmsg);
  atomic_add_u32_noret (&rmsg->refcount, RMSG_REFCOUNT_RDATA_BIAS);
}

static void nn_rmsg_rmbias_and_adjust (struct nn_rmsg *rmsg, int adjust)
{
  /* This can happen to any rmsg referenced by an sample still
     progressing through the pipeline, but only by the receive
     thread.  Can't require it to be uncommitted. */
  os_uint32 sub;
  nn_log (LC_RADMIN, "rmsg_rmbias_and_adjust(%p, %d)\n", rmsg, adjust);
  ASSERT_RBUFPOOL_OWNER (rmsg->chunk.rbuf->rbufpool);
  assert (adjust >= 0);
  assert ((os_uint32) adjust < RMSG_REFCOUNT_RDATA_BIAS);
  sub = RMSG_REFCOUNT_RDATA_BIAS - (os_uint32) adjust;
  assert (rmsg->refcount >= sub);
  if (atomic_sub_u32_nv (&rmsg->refcount, sub) == 0)
    nn_rmsg_free (rmsg);
}

static void nn_rmsg_rmbias_anythread (struct nn_rmsg *rmsg)
{
  /* For removing garbage when freeing a nn_defrag. */
  os_uint32 sub = RMSG_REFCOUNT_RDATA_BIAS;
  nn_log (LC_RADMIN, "rmsg_rmbias_anythread(%p)\n", rmsg);
  assert (rmsg->refcount >= sub);
  if (atomic_sub_u32_nv (&rmsg->refcount, sub) == 0)
    nn_rmsg_free (rmsg);
}
static void nn_rmsg_unref (struct nn_rmsg *rmsg)
{
  nn_log (LC_RADMIN, "rmsg_unref(%p)\n", rmsg);
  assert (rmsg->refcount > 0);
  if (atomic_dec_u32_nv (&rmsg->refcount) == 0)
    nn_rmsg_free (rmsg);
}

void *nn_rmsg_alloc (struct nn_rmsg *rmsg, os_uint32 size)
{
  struct nn_rmsg_chunk *chunk = rmsg->lastchunk;
  struct nn_rbuf *rbuf = chunk->rbuf;
  os_uint32 size8 = ALIGN8 (size);
  void *ptr;
  nn_log (LC_RADMIN, "rmsg_alloc(%p, %u => %u)\n", rmsg, size, size8);
  ASSERT_RBUFPOOL_OWNER (rbuf->rbufpool);
  ASSERT_RMSG_UNCOMMITTED (rmsg);
  assert ((chunk->size % 8) == 0);
  assert (size8 <= rbuf->max_rmsg_size);

  if (chunk->size + size8 > rbuf->max_rmsg_size)
  {
    struct nn_rbufpool *rbufpool = rbuf->rbufpool;
    struct nn_rmsg_chunk *newchunk;
    nn_log (LC_RADMIN, "rmsg_alloc(%p, %u) limit hit - new chunk\n", rmsg, size);
    commit_rmsg_chunk (chunk);
    newchunk = nn_rbuf_alloc (rbufpool, offsetof (struct nn_rmsg_chunk, u.payload) + rbufpool->max_rmsg_size);
    if (newchunk == NULL)
    {
      nn_log (LC_RADMIN, "can't allocate more memory ... giving up\n");
      return NULL;
    }
    init_rmsg_chunk (newchunk, rbufpool->current);
    rmsg->lastchunk = chunk->next = newchunk;
    chunk = newchunk;
  }

  ptr = chunk->u.payload + chunk->size;
  chunk->size += size8;
  nn_log (LC_RADMIN, "rmsg_alloc(%p, %u) = %p\n", rmsg, size, ptr);
#if USE_VALGRIND
  if (chunk == &rmsg->chunk) {
    VALGRIND_MEMPOOL_CHANGE (rbuf->rbufpool, rmsg, rmsg, offsetof (struct nn_rmsg, chunk.u.payload) + chunk->size);
  } else {
    VALGRIND_MEMPOOL_CHANGE (rbuf->rbufpool, chunk, chunk, offsetof (struct nn_rmsg_chunk, u.payload) + chunk->size);
  }
#endif
  return ptr;
}

/* RDATA --------------------------------------- */

struct nn_rdata *nn_rdata_new (struct nn_rmsg *rmsg, os_uint32 start, os_uint32 endp1, os_uint32 submsg_offset, os_uint32 payload_offset)
{
  struct nn_rdata *d;
  if ((d = nn_rmsg_alloc (rmsg, sizeof (*d))) == NULL)
    return NULL;
  d->rmsg = rmsg;
  d->nextfrag = NULL;
  d->min = start;
  d->maxp1 = endp1;
  d->submsg_zoff = NN_OFF_TO_ZOFF (submsg_offset);
  d->payload_zoff = NN_OFF_TO_ZOFF (payload_offset);
#ifndef NDEBUG
  d->refcount_bias_added = 0;
#endif
  nn_log (LC_RADMIN, "rdata_new(%p, bytes [%u,%u), submsg @ %u, payload @ %u) = %p\n", rmsg, start, endp1, NN_RDATA_SUBMSG_OFF (d), NN_RDATA_PAYLOAD_OFF (d), d);
  return d;
}

static void nn_rdata_addbias (struct nn_rdata *rdata)
{
  nn_log (LC_RADMIN, "rdata_addbias(%p)\n", rdata);
#ifndef NDEBUG
  ASSERT_RBUFPOOL_OWNER (rdata->rmsg->chunk.rbuf->rbufpool);
  if (atomic_inc_u32_nv (&rdata->refcount_bias_added) != 1)
    abort ();
#endif
  nn_rmsg_addbias (rdata->rmsg);
}

static void nn_rdata_rmbias_and_adjust (struct nn_rdata *rdata, int adjust)
{
  nn_log (LC_RADMIN, "rdata_rmbias_and_adjust(%p, %d)\n", rdata, adjust);
#ifndef NDEBUG
  if (atomic_dec_u32_nv (&rdata->refcount_bias_added) != 0)
    abort ();
#endif
  nn_rmsg_rmbias_and_adjust (rdata->rmsg, adjust);
}

static void nn_rdata_rmbias_anythread (struct nn_rdata *rdata)
{
  nn_log (LC_RADMIN, "rdata_rmbias_anytrhead(%p, %d)\n", rdata);
#ifndef NDEBUG
  if (atomic_dec_u32_nv (&rdata->refcount_bias_added) != 0)
    abort ();
#endif
  nn_rmsg_rmbias_anythread (rdata->rmsg);
}

static void nn_rdata_unref (struct nn_rdata *rdata)
{
  nn_log (LC_RADMIN, "rdata_rdata_unref(%p)\n", rdata);
  nn_rmsg_unref (rdata->rmsg);
}

/* DEFRAG --------------------------------------------------------------

   Defragmentation happens separately from reordering, the reason
   being that defragmentation really is best done only once, and
   besides it simplifies reordering because it only ever has to deal
   with whole messages.

   The defragmeter accepts both rdatas that are fragments of samples
   and rdatas that are complete samples.  The unfragmented ones are
   returned immediately for further processing, in the format also
   used for fragmented samples.  Any rdata stored in the defrag index
   as well as unfragmented ones returned immediately are accounted for
   in rmsg::refcount.

   Defragmenting one sample is done using an interval tree where the
   minima and maxima are given by byte indexes of the received
   framgents.  Consecutive frags get chained in one interval, to keep
   the tree small even in the worst case.

   These intervals are represented using defrag_iv, and the fragment
   chain for an interval is built using the nextfrag links in the
   rdata.

   The defragmenter can defragment multiple samples in parallel (even
   though a writer normally produces a single fragment chain only,
   things may be different when packets get lost and/or
   (transient-local) data is resent).

   Each sample is represented using an rsample.  Each contains the
   root of an interval tree of fragments with a cached pointer to the
   last known interval (because we expect the data to arrive in-order
   and like to avoid searching).  The rsamples are stored in a tree
   indexed on sequence number, which itself caches the last sample it
   is currently defragmenting, again to avoid searching.

   The memory for an rsample is later re-used by the reordering
   mechanism.  Hence the union.  For that use, see REORDER.

   Partial and complete overlap of fragments is acceptable, but may
   result in a fragment chain containing fragments that do not add any
   bytes of information.  Those should be skipped by the deserializer.
   If the sender decides to suddenly change the fragmentation for a
   message, we happily keep processing them, even though there is no
   good reason for the sender to do so and the likelihood of such
   messy fragment chains increases significantly.

   Once done defragmenting, the tree consists of a root node only,
   which points to a list of fragments, in-order (but for the caveat
   above).

   Memory used for the storage of interval nodes while defragmenting
   is afterward re-used for chaining samples.  An unfragmented message
   will have a new sample chain allocated for this purpose, a
   fragmented message will have at least one interval allocated to it
   and thus have sufficient space for the chain node.

   FIXME: These AVL trees are overkill.  Either switch to parent-less
   red-black trees (they have better performance anyway and only need
   a single bit of state) or to splay trees (must have a parent
   because they can degenerate to linear structures, unless the number
   of intervals in the tree is limited, which probably is a good idea
   anyway). */

struct nn_defrag_iv {
  STRUCT_AVLNODE (nn_defrag_iv_avlnode, struct nn_defrag_iv *) avlnode; /* for nn_rsample.defrag::fragtree */
  os_uint32 min, maxp1;
  struct nn_rdata *first;
  struct nn_rdata *last;
};

struct nn_rsample {
  union {
    struct nn_rsample_defrag {
      STRUCT_AVLNODE (nn_rsample_defrag_avlnode, struct nn_rsample *) avlnode; /* for nn_defrag::sampletree */
      STRUCT_AVLTREE (nn_defrag_ivtree, struct nn_defrag_iv *) fragtree;
      struct nn_defrag_iv *lastfrag;
      struct nn_rsample_info *sampleinfo;
      os_int64 seq;
    } defrag;
    struct nn_rsample_reorder {
      STRUCT_AVLNODE (nn_rsample_reorder_avlnode, struct nn_rsample *) avlnode; /* for nn_reorder::sampleivtree, if head of a chain */
      struct nn_rsample_chain sc; /* this interval's */
      os_int64 min, maxp1;        /* interval: [min,maxp1) / */
    } reorder;
  } u;
};

struct nn_defrag {
  STRUCT_AVLTREE (nn_defrag_sampletree, struct nn_rsample *) sampletree;
  struct nn_rsample *max_sample; /* = max(sampletree) */
  os_uint32 n_samples;
  os_uint32 max_samples;
  enum nn_defrag_drop_mode drop_mode;
};

static int compare_uint32 (const void *va, const void *vb)
{
  os_uint32 a = *((const os_uint32 *) va);
  os_uint32 b = *((const os_uint32 *) vb);
  return (a == b) ? 0 : (a < b) ? -1 : 1;
}

static int compare_int64 (const void *va, const void *vb)
{
  os_int64 a = *((const os_int64 *) va);
  os_int64 b = *((const os_int64 *) vb);
  return (a == b) ? 0 : (a < b) ? -1 : 1;
}

struct nn_defrag *nn_defrag_new (enum nn_defrag_drop_mode drop_mode, os_uint32 max_samples)
{
  struct nn_defrag *d;
  assert (max_samples >= 1);
  if ((d = os_malloc (sizeof (*d))) == NULL)
    return NULL;
  avl_init (&d->sampletree, offsetof (struct nn_rsample, u.defrag.avlnode), offsetof (struct nn_rsample, u.defrag.seq), compare_int64, 0, 0);
  d->drop_mode = drop_mode;
  d->max_samples = max_samples;
  d->n_samples = 0;
  d->max_sample = NULL;
  return d;
}

void nn_fragchain_adjust_refcount (struct nn_rdata *frag, int adjust)
{
  struct nn_rdata *frag1;
  nn_log (LC_RADMIN, "fragchain_adjust_refcount(%p, %d)\n", frag, adjust);
  while (frag)
  {
    frag1 = frag->nextfrag;
    nn_rdata_rmbias_and_adjust (frag, adjust);
    frag = frag1;
  }
}

void nn_fragchain_rmbias_anythread (struct nn_rdata *frag, UNUSED_ARG (int adjust))
{
  struct nn_rdata *frag1;
  nn_log (LC_RADMIN, "fragchain_rmbias_anythread(%p)\n", frag);
  while (frag)
  {
    frag1 = frag->nextfrag;
    nn_rdata_rmbias_anythread (frag);
    frag = frag1;
  }
}

static void defrag_rsample_drop (struct nn_defrag *defrag, struct nn_rsample *rsample, void (*fragchain_free) (struct nn_rdata *frag, int adjust))
{
  /* Can't reference rsample after the first fragchain_free, because
     we don't know which rdata/rmsg provides the storage for the
     rsample and therefore can't increment the reference count.

     So we need to walk the fragments while guaranteeing strict
     "forward progress" in the memory accesses, which this particular
     inorder treewalk does provide. */
  struct nn_defrag_iv *todo[MAX_TREEHEIGHT], **todop = todo;
  nn_log (LC_RADMIN, "  defrag_rsample_drop (%p, %p)\n",
          (void *) defrag, (void *) rsample);
  avl_delete (&defrag->sampletree, rsample);
  assert (defrag->n_samples > 0);
  defrag->n_samples--;
  *todop = rsample->u.defrag.fragtree.root;
  while (*todop)
  {
    struct nn_defrag_iv *right, *n;
    /* First locate the minimum value in this subtree */
    n = (*todop)->avlnode.left;
    while (n)
    {
      *++todop = n;
      n = n->avlnode.left;
    }
    /* Then process it and its parents until a node N is hit that has
       a right subtree, with (by definition) key values between N and
       the parent of N */
    do {
      right = (*todop)->avlnode.right;
      fragchain_free ((*todop)->first, 0);
    } while (todop-- > todo && right == NULL);
    /* Continue with right subtree rooted at 'right' before processing
       the parent node of the last node processed in the loop above */
    *++todop = right;
  }
}

void nn_defrag_free (struct nn_defrag *defrag)
{
  struct nn_rsample *s;
  s = avl_findmin (&defrag->sampletree);
  while (s)
  {
    nn_log (LC_RADMIN, "defrag_free(%p, sample %p seq %lld)\n", defrag, s, s->u.defrag.seq);
    defrag_rsample_drop (defrag, s, nn_fragchain_rmbias_anythread);
    s = avl_findmin (&defrag->sampletree);
  }
  assert (defrag->n_samples == 0);
  os_free (defrag);
}

static int defrag_try_merge_with_succ (struct nn_rsample_defrag *sample, struct nn_defrag_iv *node)
{
  struct nn_defrag_iv *succ;

  nn_log (LC_RADMIN, "  defrag_try_merge_with_succ(%p [%u..%u)):\n",
          (void *) node, node->min, node->maxp1);
  if (node == sample->lastfrag)
  {
    /* there is no interval following node */
    nn_log (LC_RADMIN, "  node is lastfrag\n");
    return 0;
  }

  succ = avl_findsucc (&sample->fragtree, node);
  assert (succ != NULL);
  nn_log (LC_RADMIN, "  succ is %p [%u..%u)\n", (void *) succ, succ->min, succ->maxp1);
  if (succ->min > node->maxp1)
  {
    nn_log (LC_RADMIN, "  gap between node and succ\n");
    return 0;
  }
  else
  {
    os_uint32 succ_maxp1 = succ->maxp1;

    /* no longer a gap between node & succ => succ will be removed
       from the interval tree and therefore node will become the
       last interval if succ currently is */
    avl_delete (&sample->fragtree, succ);
    if (sample->lastfrag == succ)
    {
      nn_log (LC_RADMIN, "  succ is lastfrag\n");
      sample->lastfrag = node;
    }

    /* If succ's chain contains data beyond the frag we just
       received, append it to node (but do note that this doesn't
       guarantee that each fragment in the chain adds data!) and
       throw succ away.

       Do the same if succ's frag chain is completely contained in
       node, even though it wastes memory & cpu time (the latter,
       eventually): because the rsample we use may be dependent on the
       references to rmsgs of the rdata in succ, freeing it may cause
       the rsample to be freed as well. */
    if (node->maxp1 < succ_maxp1)
      nn_log (LC_RADMIN, "  succ adds data to node\n");
    else
      nn_log (LC_RADMIN, "  succ is contained in node\n");

    node->last->nextfrag = succ->first;
    node->last = succ->last;
    node->maxp1 = succ_maxp1;

    /* if the new fragment contains data beyond succ it may even
       allow merging with succ-succ */
    return node->maxp1 > succ_maxp1;
  }
}

static void defrag_rsample_addiv (struct nn_rsample_defrag *sample, struct nn_rdata *rdata, avlparent_t parent)
{
  struct nn_defrag_iv *newiv;
  if ((newiv = nn_rmsg_alloc (rdata->rmsg, sizeof (*newiv))) == NULL)
    return;
  avl_init_node (newiv, parent);
  rdata->nextfrag = NULL;
  newiv->first = newiv->last = rdata;
  newiv->min = rdata->min;
  newiv->maxp1 = rdata->maxp1;
  nn_rdata_addbias (rdata);
  avl_insert (&sample->fragtree, newiv);
  if (sample->lastfrag == NULL || rdata->min > sample->lastfrag->min)
    sample->lastfrag = newiv;
}

static void rsample_init_common (UNUSED_ARG (struct nn_rsample *rsample), UNUSED_ARG (struct nn_rdata *rdata), UNUSED_ARG (const struct nn_rsample_info *sampleinfo))
{
}

static struct nn_rsample *defrag_rsample_new (struct nn_rdata *rdata, const struct nn_rsample_info *sampleinfo, avlparent_t sampleparent)
{
  struct nn_rsample *rsample;
  struct nn_rsample_defrag *dfsample;
  avlparent_t ivparent;
  ivparent.addr = NULL;

  if ((rsample = nn_rmsg_alloc (rdata->rmsg, sizeof (*rsample))) == NULL)
    return NULL;
  rsample_init_common (rsample, rdata, sampleinfo);
  dfsample = &rsample->u.defrag;
  dfsample->lastfrag = NULL;
  dfsample->seq = sampleinfo->seq;
  if ((dfsample->sampleinfo = nn_rmsg_alloc (rdata->rmsg, sizeof (*dfsample->sampleinfo))) == NULL)
    return NULL;
  *dfsample->sampleinfo = *sampleinfo;

  avl_init_node (&dfsample->avlnode, sampleparent);
  avl_init (&dfsample->fragtree, offsetof (struct nn_defrag_iv, avlnode), offsetof (struct nn_defrag_iv, min), compare_uint32, 0, 0);

  /* add sentinel if rdata is not the first fragment of the message */
  if (rdata->min > 0)
  {
    struct nn_defrag_iv *sentinel;
    if ((sentinel = nn_rmsg_alloc (rdata->rmsg, sizeof (*sentinel))) == NULL)
      return NULL;
    avl_init_node (sentinel, ivparent);
    sentinel->first = sentinel->last = NULL;
    sentinel->min = sentinel->maxp1 = 0;
    avl_insert (&dfsample->fragtree, sentinel);
    /* sentinel necessarily is the parent of the interval representing rdata */
    avl_parent_from_node (&dfsample->fragtree, sentinel, &ivparent);
  }

  /* add an interval for the first received fragment */
  defrag_rsample_addiv (dfsample, rdata, ivparent);
  return rsample;
}

static struct nn_rsample *reorder_rsample_new (struct nn_rdata *rdata, const struct nn_rsample_info *sampleinfo)
{
  /* Implements:

       defrag_rsample_new ; rsample_convert_defrag_to_reorder

     It is simple enough to warrant having an extra function. Note the
     discrepancy between defrag_rsample_new which fully initializes
     the rsample, including the AVL node headers, and this function,
     which doesn't do so. */
  struct nn_rsample *rsample;
  struct nn_rsample_reorder *s;
  struct nn_rsample_chain_elem *sce;

  if ((rsample = nn_rmsg_alloc (rdata->rmsg, sizeof (*rsample))) == NULL)
    return NULL;
  rsample_init_common (rsample, rdata, sampleinfo);

  if ((sce = nn_rmsg_alloc (rdata->rmsg, sizeof (*sce))) == NULL)
    return NULL;
  sce->fragchain = rdata;
  sce->next = NULL;
  if ((sce->sampleinfo = nn_rmsg_alloc (rdata->rmsg, sizeof (*sce->sampleinfo))) == NULL)
    return NULL;
  *sce->sampleinfo = *sampleinfo;
  rdata->nextfrag = NULL;
  nn_rdata_addbias (rdata);

  s = &rsample->u.reorder;
  s->min = sampleinfo->seq;
  s->maxp1 = sampleinfo->seq + 1;
  s->sc.first = s->sc.last = sce;
  return rsample;
}

static int is_complete (const struct nn_rsample_defrag *sample)
{
  /* Returns: NULL if 'sample' is incomplete, else 'sample'. Complete:
     one interval covering all bytes. One interval because of the
     greedy coalescing in add_fragment(). There is at least one
     interval if we get here. */
  const struct nn_defrag_iv *iv = sample->fragtree.root;
  assert (iv != NULL);
  if (iv->min == 0 && iv->maxp1 >= sample->sampleinfo->size)
  {
    /* Accept fragments containing data beyond the end of the sample,
       only to filter them out (or not, as the case may be) at a later
       stage. Dropping them before the defragmeter leaves us with
       samples that will never be completed; dropping them in the
       defragmenter would be feasible by discarding all fragments of
       that sample collected so far. */
    assert (iv->avlnode.left == NULL && iv->avlnode.right == NULL);
    return 1;
  }
  else
  {
    return 0;
  }
}

static void rsample_convert_defrag_to_reorder (struct nn_rsample *sample)
{
  /* Converts an rsample as stored in defrag to one as stored in a
     reorder admin. Have to be careful with the ordering, or at least
     somewhat, and the easy way out uses a few local variables -- any
     self-respecting compiler will optimise them away, and any
     self-respecting CPU would need to copy them via registers anyway
     because it uses a load-store architecture. */
  struct nn_rdata *fragchain = sample->u.defrag.fragtree.root->first;
  struct nn_rsample_info *sampleinfo = sample->u.defrag.sampleinfo;
  struct nn_rsample_chain_elem *sce;
  os_int64 seq = sample->u.defrag.seq;

  /* re-use memory fragment interval node for sample chain */
  sce = (struct nn_rsample_chain_elem *) sample->u.defrag.fragtree.root;
  sce->fragchain = fragchain;
  sce->next = NULL;
  sce->sampleinfo = sampleinfo;

  sample->u.reorder.sc.first = sample->u.reorder.sc.last = sce;
  sample->u.reorder.min = seq;
  sample->u.reorder.maxp1 = seq + 1;
}

static struct nn_rsample *defrag_add_fragment (struct nn_rsample *sample, struct nn_rdata *rdata, const struct nn_rsample_info *sampleinfo)
{
  struct nn_rsample_defrag *dfsample = &sample->u.defrag;
  struct nn_defrag_iv *predeq, *succ;
  const os_uint32 min = rdata->min;
  const os_uint32 maxp1 = rdata->maxp1;

  /* min, max are byte offsets; contents has max-min+1 bytes; it all
     concerns the message pointer to by sample */
  assert (min < maxp1);
  /* and it must concern this message */
  assert (dfsample->seq == sampleinfo->seq);
  /* relatively expensive test: lastfrag, tree must be consistent */
  assert (dfsample->lastfrag == avl_findmax (&dfsample->fragtree));

  nn_log (LC_RADMIN, "  lastfrag %p [%u..%u)\n",
          (void *) dfsample->lastfrag,
          dfsample->lastfrag->min, dfsample->lastfrag->maxp1);

  /* Interval tree is sorted on min offset; each key is unique:
     otherwise one would be wholly contained in another. */
  if (min >= dfsample->lastfrag->min)
  {
    /* Assumed normal case: fragment appends data */
    predeq = dfsample->lastfrag;
    nn_log (LC_RADMIN, "  fast path: predeq = lastfrag\n");
  }
  else
  {
    /* Slow path: find preceding fragment by tree search */
    predeq = avl_lookup_predeq (&dfsample->fragtree, &min);
    nn_log (LC_RADMIN, "  slow path: predeq = lookup %u => %p [%u..%u)\n",
            min, (void *) predeq, predeq->min, predeq->maxp1);
  }

  /* we have a sentinel interval of [0,0) until we receive a packet
     that contains the first byte of the message, that is, there
     should always be predeq */
  assert (predeq != NULL);

  if (predeq->maxp1 >= maxp1)
  {
    /* new is contained in predeq, discard new; rdata did not cause
       completion of a sample */
    nn_log (LC_RADMIN, "  new contained in predeq\n");
    return NULL;
  }
  else if (min <= predeq->maxp1)
  {
    /* new extends predeq, add it to the chain (necessarily at the
       end); this may close the gap to the successor of predeq; predeq
       need not have a fragment chain yet (it may be the sentinel) */
    nn_log (LC_RADMIN, "  grow predeq with new\n");
    nn_rdata_addbias (rdata);
    rdata->nextfrag = NULL;
    if (predeq->first)
      predeq->last->nextfrag = rdata;
    else
    {
      /* 'Tis the sentinel => rewrite the sample info so we
         eventually always use the sample info contributed by the
         first fragment */
      predeq->first = rdata;
      *dfsample->sampleinfo = *sampleinfo;
    }
    predeq->last = rdata;
    predeq->maxp1 = maxp1;
    /* it may now be possible to merge with the successor */
    while (defrag_try_merge_with_succ (dfsample, predeq))
      ;
    return is_complete (dfsample) ? sample : NULL;
  }
  else if (predeq != dfsample->lastfrag && /* if predeq is last frag, there is no succ */
           (succ = avl_findsucc (&dfsample->fragtree, predeq)) != NULL &&
           succ->min <= maxp1)
  {
    /* extends succ (at the low end; no guarantee each individual
       fragment in the chain adds value); but doesn't overlap with
       predeq so the tree structure doesn't change even though the key
       does change */
    nn_log (LC_RADMIN, "  extending succ %p [%u..%u) at head\n",
            (void *) succ, succ->min, succ->maxp1);
    nn_rdata_addbias (rdata);
    rdata->nextfrag = succ->first;
    succ->first = rdata;
    succ->min = min;
    /* new one may cover all of succ & more, in which case we must
       update the max of succ & see if we can merge it with
       succ-succ */
    if (maxp1 > succ->maxp1)
    {
      nn_log (LC_RADMIN, "  extending succ at end as well\n");
      succ->maxp1 = maxp1;
      while (defrag_try_merge_with_succ (dfsample, succ))
        ;
    }
    assert (!is_complete (dfsample));
    return NULL;
  }
  else
  {
    /* doesn't extend either predeq at the end or succ at the head =>
       new interval; rdata did not cause completion of sample */
    avlparent_t parent;
    nn_log (LC_RADMIN, "  new interval\n");
    if (avl_lookup (&dfsample->fragtree, &min, &parent))
      assert (0);
    defrag_rsample_addiv (dfsample, rdata, parent);
    return NULL;
  }
}

static int nn_rdata_is_fragment (const struct nn_rdata *rdata, const struct nn_rsample_info *sampleinfo)
{
  /* sanity check: min, maxp1 must be within bounds */
  assert (rdata->min <= rdata->maxp1);
  assert (rdata->maxp1 <= sampleinfo->size);
  return !(rdata->min == 0 && rdata->maxp1 == sampleinfo->size);
}

static int defrag_limit_samples (struct nn_defrag *defrag, os_int64 seq, os_int64 *max_seq)
{
  struct nn_rsample *sample_to_drop = NULL;
  if (defrag->n_samples < defrag->max_samples)
    return 1;
  nn_log (LC_RADMIN, "  max samples reached\n");
  switch (defrag->drop_mode)
  {
    case NN_DEFRAG_DROP_LATEST:
      nn_log (LC_RADMIN, "  drop mode = DROP_LATEST\n");
      if (seq > defrag->max_sample->u.defrag.seq)
      {
        nn_log (LC_RADMIN, "  new sample is new latest => discarding it\n");
        return 0;
      }
      sample_to_drop = defrag->max_sample;
      break;
    case NN_DEFRAG_DROP_OLDEST:
      nn_log (LC_RADMIN, "  drop mode = DROP_OLDEST\n");
      sample_to_drop = avl_findmin (&defrag->sampletree);
      if (seq < sample_to_drop->u.defrag.seq)
      {
        nn_log (LC_RADMIN, "  new sample is new oldest => discarding it\n");
        return 0;
      }
      break;
  }
  assert (sample_to_drop != NULL);
  defrag_rsample_drop (defrag, sample_to_drop, nn_fragchain_adjust_refcount);
  if (sample_to_drop == defrag->max_sample)
  {
    defrag->max_sample = avl_findmax (&defrag->sampletree);
    *max_seq = defrag->max_sample ? defrag->max_sample->u.defrag.seq : 0;
    nn_log (LC_RADMIN, "  updating max_sample: now %p %lld\n",
            (void *) defrag->max_sample,
            defrag->max_sample ? defrag->max_sample->u.defrag.seq : 0);
  }
  return 1;
}

struct nn_rsample *nn_defrag_rsample (struct nn_defrag *defrag, struct nn_rdata *rdata, const struct nn_rsample_info *sampleinfo)
{
  /* Takes an rdata, records it in defrag if needed and returns an
     rdata chain representing a complete message ready for further
     processing if 'rdata' is complete or caused a message to become
     complete.

     On return 'rdata' is either: (a) stored in defrag and the rmsg
     refcount is biased; (b) refcount is biased and sample returned
     immediately because it wasn't actually a fragment; or (c) no
     effect on refcount & and not stored because it did not add any
     information.

     on entry:

     - rdata not refcounted, chaining fields need not be initialized.

     - sampleinfo fully initialised if first frag, else just seq,
       fragsize and size; will be copied onto memory allocated from
       the receive buffer

     return: all rdatas referenced in the chain returned by this
     function have been accounted for in the refcount of their rmsgs
     by adding BIAS to the refcount. */
  struct nn_rsample *sample, *result;
  os_int64 max_seq;
  avlparent_t parent;

  assert (defrag->n_samples <= defrag->max_samples);

  /* not a fragment => always complete, so refcount rdata, turn into a
     valid chain behind a valid msginfo and return it. */
  if (!nn_rdata_is_fragment (rdata, sampleinfo))
    return reorder_rsample_new (rdata, sampleinfo);

  /* max_seq is used for the fast path, and is 0 when there is no
     last message in 'defrag'. max_seq and max_sample must be
     consistent. Max_sample must be consistent with tree */
  assert (defrag->max_sample == avl_findmax (&defrag->sampletree));
  max_seq = defrag->max_sample ? defrag->max_sample->u.defrag.seq : 0;
  nn_log (LC_RADMIN, "defrag_rsample(%p, %p [%u..%u) msg %p, %p seq %lld size %u) max_seq %p %lld:\n",
          (void *) defrag, (void *) rdata, rdata->min, rdata->maxp1, rdata->rmsg,
          (void *) sampleinfo, sampleinfo->seq, sampleinfo->size,
          (void *) defrag->max_sample, max_seq);
  /* fast path: rdata is part of message with the highest sequence
     number we're currently defragmenting, or is beyond that */
  if (sampleinfo->seq == max_seq)
  {
    nn_log (LC_RADMIN, "  add fragment to max_sample\n");
    result = defrag_add_fragment (defrag->max_sample, rdata, sampleinfo);
  }
  else if (!defrag_limit_samples (defrag, sampleinfo->seq, &max_seq))
  {
    nn_log (LC_RADMIN, "  discarding sample\n");
    result = NULL;
  }
  else if (sampleinfo->seq > max_seq)
  {
    /* a node with a key greater than the maximum in a tree always has
       the old maximum node as its parent */
    nn_log (LC_RADMIN, "  new max sample\n");
    avl_parent_from_node (&defrag->sampletree, defrag->max_sample, &parent);
    if ((sample = defrag_rsample_new (rdata, sampleinfo, parent)) == NULL)
      return NULL;
    avl_insert (&defrag->sampletree, sample);
    defrag->max_sample = sample;
    defrag->n_samples++;
    result = NULL;
  }
  else if ((sample = avl_lookup (&defrag->sampletree, &sampleinfo->seq, &parent)) == NULL)
  {
    /* a new sequence number, but smaller than the maximum */
    nn_log (LC_RADMIN, "  new sample less than max\n");
    assert (sampleinfo->seq < max_seq);
    if ((sample = defrag_rsample_new (rdata, sampleinfo, parent)) == NULL)
      return NULL;
    avl_insert (&defrag->sampletree, sample);
    defrag->n_samples++;
    result = NULL;
  }
  else
  {
    /* adds (or, as the case may be, doesn't add) to a known message */
    nn_log (LC_RADMIN, "  add fragment to %p\n", (void *) sample);
    result = defrag_add_fragment (sample, rdata, sampleinfo);
  }

  if (result != NULL)
  {
    /* Once completed, remove from defrag sample tree and convert to
       reorder format. If it is the sample with the maximum sequence in
       the tree, an update of max_sample is required. */
    nn_log (LC_RADMIN, "  complete\n");
    avl_delete (&defrag->sampletree, result);
    assert (defrag->n_samples > 0);
    defrag->n_samples--;
    if (result == defrag->max_sample)
    {
      defrag->max_sample = avl_findmax (&defrag->sampletree);
      nn_log (LC_RADMIN, "  updating max_sample: now %p %lld\n",
              (void *) defrag->max_sample,
              defrag->max_sample ? defrag->max_sample->u.defrag.seq : 0);
    }
    rsample_convert_defrag_to_reorder (result);
  }

  assert (defrag->max_sample == avl_findmax (&defrag->sampletree));
  return result;
}

void nn_defrag_notegap (struct nn_defrag *defrag, os_int64 min, os_int64 maxp1)
{
  /* All sequence numbers in [min,maxp1) are unavailable so any
     fragments in that range must be discarded.  Used both for
     Hearbeats (by setting min=1) and for Gaps. */
  struct nn_rsample *s = avl_lookup_succeq (&defrag->sampletree, &min);
  while (s && s->u.defrag.seq < maxp1)
  {
    struct nn_rsample *s1 = avl_findsucc (&defrag->sampletree, s);
    defrag_rsample_drop (defrag, s, nn_fragchain_adjust_refcount);
    s = s1;
  }
  defrag->max_sample = avl_findmax (&defrag->sampletree);
}

int nn_defrag_nackmap (struct nn_defrag *defrag, os_int64 seq, os_uint32 maxfragnum, struct nn_fragment_number_set *map, os_uint32 maxsz)
{
  struct nn_rsample *s;
  struct nn_defrag_iv *iv;
  os_uint32 i, fragsz;
  assert (maxsz <= 256);
  s = avl_lookup (&defrag->sampletree, &seq, NULL);
  if (s == NULL)
  {
    /* If we know nothing about the sample, say so */
    return -1;
  }
  else
  {
    /* We always have an interval starting at 0, which is empty if we
       are missing the first fragment. */
    struct nn_defrag_iv *liv = s->u.defrag.lastfrag;
    nn_fragment_number_t map_end;
    iv = avl_findmin (&s->u.defrag.fragtree);
    fragsz = s->u.defrag.sampleinfo->fragsize;
    assert (iv != NULL);
    map->bitmap_base = iv->maxp1 / fragsz;
    if (liv->maxp1 < (maxfragnum + 1) * fragsz)
      map_end = maxfragnum;
    else if (liv->min > 0)
      map_end = (liv->min - 1) / fragsz;
    else
      map_end = 0;
    map->numbits = (map_end < map->bitmap_base) ? 0 : map_end - map->bitmap_base + 1;
    iv = avl_findsucc (&s->u.defrag.fragtree, iv);
  }
  if (map->numbits > maxsz)
    map->numbits = maxsz;
  nn_bitset_zero (map->numbits, map->bits);
  /* Set bits for gaps, if sample is unknown, iv = NULL and loop won't
     run. */
  i = map->bitmap_base;
  while (iv && i < map->bitmap_base + map->numbits)
  {
    os_uint32 bound = iv->min / fragsz;
    if ((iv->min % fragsz) != 0)
      ++bound;
    for (; i < map->bitmap_base + map->numbits && i < bound; i++)
    {
      int x = (int) (i - map->bitmap_base);
      nn_bitset_set (map->numbits, map->bits, x);
    }
    i = iv->maxp1 / fragsz;
    iv = avl_findsucc (&s->u.defrag.fragtree, iv);
  }
  /* Fragments missing beyond the last interval - the only loop for an
     unknown message */
  for (; i < map->bitmap_base + map->numbits; i++)
  {
    int x = (int) (i - map->bitmap_base);
    nn_bitset_set (map->numbits, map->bits, x);
  }
  return map->numbits;
}

/* REORDER -------------------------------------------------------------

   The reorder index tracks out-of-order messages as non-overlapping,
   non-consecutive intervals of sequence numbers, with each interval
   pointing to a chain of rsamples (rsample_chain{,_elem}).  The
   maximum number of samples stored by the radmin is max_samples
   (setting it to 2**32-1 effectively makes it unlimited, by you're
   then you're probably into TB territority as you need at least an
   rmsg, rdata, sampleinfo, rsample, and a rsample_chain_elem, which
   adds up to quite a few bytes).

   The policy is to prefer the lowest sequence numbers, as those need
   to be delivered before the higher ones can be, and also because one
   radmin tracks only a single sequence.  Historical data uses a
   per-reader radmin.

   Each reliable proxy writer has a reorder admin for reordering
   messages, the "primary" reorder admin.  For the primary one, it is
   possible to store indexing data in memory originally allocated
   memory for defragmenting, as the defragmenter is done with it and
   this admin is the only one indexing the sample.

   Each out-of-sync proxy-writer--reader match also has an reorder
   instance, a "secondary" reorder admin, but those can't re-use
   memory like the proxy-writer's can, because there can be any number
   of them.  Before inserting in one of these, the sample must first
   be replicated using reorder_rsample_dup(), which fortunately is an
   extremely cheap operation.

   A sample either goes to the primary one (which may store it, reject
   it, or return it and subsequent samples immediately) [CASE I], or
   it goes to any number of secondary ones [CASE II].

   The reorder_rsample function may require updates to the reference
   counts of the rmsgs referenced by the rdatas in the sample it was
   called with (and _only_ to those of that particular sample, as
   others underwent all this processing before).  The
   "refcount_adjust" in/out parameter is updated to reflect the
   required change.

   A complicating factor is that after storing a sample in a reorder
   admin it potentially becomes part of a chain of samples, and may be
   located anywhere within that chain.  When that happens, the rsample
   parameter provided to reorder_rsample becomes useless for adjusting
   the reference counts as required.

   The initial reference count as it comes out of defragmentation is
   always BIAS-per-rdata, which means all rmgs referenced by the
   sample have refcount = BIAS if there is only ever a single sample
   in each rmsg.  (If multiple data submessages have been packed into
   a single message, they'll all contribute to the refcount.)

   The reference count adjustment is incremented by reorder_rsample
   whenever it stores or forwards the sample, and left unchanged when
   it rejects it (old samples & duplicates).  The initial reference
   needs to be accounted for as well, and so:

   - In [CASE I]: accept (or forward): +1 for accepting it, -BIAS for
     the initial reference, for a net change of 1-BIAS.  Reject: 0 for
     rejecting it, still -BIAS for the initial reference, for a net
     change of -BIAS.

   - In [CASE 2], each reorder admin gets its own copy of the sample,
     and therefore the sample that came out of defragmentation is
     unchanged, and may thus be used, regardless of the adjustment
     required.

     Accept by M out N: +M for accepting, 0 for the N-M rejects, -BIAS
     for the initial reference.  For a net change of M-BIAS.

   So in both cases, the adjustment needed is the number of reorder
   admins that accepted it, less BIAS for the initial reference.  We
   can't use the original sample because of [CASE I], so we adjust
   based on the fragment chain instead of the sample.  Example code is
   in the overview comment at the top of this file. */

struct nn_reorder {
  STRUCT_AVLTREE (nn_reorder_ivtree, struct nn_rsample *) sampleivtree;
  struct nn_rsample *max_sampleiv; /* = max(sampleivtree) */
  os_int64 next_seq;
  enum nn_reorder_mode mode;
  os_uint32 max_samples;
  os_uint32 n_samples;
};

struct nn_reorder *nn_reorder_new (enum nn_reorder_mode mode, os_uint32 max_samples)
{
  struct nn_reorder *r;
  if ((r = os_malloc (sizeof (*r))) == NULL)
    return NULL;
  avl_init (&r->sampleivtree, offsetof (struct nn_rsample, u.reorder.avlnode), offsetof (struct nn_rsample, u.reorder.min), compare_int64, 0, 0);
  r->max_sampleiv = NULL;
  r->next_seq = 1;
  r->mode = mode;
  r->max_samples = max_samples;
  r->n_samples = 0;
  return r;
}

void nn_fragchain_unref (struct nn_rdata *frag)
{
  struct nn_rdata *frag1;
  while (frag)
  {
    frag1 = frag->nextfrag;
    nn_rdata_unref (frag);
    frag = frag1;
  }
}

void nn_reorder_free (struct nn_reorder *r)
{
  struct nn_rsample *iv;
  struct nn_rsample_chain_elem *sce;
  /* FXIME: instead of findmin/delete, a treewalk can be used. */
  iv = avl_findmin (&r->sampleivtree);
  while (iv)
  {
    avl_delete (&r->sampleivtree, iv);
    sce = iv->u.reorder.sc.first;
    while (sce)
    {
      struct nn_rsample_chain_elem *sce1 = sce->next;
      nn_fragchain_unref (sce->fragchain);
      sce = sce1;
    }
    iv = avl_findmin (&r->sampleivtree);
  }
  os_free (r);
}

static void reorder_add_rsampleiv (struct nn_reorder *reorder, struct nn_rsample *rsample)
{
  avlparent_t parent;
  struct nn_rsample *x;
  x = avl_lookup (&reorder->sampleivtree, &rsample->u.reorder.min, &parent);
  assert (x == NULL);
  avl_init_node (&rsample->u.reorder.avlnode, parent);
  avl_insert (&reorder->sampleivtree, rsample);
}

static int rsample_is_singleton (const struct nn_rsample_reorder *s)
{
  assert (s->min < s->maxp1);
  if (s->min + 1 != s->maxp1)
    return 0;
  assert (s->min + 1 == s->maxp1);
  assert (s->sc.first != NULL);
  assert (s->sc.first == s->sc.last);
  assert (s->sc.first->next == NULL);
  return 1;
}

static void append_rsample_interval (struct nn_rsample *a, struct nn_rsample *b)
{
  a->u.reorder.sc.last->next = b->u.reorder.sc.first;
  a->u.reorder.sc.last = b->u.reorder.sc.last;
  a->u.reorder.maxp1 = b->u.reorder.maxp1;
}

static int reorder_try_append_and_discard (struct nn_reorder *reorder, struct nn_rsample *appendto, struct nn_rsample *todiscard)
{
  if (todiscard == NULL)
  {
    nn_log (LC_RADMIN, "  try_append_and_discard: fail: todiscard = NULL\n");
    return 0;
  }
  else if (appendto->u.reorder.maxp1 < todiscard->u.reorder.min)
  {
    nn_log (LC_RADMIN, "  try_append_and_discard: fail: appendto = [%lld,%lld) @ %p, "
            "todiscard = [%lld,%lld) @ %p - gap\n",
            appendto->u.reorder.min, appendto->u.reorder.maxp1, (void *) appendto,
            todiscard->u.reorder.min, todiscard->u.reorder.maxp1, (void *) todiscard);
    return 0;
  }
  else
  {
    nn_log (LC_RADMIN, "  try_append_and_discard: success: appendto = [%lld,%lld) @ %p, "
            "todiscard = [%lld,%lld) @ %p\n",
            appendto->u.reorder.min, appendto->u.reorder.maxp1, (void *) appendto,
            todiscard->u.reorder.min, todiscard->u.reorder.maxp1, (void *) todiscard);
    assert (todiscard->u.reorder.min == appendto->u.reorder.maxp1);
    avl_delete (&reorder->sampleivtree, todiscard);
    append_rsample_interval (appendto, todiscard);
    nn_log (LC_RADMIN, "  try_append_and_discard: max_sampleiv needs update? %s\n",
            (todiscard == reorder->max_sampleiv) ? "yes" : "no");
    /* Inform caller whether reorder->max must be updated -- the
       expected thing to do is to update it to appendto here, but that
       fails if appendto isn't actually in the tree.  And that happens
       to be the fast path where the sample that comes in has the
       sequence number we expected. */
    return todiscard == reorder->max_sampleiv;
  }
}

struct nn_rsample *nn_reorder_rsample_dup (struct nn_rmsg *rmsg, struct nn_rsample *rsampleiv)
{
  /* Duplicates the rsampleiv without updating any reference counts:
     that is left to the caller, as they do not need to be updated if
     the duplicate ultimately doesn't get used.

     The rmsg is the one to allocate from, and must be the one
     currently being processed (one can only allocate memory from an
     uncommitted rmsg) and must be referenced by an rdata in
     rsampleiv. */
  struct nn_rsample *rsampleiv_new;
  struct nn_rsample_chain_elem *sce;
  assert (rsample_is_singleton (&rsampleiv->u.reorder));
#ifndef NDEBUG
  {
    struct nn_rdata *d = rsampleiv->u.reorder.sc.first->fragchain;
    while (d && d->rmsg != rmsg)
      d = d->nextfrag;
    assert (d != NULL);
  }
#endif
  if ((rsampleiv_new = nn_rmsg_alloc (rmsg, sizeof (*rsampleiv_new))) == NULL)
    return NULL;
  if ((sce = nn_rmsg_alloc (rmsg, sizeof (*sce))) == NULL)
    return NULL;
  sce->fragchain = rsampleiv->u.reorder.sc.first->fragchain;
  sce->next = NULL;
  *rsampleiv_new = *rsampleiv;
  rsampleiv_new->u.reorder.sc.first = rsampleiv_new->u.reorder.sc.last = sce;
  return rsampleiv_new;
}

struct nn_rdata *nn_rsample_fragchain (struct nn_rsample *rsample)
{
  assert (rsample_is_singleton (&rsample->u.reorder));
  return rsample->u.reorder.sc.first->fragchain;
}

static char reorder_mode_as_char (const struct nn_reorder *reorder)
{
  switch (reorder->mode)
  {
    case NN_REORDER_MODE_NORMAL: return 'R';
    case NN_REORDER_MODE_MONOTONICALLY_INCREASING: return 'U';
    case NN_REORDER_MODE_ALWAYS_DELIVER: return 'A';
  }
  assert (0);
  return '?';
}

static void delete_last_sample (struct nn_reorder *reorder)
{
  struct nn_rsample_reorder *last = &reorder->max_sampleiv->u.reorder;
  struct nn_rdata *fragchain;

  /* This just removes it, it doesn't adjust the count. It is not
     supposed to be called on an radmin with only one sample. */
  assert (reorder->n_samples > 0);
  assert (reorder->max_sampleiv != NULL);

  if (rsample_is_singleton (last))
  {
    /* Last sample is in an interval of its own - delete it, and
       recalc max_sampleiv. */
    nn_log (LC_RADMIN, "  delete_last_sample: in singleton interval\n");
    fragchain = last->sc.first->fragchain;
    avl_delete (&reorder->sampleivtree, reorder->max_sampleiv);
    reorder->max_sampleiv = avl_findmax (&reorder->sampleivtree);
    /* No harm done if it the sampleivtree is empty, except that we
       chose not to allow it */
    assert (reorder->max_sampleiv != NULL);
  }
  else
  {
    /* Last sample is to be removed from the final interval.  Which
       requires scanning the sample chain because it is a
       singly-linked list (so you might not want max_samples set very
       large!).  Can't be a singleton list, so might as well chop off
       one evaluation of the loop condition. */
    struct nn_rsample_chain_elem *e, *pe;
    nn_log (LC_RADMIN, "  delete_last_sample: scanning last interval [%llu..%llu)\n",
            last->min, last->maxp1);
    assert (last->sc.first != last->sc.last);
    e = last->sc.first;
    do {
      pe = e;
      e = e->next;
    } while (e != last->sc.last);
    fragchain = e->fragchain;
    pe->next = NULL;
    assert (pe->sampleinfo->seq + 2 == last->maxp1);
    last->sc.last = pe;
    last->maxp1--;
  }

  nn_fragchain_unref (fragchain);
}

enum nn_reorder_result nn_reorder_rsample (struct nn_rsample_chain *sc, struct nn_reorder *reorder, struct nn_rsample *rsampleiv, int *refcount_adjust)
{
  /* Adds an rsample (represented as an interval) to the reorder admin
     and returns the chain of consecutive samples ready for delivery
     because of the insertion.  Consequently, if it returns a sample
     chain, the sample referenced by rsampleiv is the first in the
     chain.

     refcount_adjust is incremented if the sample is not discarded. */
  struct nn_rsample_reorder *s = &rsampleiv->u.reorder;

  nn_log (LC_RADMIN, "reorder_sample(%p %c, %lld @ %p) expecting %lld:\n", (void *) reorder, reorder_mode_as_char (reorder), rsampleiv->u.reorder.min, (void *) rsampleiv, reorder->next_seq);

  /* Incoming rsample must be a singleton */
  assert (rsample_is_singleton (s));

  /* Reorder must not contain samples with sequence numbers <= next
     seq; max must be set iff the reorder is non-empty. */
#ifndef NDEBUG
  {
    struct nn_rsample *min = avl_findmin (&reorder->sampleivtree);
    if (min)
      nn_log (LC_RADMIN, "  min = %lld @ %p\n", min->u.reorder.min, (void *) min);
    assert (min == NULL || reorder->next_seq < min->u.reorder.min);
    assert ((reorder->max_sampleiv == NULL && min == NULL) ||
            (reorder->max_sampleiv != NULL && min != NULL));
  }
#endif
  assert ((!!avl_empty (&reorder->sampleivtree)) == (reorder->max_sampleiv == NULL));
  assert (reorder->max_sampleiv == NULL || reorder->max_sampleiv == avl_findmax (&reorder->sampleivtree));
  assert (reorder->n_samples <= reorder->max_samples);
  if (reorder->max_sampleiv)
    nn_log (LC_RADMIN, "  max = [%lld,%lld) @ %p\n", reorder->max_sampleiv->u.reorder.min, reorder->max_sampleiv->u.reorder.maxp1, (void *) reorder->max_sampleiv);

  if (s->min == reorder->next_seq ||
      (s->min > reorder->next_seq && reorder->mode == NN_REORDER_MODE_MONOTONICALLY_INCREASING) ||
      reorder->mode == NN_REORDER_MODE_ALWAYS_DELIVER)
  {
    /* 's' is next sample to be delivered; maybe we can append the
       first interval in the tree to it.  We can avoid all processing
       if the index is empty, which is the normal case.  Unreliable
       out-of-order either ends up here or in discard.)  */
    if (reorder->max_sampleiv != NULL)
    {
      struct nn_rsample *min = avl_findmin (&reorder->sampleivtree);
      nn_log (LC_RADMIN, "  try append_and_discard\n");
      if (reorder_try_append_and_discard (reorder, rsampleiv, min))
        reorder->max_sampleiv = NULL;
    }
    reorder->next_seq = s->maxp1;
    *sc = rsampleiv->u.reorder.sc;
    (*refcount_adjust)++;
    nn_log (LC_RADMIN, "  return [%lld,%lld)\n", s->min, s->maxp1);

    /* Adjust n_samples, new sample is not counted yet */
    assert (s->maxp1 - s->min >= 1);
    assert (reorder->n_samples >= s->maxp1 - s->min - 1);
    reorder->n_samples -= s->maxp1 - s->min - 1;
    return NN_REORDER_DELIVER;
  }
  else if (s->min < reorder->next_seq)
  {
    /* we've moved beyond this one: discard it; no need to adjust
       n_samples */
    nn_log (LC_RADMIN, "  discard: too old\n");
    return NN_REORDER_TOO_OLD; /* don't want refcount increment */
  }
  else if (avl_empty (&reorder->sampleivtree))
  {
    /* else, if nothing's stored simply add this one, max_samples = 0
       is technically allowed, and potentially useful, so check for
       it */
    assert (reorder->n_samples == 0);
    nn_log (LC_RADMIN, "  adding to empty store\n");
    if (reorder->max_samples == 0)
    {
      nn_log (LC_RADMIN, "  NOT - max_samples hit\n");
      return NN_REORDER_REJECT;
    }
    else
    {
      reorder_add_rsampleiv (reorder, rsampleiv);
      reorder->max_sampleiv = rsampleiv;
      reorder->n_samples++;
    }
  }
  else if (s->min == reorder->max_sampleiv->u.reorder.maxp1)
  {
    /* grow the last interval, if we're still accepting samples */
    nn_log (LC_RADMIN, "  growing last interval\n");
    if (reorder->n_samples < reorder->max_samples)
    {
      append_rsample_interval (reorder->max_sampleiv, rsampleiv);
      reorder->n_samples++;
    }
    else
    {
      nn_log (LC_RADMIN, "  NOT - max_samples hit\n");
      return NN_REORDER_REJECT;
    }
  }
  else if (s->min > reorder->max_sampleiv->u.reorder.maxp1)
  {
    if (reorder->n_samples < reorder->max_samples)
    {
      nn_log (LC_RADMIN, "  new interval at end\n");
      reorder_add_rsampleiv (reorder, rsampleiv);
      reorder->max_sampleiv = rsampleiv;
      reorder->n_samples++;
    }
    else
    {
      nn_log (LC_RADMIN, "  discarding sample: max_samples reached and sample at end\n");
      return NN_REORDER_REJECT;
    }
  }
  else
  {
    /* lookup interval predeq=[m,n) s.t. m <= s->min and
       immsucc=[m',n') s.t. m' = s->maxp1:

       - if m <= s->min < n we discard it (duplicate)
       - if n=s->min we can append s to predeq
       - if immsucc exists we can prepend s to immsucc
       - and possibly join predeq, s, and immsucc */
    struct nn_rsample *predeq, *immsucc;
    nn_log (LC_RADMIN, "  hard case ...\n");
    predeq = avl_lookup_predeq (&reorder->sampleivtree, &s->min);
    if (predeq)
      nn_log (LC_RADMIN, "  predeq = [%lld,%lld) @ %p\n",
              predeq->u.reorder.min, predeq->u.reorder.maxp1, (void *) predeq);
    else
      nn_log (LC_RADMIN, "  predeq = null\n");
    if (predeq && s->min >= predeq->u.reorder.min && s->min < predeq->u.reorder.maxp1)
    {
      /* contained in predeq */
      nn_log (LC_RADMIN, "  discard: contained in predeq\n");
      return NN_REORDER_REJECT;
    }

    immsucc = avl_lookup (&reorder->sampleivtree, &s->maxp1, NULL);
    if (immsucc)
      nn_log (LC_RADMIN, "  immsucc = [%lld,%lld) @ %p\n",
              immsucc->u.reorder.min, immsucc->u.reorder.maxp1, (void *) immsucc);
    else
      nn_log (LC_RADMIN, "  immsucc = null\n");
    if (predeq && s->min == predeq->u.reorder.maxp1)
    {
      /* grow predeq at end, and maybe append immsucc as well */
      nn_log (LC_RADMIN, "  growing predeq at end ...\n");
      append_rsample_interval (predeq, rsampleiv);
      if (reorder_try_append_and_discard (reorder, predeq, immsucc))
        reorder->max_sampleiv = predeq;
    }
    else if (immsucc)
    {
      /* no predecessor, grow immsucc at head, which _does_ alter the
         key of the node in the tree, but _doesn't_ change the tree's
         structure. */
      nn_log (LC_RADMIN, "  growing immsucc at head\n");
      s->sc.last->next = immsucc->u.reorder.sc.first;
      immsucc->u.reorder.sc.first = s->sc.first;
      immsucc->u.reorder.min = s->min;
    }
    else
    {
      /* neither extends predeq nor immsucc */
      nn_log (LC_RADMIN, "  new interval\n");
      reorder_add_rsampleiv (reorder, rsampleiv);
      if (rsampleiv->u.reorder.min > reorder->max_sampleiv->u.reorder.min)
        reorder->max_sampleiv = rsampleiv;
    }

    /* do not let radmin grow beyond max_samples; now that we've
       inserted it (and possibly have grown the radmin beyond its max
       size), we no longer risk deleting the interval that the new
       sample belongs to when deleting the last sample. */
    if (reorder->n_samples < reorder->max_samples)
      reorder->n_samples++;
    else
      delete_last_sample (reorder);
  }

  (*refcount_adjust)++;
  return NN_REORDER_ACCEPT;
}

static struct nn_rsample *coalesce_intervals_touching_range (os_uint32 *nsamples, struct nn_reorder *reorder, os_int64 min, os_int64 maxp1)
{
  struct nn_rsample *s, *t;
  os_uint32 n = 0;
  /* Find first (lowest m) interval [m,n) s.t. n >= min && m <= maxp1 */
  s = avl_lookup_predeq (&reorder->sampleivtree, &min);
  if (s && s->u.reorder.maxp1 >= min)
  {
    /* m <= min && n >= min (note: pred of s [m',n') necessarily has n' < m) */
#ifndef NDEBUG
    struct nn_rsample *q = avl_findpred (&reorder->sampleivtree, s);
    assert (q == NULL || q->u.reorder.maxp1 < min);
#endif
  }
  else
  {
    /* No good, but the first (if s = NULL) or the next one (if s !=
       NULL) may still have m <= maxp1 (m > min is implied now).  If
       not, no such interval.  */
    s = avl_findsucc (&reorder->sampleivtree, s);
    if (!(s && s->u.reorder.min <= maxp1))
      return NULL;
  }
  /* Append successors [m',n') s.t. m' <= maxp1 to s */
  n = s->u.reorder.maxp1 - s->u.reorder.min;
  while ((t = avl_findsucc (&reorder->sampleivtree, s)) != NULL && t->u.reorder.min <= maxp1)
  {
    avl_delete (&reorder->sampleivtree, t);
    append_rsample_interval (s, t);
    n += t->u.reorder.maxp1 - t->u.reorder.min;
  }
  /* If needed, grow range to [min,maxp1) */
  if (min < s->u.reorder.min)
    s->u.reorder.min = min;
  if (maxp1 > s->u.reorder.maxp1)
    s->u.reorder.maxp1 = maxp1;
  *nsamples = n;
  return s;
}

struct nn_rdata *nn_rdata_newgap (struct nn_rmsg *rmsg)
{
  struct nn_rdata *d;
  if ((d = nn_rdata_new (rmsg, 0, 0, 0, 0)) == NULL)
    return NULL;
  nn_rdata_addbias (d);
  return d;
}

static int reorder_insert_gap (struct nn_reorder *reorder, struct nn_rdata *rdata, os_int64 min, os_int64 maxp1)
{
  struct nn_rsample_chain_elem *sce;
  struct nn_rsample *s, *tmp;
  avlparent_t parent;
  if ((tmp = avl_lookup (&reorder->sampleivtree, &min, &parent)) != NULL)
    assert (0);
  if ((sce = nn_rmsg_alloc (rdata->rmsg, sizeof (*sce))) == NULL)
    return 0;
  sce->fragchain = rdata;
  sce->next = NULL;
  sce->sampleinfo = NULL;
  if ((s = nn_rmsg_alloc (rdata->rmsg, sizeof (*s))) == NULL)
    return 0;
  avl_init_node (&s->u.reorder.avlnode, parent);
  s->u.reorder.sc.first = s->u.reorder.sc.last = sce;
  s->u.reorder.min = min;
  s->u.reorder.maxp1 = maxp1;
  avl_insert (&reorder->sampleivtree, s);
  return 0;
}

enum nn_reorder_result nn_reorder_gap (struct nn_rsample_chain *sc, struct nn_reorder *reorder, struct nn_rdata *rdata, os_int64 min, os_int64 maxp1, int *refcount_adjust)
{
  /* All sequence numbers in [min,maxp1) are unavailable so any
     fragments in that range must be discarded.  Used both for
     Hearbeats (by setting min=1) and for Gaps.

       Case I: maxp1 <= next_seq.  No effect whatsoever.

     Otherwise:

       Case II: min <= next_seq.  All samples we have with sequence
         numbers less than maxp1 plus those following it consecutively
         are returned, and next_seq is updated to max(maxp1, highest
         returned sequence number+1)

     Else:

       Case IIII: Causes coalescing of intervals overlapping with
         [min,maxp1) or consecutive to it, possibly extending
         intervals to min on the lower bound or maxp1 on the upper
         one, or if there are no such intervals, the creation of a
         [min,maxp1) interval without any samples.

     NOTE: must not store anything (i.e. modify rdata,
     refcount_adjust) if gap causes data to be delivered: altnerative
     path for out-of-order delivery if all readers of a reliable
     proxy-writer are unrelibale depends on it. */
  struct nn_rsample *coalesced;
  os_uint32 nsamples;

  nn_log (LC_RADMIN, "reorder_gap(%p %c, [%lld,%lld) data %p) expecting %lld:\n",
          (void *) reorder, reorder_mode_as_char (reorder),
          min, maxp1, (void *) rdata, reorder->next_seq);

  if (maxp1 <= reorder->next_seq)
  {
    nn_log (LC_RADMIN, "  too old\n");
    return NN_REORDER_TOO_OLD;
  }
  if (reorder->mode != NN_REORDER_MODE_NORMAL)
  {
    nn_log (LC_RADMIN, "  special mode => don't care\n");
    return NN_REORDER_REJECT;
  }

  /* Coalesce all intervals [m,n) with n >= min or m <= maxp1 */
  if ((coalesced = coalesce_intervals_touching_range (&nsamples, reorder, min, maxp1)) == NULL)
  {
    enum nn_reorder_result res;
    nn_log (LC_RADMIN, "  coalesced = null\n");
    if (min <= reorder->next_seq)
    {
      nn_log (LC_RADMIN, "  next expected: %lld\n", maxp1);
      reorder->next_seq = maxp1;
      res = NN_REORDER_REJECT;
    }
    else if (reorder->max_sampleiv && min > reorder->max_sampleiv->u.reorder.maxp1 && reorder->n_samples == reorder->max_samples)
    {
      nn_log (LC_RADMIN, "  discarding gap: max_samples reached and gap at end\n");
      res = NN_REORDER_REJECT;
    }
    else
    {
      nn_log (LC_RADMIN, "  storing gap\n");
      reorder_insert_gap (reorder, rdata, min, maxp1);
      res = NN_REORDER_ACCEPT;
      /* do not let radmin grow beyond max_samples; there is a small
         possibility that we insert it & delete it immediately
         afterward. */
      if (reorder->n_samples < reorder->max_samples)
        reorder->n_samples++;
      else
        delete_last_sample (reorder);
      (*refcount_adjust)++;
    }
    reorder->max_sampleiv = avl_findmax (&reorder->sampleivtree);
    return res;
  }
  else if (coalesced->u.reorder.min <= reorder->next_seq)
  {
    nn_log (LC_RADMIN, "  coalesced = [%lld,%lld) @ %p containing %u samples\n",
            coalesced->u.reorder.min, coalesced->u.reorder.maxp1,
            (void *) coalesced, nsamples);
    avl_delete (&reorder->sampleivtree, coalesced);
    if (coalesced->u.reorder.min <= reorder->next_seq)
      assert (min <= reorder->next_seq);
    reorder->next_seq = coalesced->u.reorder.maxp1;
    reorder->max_sampleiv = avl_findmax (&reorder->sampleivtree);
    nn_log (LC_RADMIN, "  next expected: %lld\n", reorder->next_seq);
    *sc = coalesced->u.reorder.sc;

    /* Adjust n_samples */
    assert (nsamples >= 1);
    assert (reorder->n_samples >= nsamples);
    reorder->n_samples -= nsamples;
    return NN_REORDER_DELIVER;
  }
  else
  {
    nn_log (LC_RADMIN, "  coalesced = [%lld,%lld) @ %p - that is all\n",
            coalesced->u.reorder.min, coalesced->u.reorder.maxp1, (void *) coalesced);
    reorder->max_sampleiv = avl_findmax (&reorder->sampleivtree);
    return NN_REORDER_REJECT;
  }
}

int nn_reorder_nackmap (struct nn_reorder *reorder, os_int64 maxseq, struct nn_sequence_number_set *map, os_uint32 maxsz)
{
  struct nn_rsample *iv;
  os_int64 base, i;

  /* reorder->next_seq-1 is the last one we delivered, so the last one
     we ack; maxseq is the latest sample we know exists.  Valid bitmap
     lengths are 1 .. 256, so maxsz must be within that range, except
     that we allow length-0 bitmaps here as well.  Map->numbits is
     bounded by max(based on sequence numbers, maxsz). */
  assert (maxsz <= 256);
  base = reorder->next_seq;
  if (maxseq + 1 < base)
  {
    nn_log (LC_ERROR, "nn_reorder_nackmap: incorrect max sequence number supplied (maxseq %lld base %lld)\n", maxseq, base);
    maxseq = base - 1;
  }

  map->bitmap_base = nn_toSN (base);
  if (maxseq + 1 - base > maxsz)
    map->numbits = maxsz;
  else
    map->numbits = (os_uint32) (maxseq + 1 - base);
  nn_bitset_zero (map->numbits, map->bits);

  if ((iv = avl_findmin (&reorder->sampleivtree)) != NULL)
    assert (iv->u.reorder.min > base);
  i = base;
  while (iv && i < base + map->numbits)
  {
    for (; i < base + map->numbits && i < iv->u.reorder.min; i++)
    {
      int x = (int) (i - base);
      nn_bitset_set (map->numbits, map->bits, x);
    }
    i = iv->u.reorder.maxp1;
    iv = avl_findsucc (&reorder->sampleivtree, iv);
  }
  for (; i < base + map->numbits; i++)
  {
    int x = (int) (i - base);
    nn_bitset_set (map->numbits, map->bits, x);
  }
  return map->numbits;
}

os_int64 nn_reorder_next_seq (const struct nn_reorder *reorder)
{
  return reorder->next_seq;
}

/* DQUEUE -------------------------------------------------------------- */

struct nn_dqueue {
  os_mutex lock;
  os_cond cond;
  nn_dqueue_handler_t handler;
  void *handler_arg;

  struct nn_rsample_chain sc;

  os_threadId tid;
  char *name;
};

enum dqueue_elem_kind {
  DQEK_DATA,
  DQEK_GAP,
  DQEK_BUBBLE
};

enum nn_dqueue_bubble_kind {
  NN_DQBK_STOP, /* _not_ os_malloc()ed! */
  NN_DQBK_CALLBACK
};

struct nn_dqueue_bubble {
  /* sample_chain_elem must be first: and is used to link it into the
     queue, with the sampleinfo pointing to itself, but mangled */
  struct nn_rsample_chain_elem sce;

  enum nn_dqueue_bubble_kind kind;
  union {
    /* stop */
    struct {
      nn_dqueue_callback_t cb;
      void *arg;
    } cb;
  } u;
};

static enum dqueue_elem_kind dqueue_elem_kind (const struct nn_rsample_chain_elem *e)
{
  if (e->sampleinfo == NULL)
    return DQEK_GAP;
  else if ((char *) e->sampleinfo != (char *) e)
    return DQEK_DATA;
  else
    return DQEK_BUBBLE;
}

static void *dqueue_thread (struct nn_dqueue *q)
{
  int keepgoing = 1;
  os_mutexLock (&q->lock);
  while (keepgoing)
  {
    struct nn_rsample_chain sc;
    if (q->sc.first == NULL)
      os_condWait (&q->cond, &q->lock);
    sc = q->sc;
    q->sc.first = q->sc.last = NULL;
    os_mutexUnlock (&q->lock);

    while (sc.first)
    {
      struct nn_rsample_chain_elem *e = sc.first;
      int ret;
      sc.first = e->next;
      switch (dqueue_elem_kind (e))
      {
        case DQEK_DATA:
          ret = q->handler (e->sampleinfo, e->fragchain, q->handler_arg);
          assert (ret == 0); /* so every handler will return 0 */
          /* FALLS THROUGH */
        case DQEK_GAP:
          nn_fragchain_unref (e->fragchain);
          break;

        case DQEK_BUBBLE:
          {
            struct nn_dqueue_bubble *b = (struct nn_dqueue_bubble *) e->sampleinfo;
            if (b->kind == NN_DQBK_STOP)
            {
              /* Stuff enqueued behind the bubble will still be
                 processed, we do want to drain the queue.  Nothing
                 may be queued anymore once we queue the stop bubble,
                 so q->sc.first should be empty.  If it isn't
                 ... dqueue_free fail an assertion.  STOP bubble
                 doesn't get malloced, and hence not freed. */
              keepgoing = 0;
            }
            else
            {
              switch (b->kind)
              {
                case NN_DQBK_STOP:
                  abort ();
                case NN_DQBK_CALLBACK:
                  b->u.cb.cb (b->u.cb.arg);
                  break;
              }
              os_free (b);
            }
            break;
          }
      }
    }

    os_mutexLock (&q->lock);
  }
  os_mutexUnlock (&q->lock);
  return NULL;
}

struct nn_dqueue *nn_dqueue_new (const char *name, nn_dqueue_handler_t handler, void *arg)
{
  struct nn_dqueue *q;
  os_mutexAttr mattr;
  os_threadAttr tattr;
  os_condAttr cattr;

  if ((q = os_malloc (sizeof (*q))) == NULL)
    goto fail_q;
  if ((q->name = os_strdup (name)) == NULL)
    goto fail_name;
  q->handler = handler;
  q->handler_arg = arg;
  q->sc.first = q->sc.last = NULL;

  os_mutexAttrInit (&mattr);
  mattr.scopeAttr = OS_SCOPE_PRIVATE;
  if (os_mutexInit (&q->lock, &mattr) != os_resultSuccess)
    goto fail_lock;

  os_condAttrInit (&cattr);
  cattr.scopeAttr = OS_SCOPE_PRIVATE;
  if (os_condInit (&q->cond, &q->lock, &cattr) != os_resultSuccess)
    goto fail_cond;

  os_threadAttrInit (&tattr);
  if (os_threadCreate (&q->tid, "dqueue", &tattr, (void * (*) (void *)) dqueue_thread, q) != os_resultSuccess)
    goto fail_thread;
  return q;

 fail_thread:
  os_condDestroy (&q->cond);
 fail_cond:
  os_mutexDestroy (&q->lock);
 fail_lock:
  os_free (q->name);
 fail_name:
  os_free (q);
 fail_q:
  return NULL;
}

void nn_dqueue_enqueue (struct nn_dqueue *q, struct nn_rsample_chain *sc)
{
  assert (sc->first);
  assert (sc->last->next == NULL);
  os_mutexLock (&q->lock);
  if (q->sc.first)
  {
    q->sc.last->next = sc->first;
    q->sc.last = sc->last;
    os_mutexUnlock (&q->lock);
  }
  else
  {
    q->sc = *sc;
    os_mutexUnlock (&q->lock);
    os_condSignal (&q->cond);
  }
}

static void nn_dqueue_enqueue_bubble (struct nn_dqueue *q, struct nn_dqueue_bubble *b)
{
  b->sce.next = NULL;
  b->sce.fragchain = NULL;
  b->sce.sampleinfo = (struct nn_rsample_info *) b;
  os_mutexLock (&q->lock);
  if (q->sc.first)
  {
    q->sc.last->next = &b->sce;
    q->sc.last = &b->sce;
    os_mutexUnlock (&q->lock);
  }
  else
  {
    q->sc.first = q->sc.last = &b->sce;
    os_mutexUnlock (&q->lock);
    os_condSignal (&q->cond);
  }
}

void nn_dqueue_enqueue_callback (struct nn_dqueue *q, nn_dqueue_callback_t cb, void *arg)
{
  struct nn_dqueue_bubble *b;
  b = os_malloc (sizeof (*b));
  b->kind = NN_DQBK_CALLBACK;
  b->u.cb.cb = cb;
  b->u.cb.arg = arg;
  nn_dqueue_enqueue_bubble (q, b);
}

void nn_dqueue_free (struct nn_dqueue *q)
{
  /* There must not be any thread enqueueing things anymore at this
     point.  The stop bubble is special in that it does _not_ get
     malloced or freed, but instead lives on the stack for a little
     while.  It would be a shame to fail in free() due to a lack of
     heap space, would it not? */
  struct nn_dqueue_bubble b;
  b.kind = NN_DQBK_STOP;
  nn_dqueue_enqueue_bubble (q, &b);

  os_threadWaitExit (q->tid, (void **) 0);
  assert (q->sc.first == NULL);
  os_condDestroy (&q->cond);
  os_mutexDestroy (&q->lock);
  os_free (q->name);
  os_free (q);
}
