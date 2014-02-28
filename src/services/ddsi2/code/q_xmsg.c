#include <ctype.h>
#include <stddef.h>
#include <assert.h>

#ifndef _WIN32
#include <limits.h> /* for IOV_MAX */
#endif

#include "os_heap.h"
#include "os_if.h"
#include "os_semaphore.h"

#include "ut_avl.h"
#include "ut_thread_pool.h"

#include "q_osplser.h"
#include "q_protocol.h"
#include "q_xqos.h"
#include "q_bswap.h"
#include "q_rtps.h"
#include "q_addrset.h"
#include "q_error.h"
#include "q_misc.h"
#include "q_log.h"
#include "q_unused.h"
#include "q_xmsg.h"
#include "q_align.h"
#include "q_config.h"
#include "q_entity.h"
#include "q_pcap.h"
#include "q_globals.h"
#include "q_ephash.h"

#include "sysdeps.h"

#define NN_XMSG_MAX_ALIGN 8
#define NN_XMSG_CHUNK_SIZE 128

#if HAVE_ATOMIC_LIFO && ! defined XMSGPOOL_STATISTICS
#define USE_ATOMIC_LIFO 1
#else
#define USE_ATOMIC_LIFO 0
#endif

struct nn_xmsgpool {
#if USE_ATOMIC_LIFO
  os_atomic_lifo_t freelist;
#else
  os_mutex lock;
  int nalloced;
  int nfree;
  struct nn_xmsg_chain_elem *freelist;
#endif
};

struct nn_xmsg_data {
  InfoSRC_t src;
  InfoDST_t dst;
  char payload[1]; /* of size maxsz */
};

struct nn_xmsg_chain_elem {
  struct nn_xmsg_chain_elem *older;
};

enum nn_xmsg_dstmode {
  NN_XMSG_DST_UNSET,
  NN_XMSG_DST_ONE,
  NN_XMSG_DST_ALL
};

struct nn_xmsg {
  struct nn_xmsgpool *pool;
  unsigned maxsz;
  unsigned sz;
  int have_params;
  struct serdata *refd_payload;
  struct iovec refd_payload_iov;
  os_int64 maxdelay;

  /* Backref for late updating of available sequence numbers, and
     merging of retransmits. */
  enum nn_xmsg_kind kind;
  union {
    char control;
    struct {
      nn_guid_t wrguid;
      os_int64 wrseq;
      nn_fragment_number_t wrfragid;
      /* readerId encodes offset to destination readerId or 0 -- used
         only for rexmits, but more convenient to combine both into
         one struct in the union */
      unsigned readerId_off;
    } data;
  } kindspecific;

  enum nn_xmsg_dstmode dstmode;
  union {
    struct {
      os_sockaddr_storage loc;  /* send just to this locator */
    } one;
    struct {
      struct addrset *as;       /* send to all addresses in set */
    } all;
  } dstaddr;

  struct nn_xmsg_chain_elem link;
  struct nn_xmsg_data *data;
};

/* Worst-case: change of SRC [+1] but no DST, submessage [+1], ref'd
   payload [+1].  So 128 iovecs => at least ~40 submessages, so for
   very small ones still >1kB. */
#define NN_XMSG_MAX_SUBMESSAGE_IOVECS 3

#ifdef IOV_MAX
#if IOV_MAX > 0 && IOV_MAX < 128
#define NN_XMSG_MAX_MESSAGE_IOVECS IOV_MAX
#endif
#endif /* defined IOV_MAX */
#ifndef NN_XMSG_MAX_MESSAGE_IOVECS
#define NN_XMSG_MAX_MESSAGE_IOVECS 128
#endif

/* Used to keep them in order, but it now transpires that delayed
   updating of writer seq nos benefits from having them in the
   reverse order.  They are not being used for anything else, so
   we no longer maintain a pointer to both ends. */
struct nn_xmsg_chain {
  struct nn_xmsg_chain_elem *latest;
};


struct nn_xpack 
{
  Header_t hdr;
  MsgLen_t msg_len;
  nn_guid_prefix_t *last_src;
  InfoDST_t *last_dst;
  os_int64 maxdelay;
  unsigned packetid;
  os_uint32 calls;
  ddsi_tran_conn_t conn;
  os_sem_t sem;
  os_size_t niov;
  struct iovec iov[NN_XMSG_MAX_MESSAGE_IOVECS];
  enum nn_xmsg_dstmode dstmode;

  union
  {
    os_sockaddr_storage loc;    /* send just to this locator */
    struct addrset *as;         /* send to all addresses in set */
  } dstaddr;

  struct nn_xmsg_chain included_msgs;

#ifndef NDEBUG
  os_threadId owner_tid;
#endif



};

/* XMSGPOOL ------------------------------------------------------------

   Great expectations, but so far still wanting. */

static void nn_xmsg_realfree (struct nn_xmsg *m);

struct nn_xmsgpool *nn_xmsgpool_new (void)
{
  struct nn_xmsgpool *pool;
  pool = os_malloc (sizeof (*pool));
#if USE_ATOMIC_LIFO
  os_atomic_lifo_init (&pool->freelist);
#else
  os_mutexInit (&pool->lock, &gv.mattr);
  pool->freelist = NULL;
  pool->nalloced = 0;
  pool->nfree = 0;
#endif
  return pool;
}

void nn_xmsgpool_free (struct nn_xmsgpool *pool)
{
#if USE_ATOMIC_LIFO
  struct nn_xmsg *m;
  while ((m = os_atomic_lifo_pop (&pool->freelist, offsetof (struct nn_xmsg, link.older))) != NULL)
    nn_xmsg_realfree (m);
  TRACE (("xmsgpool_free(%p)\n", pool));
#else
  while (pool->freelist)
  {
    struct nn_xmsg *m = (struct nn_xmsg *) ((char *) pool->freelist - offsetof (struct nn_xmsg, link));
    pool->freelist = pool->freelist->older;
    nn_xmsg_realfree (m);
  }
  os_mutexDestroy (&pool->lock);
  TRACE (("xmsgpool_free(%p) nalloced %d nfree %d\n", pool, pool->nalloced, pool->nfree));
#endif
  os_free (pool);
}

/* XMSG ----------------------------------------------------------------

   All messages that are sent start out as xmsgs, which is a sequence
   of submessages potentially ending with a blob of serialized data.
   Such serialized data is given as a reference to part of a serdata.

   An xmsg can be queued for transmission, after which it must be
   forgotten by its creator.  The queue handler packs them into xpacks
   (see below), transmits them, and releases them.

   Currently, the message pool is fake, so 2 mallocs and frees are
   needed for each message, and additionally, it involves address set
   manipulations.  The latter is especially inefficiently dealt with
   in the xpack. */

static void nn_xmsg_reinit (struct nn_xmsg *m, enum nn_xmsg_kind kind)
{
  m->sz = 0;
  m->have_params = 0;
  m->refd_payload = NULL;
  m->dstmode = NN_XMSG_DST_UNSET;
  m->kind = kind;
  m->maxdelay = 0;
  memset (&m->kindspecific, 0, sizeof (m->kindspecific));
}

static struct nn_xmsg *nn_xmsg_allocnew (struct nn_xmsgpool *pool, unsigned expected_size, enum nn_xmsg_kind kind)
{
  const nn_vendorid_t myvendorid = MY_VENDOR_ID;
  struct nn_xmsg *m;
  struct nn_xmsg_data *d;

  if (expected_size == 0)
    expected_size = NN_XMSG_CHUNK_SIZE;

  if ((m = os_malloc (sizeof (*m))) == NULL)
    return NULL;

  m->pool = pool;
  m->maxsz = (expected_size + NN_XMSG_CHUNK_SIZE - 1) & -NN_XMSG_CHUNK_SIZE;

  if ((d = m->data = os_malloc (offsetof (struct nn_xmsg_data, payload) + m->maxsz)) == NULL)
  {
    os_free (m);
    return NULL;
  }
  d->src.smhdr.submessageId = SMID_INFO_SRC;
  d->src.smhdr.flags = (PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0);
  d->src.smhdr.octetsToNextHeader = sizeof (d->src) - (offsetof (InfoSRC_t, smhdr.octetsToNextHeader) + 2);
  d->src.unused = 0;
  d->src.version.major = RTPS_MAJOR;
  d->src.version.minor = RTPS_MINOR;
  d->src.vendorid = myvendorid;
  d->dst.smhdr.submessageId = SMID_INFO_DST;
  d->dst.smhdr.flags = (PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0);
  d->dst.smhdr.octetsToNextHeader = sizeof (d->dst.guid_prefix);
  nn_xmsg_reinit (m, kind);
  return m;
}

struct nn_xmsg *nn_xmsg_new (struct nn_xmsgpool *pool, const nn_guid_prefix_t *src_guid_prefix, unsigned expected_size, enum nn_xmsg_kind kind)
{
  struct nn_xmsg *m;
#if USE_ATOMIC_LIFO
  if ((m = os_atomic_lifo_pop (&pool->freelist, offsetof (struct nn_xmsg, link.older))) != NULL)
    nn_xmsg_reinit (m, kind);
  else if ((m = nn_xmsg_allocnew (pool, expected_size, kind)) == NULL)
    return NULL;
#else
  os_mutexLock (&pool->lock);
  if (pool->freelist)
  {
    m = (struct nn_xmsg *) ((char *) pool->freelist - offsetof (struct nn_xmsg, link));
    pool->freelist = pool->freelist->older;
    pool->nfree--;
    os_mutexUnlock (&pool->lock);
    nn_xmsg_reinit (m, kind);
  }
  else
  {
    pool->nalloced++;
    os_mutexUnlock (&pool->lock);
    if ((m = nn_xmsg_allocnew (pool, expected_size, kind)) == NULL)
      return NULL;
  }
#endif
  m->data->src.guid_prefix = nn_hton_guid_prefix (*src_guid_prefix);
  return m;
}

static void nn_xmsg_realfree (struct nn_xmsg *m)
{
  os_free (m->data);
  os_free (m);
}

void nn_xmsg_free (struct nn_xmsg *m)
{
  struct nn_xmsgpool *pool = m->pool;
  if (m->refd_payload)
    serdata_unref (m->refd_payload);
  if (m->dstmode == NN_XMSG_DST_ALL)
    unref_addrset (m->dstaddr.all.as);
#if USE_ATOMIC_LIFO
  os_atomic_lifo_push (&pool->freelist, m, offsetof (struct nn_xmsg, link.older));
#else
  os_mutexLock (&pool->lock);
  m->link.older = pool->freelist;
  pool->freelist = &m->link;
  pool->nfree++;
  os_mutexUnlock (&pool->lock);
#endif
}

/************************************************/

#ifndef NDEBUG
static int submsg_is_compatible (const struct nn_xmsg *msg, SubmessageKind_t smkind)
{
  switch (msg->kind)
  {
    case NN_XMSG_KIND_CONTROL:
      switch (smkind)
      {
        case SMID_PAD:
          /* never use this one -- so let's crash when we do :) */
          return 0;
        case SMID_INFO_SRC: case SMID_INFO_REPLY_IP4:
        case SMID_INFO_DST: case SMID_INFO_REPLY:
          /* we never generate these directly */
          return 0;
        case SMID_INFO_TS:
        case SMID_ACKNACK: case SMID_HEARTBEAT:
        case SMID_GAP: case SMID_NACK_FRAG:
        case SMID_HEARTBEAT_FRAG:
        case SMID_PT_INFO_CONTAINER:
        case SMID_PT_MSG_LEN:
        case SMID_PT_ENTITY_ID:
          /* normal control stuff is ok */
          return 1;
        case SMID_DATA: case SMID_DATA_FRAG:
          /* but data is strictly verboten */
          return 0;
      }
      assert (0);
      break;
    case NN_XMSG_KIND_DATA:
    case NN_XMSG_KIND_DATA_REXMIT:
      switch (smkind)
      {
        case SMID_PAD:
          /* never use this one -- so let's crash when we do :) */
          return 0;
        case SMID_INFO_SRC: case SMID_INFO_REPLY_IP4:
        case SMID_INFO_DST: case SMID_INFO_REPLY:
          /* we never generate these directly */
          return 0;
        case SMID_INFO_TS: case SMID_DATA: case SMID_DATA_FRAG:
          /* Timestamp only preceding data; data may be present just
             once for rexmits.  The readerId offset can be used to
             ensure rexmits have only one data submessages -- the test
             won't work for initial transmits, but those currently
             don't allow a readerId */
          return msg->kindspecific.data.readerId_off == 0;
        case SMID_ACKNACK:
        case SMID_HEARTBEAT:
        case SMID_GAP:
        case SMID_NACK_FRAG:
        case SMID_HEARTBEAT_FRAG:
        case SMID_PT_INFO_CONTAINER:
        case SMID_PT_MSG_LEN:
        case SMID_PT_ENTITY_ID:
          /* anything else is strictly verboten */
          return 0;
      }
      assert (0);
      break;
  }
  assert (0);
  return 1;
}
#endif

int nn_xmsg_compare_fragid (const struct nn_xmsg *a, const struct nn_xmsg *b)
{
  int c;
  assert (a->kind == NN_XMSG_KIND_DATA_REXMIT);
  assert (b->kind == NN_XMSG_KIND_DATA_REXMIT);
  /* I think most likely discriminator is seq, then writer guid, then
     fragid, but we'll stick to the "expected" order for now: writer,
     seq, frag */
  if ((c = memcmp (&a->kindspecific.data.wrguid, &b->kindspecific.data.wrguid, sizeof (a->kindspecific.data.wrguid))) != 0)
    return c;
  else if (a->kindspecific.data.wrseq != b->kindspecific.data.wrseq)
    return (a->kindspecific.data.wrseq < b->kindspecific.data.wrseq) ? -1 : 1;
  else if (a->kindspecific.data.wrfragid != b->kindspecific.data.wrfragid)
    return (a->kindspecific.data.wrfragid < b->kindspecific.data.wrfragid) ? -1 : 1;
  else
    return 0;
}

unsigned nn_xmsg_size (const struct nn_xmsg *m)
{
  return m->sz;
}

enum nn_xmsg_kind nn_xmsg_kind (const struct nn_xmsg *m)
{
  return m->kind;
}

void nn_xmsg_guid_seq_fragid (const struct nn_xmsg *m, nn_guid_t *wrguid, os_int64 *wrseq, nn_fragment_number_t *wrfragid)
{
  assert (m->kind != NN_XMSG_KIND_CONTROL);
  *wrguid = m->kindspecific.data.wrguid;
  *wrseq = m->kindspecific.data.wrseq;
  *wrfragid = m->kindspecific.data.wrfragid;
}

void *nn_xmsg_payload (unsigned *sz, struct nn_xmsg *m)
{
  *sz = m->sz;
  return m->data->payload;
}

void nn_xmsg_submsg_init (struct nn_xmsg *msg, struct nn_xmsg_marker marker, SubmessageKind_t smkind)
{
  SubmessageHeader_t *hdr = (SubmessageHeader_t *) (msg->data->payload + marker.offset);
  assert (submsg_is_compatible (msg, smkind));
  hdr->submessageId = smkind;
  hdr->flags = PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0;
  hdr->octetsToNextHeader = 0;
}

void nn_xmsg_submsg_setnext (struct nn_xmsg *msg, struct nn_xmsg_marker marker)
{
  SubmessageHeader_t *hdr = (SubmessageHeader_t *) (msg->data->payload + marker.offset);
  unsigned plsize = msg->refd_payload ? (unsigned) msg->refd_payload_iov.iov_len : 0;
  assert ((msg->sz % 4) == 0);
  assert ((plsize % 4) == 0);
  assert ((unsigned) (msg->data->payload + msg->sz + plsize - (char *) hdr) >= RTPS_SUBMESSAGE_HEADER_SIZE);
  hdr->octetsToNextHeader = (unsigned short)
    (msg->data->payload + msg->sz + plsize - (char *) hdr) - RTPS_SUBMESSAGE_HEADER_SIZE;
}

void * nn_xmsg_append (struct nn_xmsg *m, struct nn_xmsg_marker *marker, unsigned sz)
{
  static const unsigned a = 4;

  /* May realloc, in which case m may change.  But that will not
     happen if you do not exceed expected_size.  Max size is always a
     multiple of A: that means we don't have to worry about memory
     available just for alignment. */
  char *p;
  assert (1 <= a && a <= NN_XMSG_MAX_ALIGN);
  assert ((m->maxsz % a) == 0);
  if ((m->sz % a) != 0)
  {
    int npad = a - (m->sz % a);
    memset (m->data->payload + m->sz, 0, npad);
    m->sz += npad;
  }
  if (m->sz + sz > m->maxsz)
  {
    unsigned nmax = (m->maxsz + sz + NN_XMSG_CHUNK_SIZE - 1) & -NN_XMSG_CHUNK_SIZE;
    struct nn_xmsg_data *ndata = os_realloc (m->data, offsetof (struct nn_xmsg_data, payload) + nmax);
    m->maxsz = nmax;
    m->data = ndata;
  }
  p = m->data->payload + m->sz;
  if (marker)
    marker->offset = m->sz;
  m->sz += sz;
  return p;
}

void nn_xmsg_shrink (struct nn_xmsg *m, struct nn_xmsg_marker marker, unsigned sz)
{
  assert (m != NULL);
  assert (marker.offset <= m->sz);
  assert (marker.offset + sz <= m->sz);
  m->sz = marker.offset + sz;
}

int nn_xmsg_add_timestamp (struct nn_xmsg *m, os_int64 t)
{
  InfoTimestamp_t *ts;
  struct nn_xmsg_marker sm;

  ts = (InfoTimestamp_t *) nn_xmsg_append (m, &sm, sizeof (InfoTimestamp_t));
  nn_xmsg_submsg_init (m, sm, SMID_INFO_TS);
  ts->time = nn_to_ddsi_time (t);
  nn_xmsg_submsg_setnext (m, sm);
  return 0;
}

int nn_xmsg_serdata (struct nn_xmsg *m, serdata_t serdata, unsigned off, unsigned len)
{
  unsigned len4 = ALIGN4 (len);
  assert (m->refd_payload == NULL);
  m->refd_payload = serdata_ref (serdata);
  m->refd_payload_iov.iov_base = (char *) &m->refd_payload->hdr + off;
  m->refd_payload_iov.iov_len = len4;
  return 0;
}

int nn_xmsg_setdst1 (struct nn_xmsg *m, const nn_guid_prefix_t *gp, const os_sockaddr_storage *addr)
{
  assert (m->dstmode == NN_XMSG_DST_UNSET);
  m->dstmode = NN_XMSG_DST_ONE;
  m->dstaddr.one.loc = *addr;
  m->data->dst.guid_prefix = nn_hton_guid_prefix (*gp);
  return 0;
}

int nn_xmsg_setdstPRD (struct nn_xmsg *m, const struct proxy_reader *prd)
{
  os_sockaddr_storage loc;
  if (addrset_any_uc (prd->c.as, &loc) || addrset_any_mc (prd->c.as, &loc))
    nn_xmsg_setdst1 (m, &prd->e.guid.prefix, &loc);
  return 0;
}

int nn_xmsg_setdstN (struct nn_xmsg *m, struct addrset *as)
{
  assert (m->dstmode == NN_XMSG_DST_UNSET || m->dstmode == NN_XMSG_DST_ONE);
  m->dstmode = NN_XMSG_DST_ALL;
  m->dstaddr.all.as = ref_addrset (as);
  return 0;
}

int nn_xmsg_set_data_readerId (struct nn_xmsg *m, nn_entityid_t *readerId)
{
  assert (m->kind == NN_XMSG_KIND_DATA_REXMIT);
  assert (m->kindspecific.data.readerId_off == 0);
  assert ((char *) readerId > m->data->payload);
  assert ((char *) readerId < m->data->payload + m->sz);
  m->kindspecific.data.readerId_off = (unsigned) ((char *) readerId - m->data->payload);
  return 0;
}

static void clear_readerId (struct nn_xmsg *m)
{
  assert (m->kind == NN_XMSG_KIND_DATA_REXMIT);
  assert (m->kindspecific.data.readerId_off != 0);
  *((nn_entityid_t *) (m->data->payload + m->kindspecific.data.readerId_off)) =
    nn_hton_entityid (to_entityid (NN_ENTITYID_UNKNOWN));
}

static nn_entityid_t load_readerId (const struct nn_xmsg *m)
{
  assert (m->kind == NN_XMSG_KIND_DATA_REXMIT);
  assert (m->kindspecific.data.readerId_off != 0);
  return nn_ntoh_entityid (*((nn_entityid_t *) (m->data->payload + m->kindspecific.data.readerId_off)));
}

static int readerId_compatible (const struct nn_xmsg *m, const struct nn_xmsg *madd)
{
  nn_entityid_t e = load_readerId (m);
  nn_entityid_t eadd = load_readerId (madd);
  return e.u == NN_ENTITYID_UNKNOWN || e.u == eadd.u;
}

int nn_xmsg_merge_rexmit_destinations_wrlock_held (struct nn_xmsg *m, const struct nn_xmsg *madd)
{
  assert (m->kindspecific.data.wrseq >= 1);
  assert (m->kindspecific.data.wrguid.prefix.u[0] != 0);
  assert (is_writer_entityid (m->kindspecific.data.wrguid.entityid));
  assert (memcmp (&m->kindspecific.data.wrguid, &madd->kindspecific.data.wrguid, sizeof (m->kindspecific.data.wrguid)) == 0);
  assert (m->kindspecific.data.wrseq == madd->kindspecific.data.wrseq);
  assert (m->kindspecific.data.wrfragid == madd->kindspecific.data.wrfragid);
  assert (m->kind == NN_XMSG_KIND_DATA_REXMIT);
  assert (madd->kind == NN_XMSG_KIND_DATA_REXMIT);
  assert (m->kindspecific.data.readerId_off != 0);
  assert (madd->kindspecific.data.readerId_off != 0);

  TRACE ((" (%x:%x:%x:%x#%lld/%u:",
          PGUID (m->kindspecific.data.wrguid), m->kindspecific.data.wrseq, m->kindspecific.data.wrfragid + 1));

  switch (m->dstmode)
  {
    case NN_XMSG_DST_UNSET:
      assert (0);
      return 0;

    case NN_XMSG_DST_ALL:
      TRACE (("*->*)"));
      return 1;

    case NN_XMSG_DST_ONE:
      switch (madd->dstmode)
      {
        case NN_XMSG_DST_UNSET:
          assert (0);
          return 0;

        case NN_XMSG_DST_ALL:
          TRACE (("1+*->*)"));
          clear_readerId (m);
          m->dstmode = NN_XMSG_DST_ALL;
          m->dstaddr.all.as = ref_addrset (madd->dstaddr.all.as);
          return 1;

        case NN_XMSG_DST_ONE:
          if (memcmp (&m->data->dst.guid_prefix, &madd->data->dst.guid_prefix, sizeof (m->data->dst.guid_prefix)) != 0)
          {
            struct writer *wr;
            /* This is why wr->e.lock must be held: we can't safely
               reference the writer's address set if it isn't -- so
               FIXME: add a way to atomically replace the contents of
               an addrset in rebuild_writer_addrset: then we don't
               need the lock anymore, and the '_wrlock_held' suffix
               can go and everyone's life will become easier! */
            if ((wr = ephash_lookup_writer_guid (&m->kindspecific.data.wrguid)) == NULL)
            {
              TRACE (("writer-dead)"));
              return 0;
            }
            else
            {
              TRACE (("1+1->*)"));
              clear_readerId (m);
              m->dstmode = NN_XMSG_DST_ALL;
              m->dstaddr.all.as = ref_addrset (wr->as);
              return 1;
            }
          }
          else if (readerId_compatible (m, madd))
          {
            TRACE (("1+1->1)"));
            return 1;
          }
          else
          {
            TRACE (("1+1->2)"));
            clear_readerId (m);
            return 1;
          }
      }
      break;
  }
  assert (0);
  return 0;
}

int nn_xmsg_setmaxdelay (struct nn_xmsg *msg, os_int64 maxdelay)
{
  assert (msg->maxdelay == 0);
  msg->maxdelay = maxdelay;
  return 0;
}


int nn_xmsg_setwriterseq (struct nn_xmsg *msg, const nn_guid_t *wrguid, os_int64 wrseq)
{
  msg->kindspecific.data.wrguid = *wrguid;
  msg->kindspecific.data.wrseq = wrseq;
  return 0;
}

int nn_xmsg_setwriterseq_fragid (struct nn_xmsg *msg, const nn_guid_t *wrguid, os_int64 wrseq, nn_fragment_number_t wrfragid)
{
  nn_xmsg_setwriterseq (msg, wrguid, wrseq);
  msg->kindspecific.data.wrfragid = wrfragid;
  return 0;
}

void *nn_xmsg_addpar (struct nn_xmsg *m, int pid, int len)
{
  const int len4 = (len + 3) & -4; /* must alloc a multiple of 4 */
  nn_parameter_t *phdr;
  char *p;
  m->have_params = 1;
  phdr = nn_xmsg_append (m, NULL, sizeof (nn_parameter_t) + len4);
  phdr->parameterid = pid;
  phdr->length = len4;
  p = (char *) (phdr + 1);
  if (len4 > len)
  {
    /* zero out padding bytes added to satisfy parameter alignment --
       alternative: zero out, but this way valgrind/purify can tell us
       where we forgot to initialize something */
    memset (p + len, 0, len4 - len);
  }
  return p;
}

int nn_xmsg_addpar_string (struct nn_xmsg *m, int pid, const char *str)
{
  struct cdrstring *p;
  int len = (int) strlen (str) + 1;
  if ((p = nn_xmsg_addpar (m, pid, 4 + len)) == NULL)
    return ERR_OUT_OF_MEMORY;
  p->length = len;
  memcpy (p->contents, str, len);
  return 0;
}

int nn_xmsg_addpar_octetseq (struct nn_xmsg *m, int pid, const nn_octetseq_t *oseq)
{
  char *p;
  if ((p = nn_xmsg_addpar (m, pid, 4 + oseq->length)) == NULL)
    return ERR_OUT_OF_MEMORY;
  *((int *) p) = oseq->length;
  memcpy (p + sizeof (int), oseq->value, oseq->length);
  return 0;
}

int nn_xmsg_addpar_stringseq (struct nn_xmsg *m, int pid, const nn_stringseq_t *sseq)
{
  char *tmp;
  int i, len = 0;

  for (i = 0; i < sseq->n; i++)
  {
    int len1 = (int) strlen (sseq->strs[i]) + 1;
    len += 4 + ALIGN4 (len1);
  }

  if ((tmp = nn_xmsg_addpar (m, pid, 4 + len)) == NULL)
    return ERR_OUT_OF_MEMORY;

  *((int *) tmp) = sseq->n;
  tmp += sizeof (int);
  for (i = 0; i < sseq->n; i++)
  {
    struct cdrstring *p = (struct cdrstring *) tmp;
    int len1 = (int) strlen (sseq->strs[i]) + 1;
    p->length = len1;
    memcpy (p->contents, sseq->strs[i], len1);
    if (len1 < ALIGN4 (len1))
      memset (p->contents + len1, 0, ALIGN4 (len1) - len1);
    tmp += 4 + ALIGN4 (len1);
  }
  return 0;
}

int nn_xmsg_addpar_keyhash (struct nn_xmsg *m, const struct serdata *serdata)
{
  char *p;
  if ((p = nn_xmsg_addpar (m, PID_KEYHASH, 16)) == NULL)
    return ERR_OUT_OF_MEMORY;
  serdata_keyhash (serdata, p);
  return 0;
}

int nn_xmsg_addpar_guid (struct nn_xmsg *m, int pid, const nn_guid_t *guid)
{
  unsigned *pu;
  int i;
  if ((pu = nn_xmsg_addpar (m, pid, 16)) == NULL)
    return ERR_OUT_OF_MEMORY;
  for (i = 0; i < 3; i++)
    pu[i] = toBE4u (guid->prefix.u[i]);
  pu[i] = toBE4u (guid->entityid.u);
  return 0;
}

int nn_xmsg_addpar_reliability (struct nn_xmsg *m, int pid, const struct nn_reliability_qospolicy *rq)
{
  struct nn_external_reliability_qospolicy *p;
  if ((p = nn_xmsg_addpar (m, pid, sizeof (*p))) == NULL)
    return ERR_OUT_OF_MEMORY;
  if (NN_PEDANTIC_P)
  {
    switch (rq->kind)
    {
      case NN_BEST_EFFORT_RELIABILITY_QOS:
        p->kind = NN_PEDANTIC_BEST_EFFORT_RELIABILITY_QOS;
        break;
      case NN_RELIABLE_RELIABILITY_QOS:
        p->kind = NN_PEDANTIC_RELIABLE_RELIABILITY_QOS;
        break;
      default:
        assert (0);
    }
  }
  else
  {
    switch (rq->kind)
    {
      case NN_BEST_EFFORT_RELIABILITY_QOS:
        p->kind = NN_INTEROP_BEST_EFFORT_RELIABILITY_QOS;
        break;
      case NN_RELIABLE_RELIABILITY_QOS:
        p->kind = NN_INTEROP_RELIABLE_RELIABILITY_QOS;
        break;
      default:
        assert (0);
    }
  }
  p->max_blocking_time = rq->max_blocking_time;
  return 0;
}

int nn_xmsg_addpar_4u (struct nn_xmsg *m, int pid, unsigned x)
{
  unsigned *p;
  if ((p = nn_xmsg_addpar (m, pid, 4)) == NULL)
    return ERR_OUT_OF_MEMORY;
  *p = x;
  return 0;
}

int nn_xmsg_addpar_BE4u (struct nn_xmsg *m, int pid, unsigned x)
{
  unsigned *p;
  if ((p = nn_xmsg_addpar (m, pid, 4)) == NULL)
    return ERR_OUT_OF_MEMORY;
  *p = toBE4u (x);
  return 0;
}

int nn_xmsg_addpar_statusinfo (struct nn_xmsg *m, unsigned statusinfo)
{
  return nn_xmsg_addpar_BE4u (m, PID_STATUSINFO, statusinfo);
}

int nn_xmsg_addpar_wrinfo (struct nn_xmsg *m, const struct nn_prismtech_writer_info *wri)
{
  struct nn_prismtech_writer_info *p;
  if ((p = nn_xmsg_addpar (m, PID_PRISMTECH_WRITER_INFO, sizeof (*p))) == NULL)
    return ERR_OUT_OF_MEMORY;
  *p = *wri;
  return 0;
}

int nn_xmsg_addpar_sentinel (struct nn_xmsg *m)
{
  if (nn_xmsg_addpar (m, PID_SENTINEL, 0) == NULL)
    return ERR_OUT_OF_MEMORY;
  return 1;
}

int nn_xmsg_addpar_sentinel_ifparam (struct nn_xmsg *m)
{
  if (!m->have_params)
    return 0;
  else
    return nn_xmsg_addpar_sentinel (m);
}

int nn_xmsg_addpar_parvinfo (struct nn_xmsg *m, int pid, const struct nn_prismtech_participant_version_info *pvi)
{
  int i;
  unsigned slen;
  unsigned *pu;
  struct cdrstring *ps;

  /* pvi->internals cannot be NULL here */
  slen = strlen(pvi->internals) + 1; /* +1 for '\0' terminator */
  if ((pu = nn_xmsg_addpar (m, pid, NN_PRISMTECH_PARTICIPANT_VERSION_INFO_FIXED_CDRSIZE + slen)) == NULL)
    return ERR_OUT_OF_MEMORY;
  pu[0] = pvi->version;
  pu[1] = pvi->flags;
  for (i = 0; i < 3; i++)
    pu[i+2] = (pvi->unused[i]);
  ps = (struct cdrstring *)&pu[5];
  ps->length = slen;
  memcpy(ps->contents, pvi->internals, slen);

  return 0;
}

/* XMSG_CHAIN ----------------------------------------------------------

   Xpacks refer to xmsgs and need to release these after having been
   sent.  For that purpose, we have a chain of xmsgs in an xpack.

   Chain elements are embedded in the xmsg, so instead of loading a
   pointer we compute the address of the xmsg from the address of the
   chain element, &c. */

static void nn_xmsg_chain_release (struct nn_xmsg_chain *chain)
{
  nn_guid_t wrguid;
  memset (&wrguid, 0, sizeof (wrguid));

  while (chain->latest)
  {
    struct nn_xmsg_chain_elem *ce = chain->latest;
    struct nn_xmsg *m = (struct nn_xmsg *) ((char *) ce - offsetof (struct nn_xmsg, link));
    chain->latest = ce->older;

    /* If this xmsg was written by a writer different from wrguid,
       update wr->xmit_seq.  There isn't necessarily a writer, and
       for fragmented data, only the last one must be updated, which
       we do by not setting the writer+seq for those xmsgs.

       These are all local writers, and are guaranteed to have the
       same, non-zero, systemId <=> wrguid.u[0].

       They are in reverse order, so we only attempt an update if this
       xmsg was produced by a writer different from the last one we
       processed. */
    if (m->kind == NN_XMSG_KIND_DATA && m->kindspecific.data.wrguid.prefix.u[0])
    {
      if (wrguid.prefix.u[1] != m->kindspecific.data.wrguid.prefix.u[1] ||
          wrguid.prefix.u[2] != m->kindspecific.data.wrguid.prefix.u[2] ||
          wrguid.entityid.u != m->kindspecific.data.wrguid.entityid.u)
      {
        struct writer *wr;
        assert (m->kindspecific.data.wrseq != 0);
        wrguid = m->kindspecific.data.wrguid;
        if ((wr = ephash_lookup_writer_guid (&m->kindspecific.data.wrguid)) != NULL)
        {
          os_mutexLock (&wr->e.lock);
          if (m->kindspecific.data.wrseq > wr->seq_xmit)
          {
#if 0
            TRACE (("seq(%x:%x:%x:%x -> %lld)", PGUID (m->kindspecific.data.wrguid), m->kindspecific.data.wrseq));
#endif
            wr->seq_xmit = m->kindspecific.data.wrseq;
          }
          os_mutexUnlock (&wr->e.lock);
        }
      }
    }

    nn_xmsg_free (m);
  }
}

static void nn_xmsg_chain_add (struct nn_xmsg_chain *chain, struct nn_xmsg *m)
{
  m->link.older = chain->latest;
  chain->latest = &m->link;
}


/* XPACK ---------------------------------------------------------------

   Queued messages are packed into xpacks (all by-ref, using iovecs).
   The xpack is sent to the union of all address sets provided in the
   message added to the xpack.  */

static void nn_xpack_reinit (struct nn_xpack *xp)
{
  xp->dstmode = NN_XMSG_DST_UNSET;
  xp->niov = 0;
  xp->msg_len.length = 0;
  xp->included_msgs.latest = NULL;
  xp->maxdelay = T_NEVER;
  xp->packetid++;
}

struct nn_xpack *nn_xpack_new
(
  ddsi_tran_conn_t conn
)
{
  const nn_vendorid_t myvendorid = MY_VENDOR_ID;
  struct nn_xpack *xp;

  if ((xp = os_malloc (sizeof (*xp))) == NULL)
    return NULL;
#ifndef NDEBUG
  xp->owner_tid = os_threadIdSelf ();
#endif

  /* Fixed header fields, initialized just once */
  xp->hdr.protocol.id[0] = 'R';
  xp->hdr.protocol.id[1] = 'T';
  xp->hdr.protocol.id[2] = 'P';
  xp->hdr.protocol.id[3] = 'S';
  xp->hdr.version.major = RTPS_MAJOR;
  xp->hdr.version.minor = RTPS_MINOR;
  xp->hdr.vendorid = myvendorid;

  /* MSG_LEN first sub message for stream based connections */

  xp->msg_len.smhdr.submessageId = SMID_PT_MSG_LEN;
  xp->msg_len.smhdr.flags = (PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0);
  xp->msg_len.smhdr.octetsToNextHeader = 4;

  xp->conn = conn;
  xp->packetid = 0;
  nn_xpack_reinit (xp);

  os_sem_init (&xp->sem, 0);

  return xp;
}

void nn_xpack_free (struct nn_xpack *xp)
{
  assert (os_threadEqual (os_threadIdSelf (), xp->owner_tid));
  assert (xp->niov == 0);
  assert (xp->included_msgs.latest == NULL);
  os_sem_destroy (&xp->sem);
  os_free (xp);
}


static int sockaddr_size (const os_sockaddr_storage *a)
{
  switch (a->ss_family)
  {
    case AF_INET: return sizeof (os_sockaddr_in);
    case AF_INET6: return sizeof (os_sockaddr_in6);
    default: assert (0); return 0;
  }
}

static void nn_xpack_send1 (const os_sockaddr_storage * addr, void * varg)
{
  struct iovec iov[NN_XMSG_MAX_MESSAGE_IOVECS];
  struct nn_xpack * xp = varg;
  struct msghdr mhdr;
  int nbytes = 0;

  if (config.enabled_logcats & LC_TRACE)
  {
    char buf[INET6_ADDRSTRLEN_EXTENDED];
    TRACE ((" %s", sockaddr_to_string_with_port (buf, addr)));
  }

  if (config.xmit_lossiness > 0)
  {
    /* We drop APPROXIMATELY a fraction of xmit_lossiness * 10**(-3)
       of all packets to be sent */
    if ((random () % 1000) < config.xmit_lossiness)
    {
      TRACE (("(dropped)"));
      return;
    }
  }

  /* Set target data/address in message */

  memcpy (iov, xp->iov, sizeof (iov));
  memset (&mhdr, 0, sizeof (mhdr));
  mhdr.msg_iov = iov;
  mhdr.msg_iovlen = xp->niov;
  mhdr.msg_name = (os_sockaddr_storage*) addr;
  mhdr.msg_namelen = sockaddr_size (addr);

  {
    nbytes = ddsi_conn_write (xp->conn, &mhdr, xp->msg_len.length);
  }

  /* Expected that connection based writes will fail if not yet connected */

  if (nbytes <= 0 && xp->conn->m_connless)
  {
    NN_ERROR1 ("ddsi_conn_write failed %d\n", nbytes);
  }
}

typedef struct nn_xpack_send1_thread_arg
{
  const os_sockaddr_storage * addr;
  struct nn_xpack * xp;
} 
* nn_xpack_send1_thread_arg_t;

static void * nn_xpack_send1_thread (void * varg)
{
  nn_xpack_send1_thread_arg_t arg = varg;
  nn_xpack_send1 (arg->addr, arg->xp);
  if (atomic_dec_u32_nv (&arg->xp->calls) == 0)
  {
    os_sem_post (&arg->xp->sem);
  }
  os_free (varg);
  return NULL;
}

static void nn_xpack_send1_threaded (const os_sockaddr_storage * addr, void * varg)
{
  nn_xpack_send1_thread_arg_t arg = os_malloc (sizeof (*arg));
  arg->xp = (struct nn_xpack *) varg;
  arg->addr = addr;
  ut_thread_pool_submit (gv.thread_pool, nn_xpack_send1_thread, arg);
}

void nn_xpack_send (struct nn_xpack * xp)
{
  os_uint32 calls;

  assert (os_threadEqual (os_threadIdSelf (), xp->owner_tid));
  assert (xp->niov <= NN_XMSG_MAX_MESSAGE_IOVECS);

  if (xp->niov == 0)
  {
    return;
  }

  assert (xp->dstmode != NN_XMSG_DST_UNSET);

  if (config.enabled_logcats & LC_TRACE)
  {
    int i;
    TRACE (("nn_xpack_send %u:", xp->msg_len.length));
    for (i = 0; i < (int) xp->niov; i++)
    {
      TRACE ((" %p:%u", (void *) xp->iov[i].iov_base, xp->iov[i].iov_len));
    }
  }

  TRACE ((" ["));
  if (xp->dstmode == NN_XMSG_DST_ONE)
  {
    calls = 1;
    nn_xpack_send1 (&xp->dstaddr.loc, xp);
  }
  else
  {
    calls = addrset_count (xp->dstaddr.as);
    if (calls)
    {
      if (gv.thread_pool == NULL)
      {
        addrset_forall (xp->dstaddr.as, nn_xpack_send1, xp);
      }
      else
      {
        xp->calls = calls;
        addrset_forall (xp->dstaddr.as, nn_xpack_send1_threaded, xp);
        os_sem_wait (&xp->sem);
      }
    }
    unref_addrset (xp->dstaddr.as);
  }
  TRACE ((" ]\n"));
  if (calls)
  {
    nn_log (LC_TRAFFIC, "traffic-xmit (%d) %u\n", calls, xp->msg_len.length);
  }
  nn_xmsg_chain_release (&xp->included_msgs);
  nn_xpack_reinit (xp);
}

static void copy_addressing_info (struct nn_xpack *xp, const struct nn_xmsg *m)
{
  xp->dstmode = m->dstmode;
  switch (m->dstmode)
  {
    case NN_XMSG_DST_UNSET:
      assert (0);
    case NN_XMSG_DST_ONE:
      xp->dstaddr.loc = m->dstaddr.one.loc;
      break;
    case NN_XMSG_DST_ALL:
      xp->dstaddr.as = ref_addrset (m->dstaddr.all.as);
      break;
  }
}

static int addressing_info_eq_onesidederr (const struct nn_xpack *xp, const struct nn_xmsg *m)
{
  if (xp->dstmode != m->dstmode)
    return 0;
  switch (xp->dstmode)
  {
    case NN_XMSG_DST_UNSET:
      assert (0);
    case NN_XMSG_DST_ONE:
      return (memcmp (&xp->dstaddr.loc, &m->dstaddr.one.loc, sizeof (xp->dstaddr.loc)) == 0);
    case NN_XMSG_DST_ALL:
      return addrset_eq_onesidederr (xp->dstaddr.as, m->dstaddr.all.as);
  }
  assert (0);
  return 0;
}

static int nn_xpack_mayaddmsg (const struct nn_xpack *xp, const struct nn_xmsg *m)
{
  unsigned max_msg_size = config.max_msg_size;
  unsigned payload_size;

  if (xp->niov == 0)
    return 1;
  assert (xp->included_msgs.latest != NULL);
  if (xp->niov + NN_XMSG_MAX_SUBMESSAGE_IOVECS > NN_XMSG_MAX_MESSAGE_IOVECS)
    return 0;

  payload_size = m->refd_payload ? (unsigned) m->refd_payload_iov.iov_len : 0;


  if (xp->msg_len.length + m->sz + payload_size > max_msg_size)
    return 0;


  return addressing_info_eq_onesidederr (xp, m);
}

static int guid_prefix_eq (const nn_guid_prefix_t *a, const nn_guid_prefix_t *b)
{
  return a->u[0] == b->u[0] && a->u[1] == b->u[1] && a->u[2] == b->u[2];
}

int nn_xpack_addmsg (struct nn_xpack *xp, struct nn_xmsg *m)
{
  /* Returns > 0 if pack got sent out before adding m */

  static InfoDST_t static_zero_dst;
  InfoDST_t *dst;
  int niov;
  unsigned sz;
  int result = 0;
  int xpo_niov = 0;
  unsigned xpo_sz = 0;

  assert (m->kind != NN_XMSG_KIND_DATA_REXMIT || m->kindspecific.data.readerId_off != 0);

  assert (os_threadEqual (os_threadIdSelf (), xp->owner_tid));
  assert (m->sz > 0);
  assert (m->dstmode != NN_XMSG_DST_UNSET);

  /* Submessage offset must be a multiple of 4 to meet alignment
     requirement (DDSI 2.1, 9.4.1).  If we keep everything 4-byte
     aligned all the time, we don't need to check for padding here. */
  assert ((xp->msg_len.length % 4) == 0);
  assert ((m->sz % 4) == 0);
  assert (m->refd_payload == NULL || (m->refd_payload_iov.iov_len % 4) == 0);

  if (!nn_xpack_mayaddmsg (xp, m))
  {
    assert (xp->niov > 0);
    nn_xpack_send (xp);
    assert (nn_xpack_mayaddmsg (xp, m));
    result = 1;
  }

  niov = xp->niov;
  sz = xp->msg_len.length;

  /* We try to merge iovecs, but we can never merge across messages
     because of all the headers. So we can speculatively start adding
     the submessage to the pack, and if we can't transmit and restart.
     But do make sure we can't run out of iovecs. */
  assert (niov + NN_XMSG_MAX_SUBMESSAGE_IOVECS <= NN_XMSG_MAX_MESSAGE_IOVECS);

  TRACE (("xpack_addmsg %p %p(", (void *) xp, (void *) m));
  switch (m->kind)
  {
    case NN_XMSG_KIND_CONTROL:
      TRACE (("control"));
      break;
    case NN_XMSG_KIND_DATA:
    case NN_XMSG_KIND_DATA_REXMIT:
      TRACE (("%s(%x:%x:%x:%x:#%lld/%u)",
              (m->kind == NN_XMSG_KIND_DATA) ? "data" : "rexmit",
              PGUID (m->kindspecific.data.wrguid),
              m->kindspecific.data.wrseq,
              m->kindspecific.data.wrfragid + 1));
      break;
  }
  TRACE (("): niov %d sz %d", niov, sz));

  /* If a fresh xp has been provided, add an RTPS header */

  if (niov == 0)
  {
    copy_addressing_info (xp, m);
    xp->hdr.guid_prefix = m->data->src.guid_prefix;
    xp->iov[niov].iov_base = (void*) &xp->hdr;
    xp->iov[niov].iov_len = sizeof (xp->hdr);
    sz = (unsigned) xp->iov[niov].iov_len;
    niov++;

    /* Add MSG_LEN sub message for stream based transports */

    if (gv.m_factory->m_stream)
    {
      xp->iov[niov].iov_base = (void*) &xp->msg_len;
      xp->iov[niov].iov_len = sizeof (xp->msg_len);
      sz += sizeof (xp->msg_len);
      niov++;
    }

    xp->last_src = &xp->hdr.guid_prefix;
    xp->last_dst = NULL;
  }
  else
  {
    xpo_niov = xp->niov;
    xpo_sz = xp->msg_len.length;
    if (!guid_prefix_eq (xp->last_src, &m->data->src.guid_prefix))
    {
      /* If m's source participant differs from that of the source
         currently set in the packed message, add an InfoSRC note. */
      xp->iov[niov].iov_base = (void*) &m->data->src;
      xp->iov[niov].iov_len = sizeof (m->data->src);
      sz += sizeof (m->data->src);
      xp->last_src = &m->data->src.guid_prefix;
      niov++;
    }
  }

  /* We try to merge iovecs by checking iov[niov-1].  We used to check
     addressing_info_eq_onesidederr here (again), but can't because it
     relies on an imprecise check that may (timing-dependent) return
     false incorrectly */
  assert (niov >= 1);

  /* Adding this message may shorten the time this xpack may linger */
  if (m->maxdelay < xp->maxdelay)
    xp->maxdelay = m->maxdelay;

  /* If m's dst differs from that of the dst currently set in the
     packed message, add an InfoDST note. Note that neither has to
     have a dst set. */
  if (xp->last_dst == NULL)
    dst = (m->dstmode == NN_XMSG_DST_ONE) ? &m->data->dst : NULL;
  else if (m->dstmode != NN_XMSG_DST_ONE)
    dst = &static_zero_dst;
  else
    dst = guid_prefix_eq (&xp->last_dst->guid_prefix, &m->data->dst.guid_prefix) ? NULL : &m->data->dst;

  if (dst)
  {
    /* Try to merge iovecs, a few large ones should be more efficient
       than many small ones */
    if ((char *) xp->iov[niov-1].iov_base + xp->iov[niov-1].iov_len == (char *) dst)
    {
      xp->iov[niov-1].iov_len += sizeof (*dst);
    }
    else
    {
      xp->iov[niov].iov_base = (void*) dst;
      xp->iov[niov].iov_len = sizeof (*dst);
      niov++;
    }
    sz += sizeof (*dst);
    xp->last_dst = dst;
  }

  /* Append submessage; can possibly be merged with preceding iovec */
  if ((char *) xp->iov[niov-1].iov_base + xp->iov[niov-1].iov_len == (char *) m->data->payload)
    xp->iov[niov-1].iov_len += m->sz;
  else
  {
    xp->iov[niov].iov_base = m->data->payload;
    xp->iov[niov].iov_len = m->sz;
    niov++;
  }
  sz += m->sz;

  /* Append ref'd payload if given; whoever constructed the message
     should've taken care of proper alignment for the payload.  The
     ref'd payload is always at some weird address, so no chance of
     merging iovecs here. */
  if (m->refd_payload)
  {
    xp->iov[niov] = m->refd_payload_iov;
    sz += m->refd_payload_iov.iov_len;
    niov++;
  }

  /* Shouldn't've overrun iov, and shouldn't've tried to add a
     submessage that is too large for a message ... but the latter
     isn't worth checking. */
  assert (niov <= NN_XMSG_MAX_MESSAGE_IOVECS);

  /* Set total message length in MSG_LEN sub message */
  xp->msg_len.length = sz;
  xp->niov = niov;

  if (xpo_niov > 0 && sz > config.max_msg_size)
  {
    TRACE ((" => now niov %d sz %u > max_msg_size %u, nn_xpack_send niov %d sz %u now\n", niov, sz, config.max_msg_size, xpo_niov, xpo_sz));
    xp->msg_len.length = xpo_sz;
    xp->niov = xpo_niov;
    nn_xpack_send (xp);
    result = nn_xpack_addmsg (xp, m); /* Retry on emptied xp */
  }
  else
  {
    nn_xmsg_chain_add (&xp->included_msgs, m);
    TRACE ((" => now niov %d sz %u\n", niov, sz));
  }

  return result;
}

os_int64 nn_xpack_maxdelay (const struct nn_xpack *xp)
{
  return xp->maxdelay;
}

unsigned nn_xpack_packetid (const struct nn_xpack *xp)
{
  return xp->packetid;
}

/* SHA1 not available (unoffical build.) */
