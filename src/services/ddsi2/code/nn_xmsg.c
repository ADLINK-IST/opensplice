#include <ctype.h>
#include <stddef.h>

#include "os_heap.h"
#include "os_if.h"

#include "nn_avl.h"
#include "nn_osplser.h"
#include "nn_protocol.h"
#include "nn_xqos.h"
#include "nn_bswap.h"
#include "nn_rtps.h"
#include "nn_addrset.h"
#include "nn_error.h"
#include "nn_misc.h"
#include "nn_log.h"
#include "nn_unused.h"
#include "nn_xmsg.h"
#include "nn_align.h"
#include "nn_config.h"

#include "nn_rtps_private.h" /* cos I now need to look inside proxy_reader */

#include "sysdeps.h"

#define NN_XMSG_MAX_ALIGN 8
#define NN_XMSG_CHUNK_SIZE 128

struct nn_xmsgpool {
  os_mutex lock;
  int nalloced;
  int nfree;
  struct nn_xmsg_chain_elem *freelist;
};

struct nn_xmsg_data {
  InfoSRC_t src;
  InfoDST_t dst;
  char payload[1]; /* of size maxsz */
};

struct nn_xmsg_chain_elem {
  struct nn_xmsg_chain_elem *next;
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
  struct serdata *refd_payload;
  struct iovec refd_payload_iov;
  os_socket sock;

  enum nn_xmsg_dstmode dstmode;
  union {
    nn_locator_udpv4_t loc;     /* send just to this locator */
    struct addrset *as;         /* send to all addresses in set */
  } dstaddr;

  struct nn_xmsg_chain_elem link;
  struct nn_xmsg_data *data;
};

/* Worst-case: change of SRC [+1] but no DST, submessage [+1], ref'd
   payload [+1].  So 128 iovecs => at least ~40 submessages, so for
   very small ones still >1kB. */
#define NN_XMSG_MAX_SUBMESSAGE_IOVECS 3
#define NN_XMSG_MAX_MESSAGE_SIZE 4096

#ifdef IOV_MAX
#if IOV_MAX < 128
#define NN_XMSG_MAX_MESSAGE_IOVECS IOV_MAX
#endif
#endif /* defined IOV_MAX */
#ifndef NN_XMSG_MAX_MESSAGE_IOVECS
#define NN_XMSG_MAX_MESSAGE_IOVECS 128
#endif

struct nn_xmsg_chain {
  struct nn_xmsg_chain_elem *first, *last;
};

struct nn_xpack {
  Header_t hdr;
#ifndef NDEBUG
  os_threadId owner_tid;
#endif
  nn_guid_prefix_t *last_src;
  InfoDST_t *last_dst;
  int sz;
  os_socket sock;

  enum nn_xmsg_dstmode dstmode;
  union {
    nn_locator_udpv4_t loc;     /* send just to this locator */
    struct addrset *as;         /* send to all addresses in set */
  } dstaddr;

  struct nn_xmsg_chain included_msgs;

  int niov;
  struct iovec iov[NN_XMSG_MAX_MESSAGE_IOVECS];
};

/* XMSGPOOL ------------------------------------------------------------

   Great expectations, but so far still wanting. */

static void nn_xmsg_realfree (struct nn_xmsg *m);

struct nn_xmsgpool *nn_xmsgpool_new (void)
{
  os_mutexAttr attr;
  struct nn_xmsgpool *pool;
  pool = os_malloc (sizeof (*pool));
  os_mutexAttrInit (&attr);
  attr.scopeAttr = OS_SCOPE_PRIVATE;
  os_mutexInit (&pool->lock, &attr);
  pool->freelist = NULL;
  pool->nalloced = 0;
  pool->nfree = 0;
  return pool;
}

void nn_xmsgpool_free (struct nn_xmsgpool *pool)
{
  while (pool->freelist)
  {
    struct nn_xmsg *m = (struct nn_xmsg *) ((char *) pool->freelist - offsetof (struct nn_xmsg, link));
    pool->freelist = pool->freelist->next;
    nn_xmsg_realfree (m);
  }
  os_mutexDestroy (&pool->lock);
  nn_log (LC_TRACE, "xmsgpool_free(%p) nalloced %d nfree %d\n", pool, pool->nalloced, pool->nfree);
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

static void nn_xmsg_reinit (struct nn_xmsg *m)
{
  m->sz = 0;
  m->refd_payload = NULL;
  m->dstmode = NN_XMSG_DST_UNSET;
}

static struct nn_xmsg *nn_xmsg_allocnew (struct nn_xmsgpool *pool, unsigned expected_size)
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
  nn_xmsg_reinit (m);
  return m;
}

struct nn_xmsg *nn_xmsg_new (struct nn_xmsgpool *pool, os_socket sock, const nn_guid_prefix_t *src_guid_prefix, unsigned expected_size)
{
  struct nn_xmsg *m;
  os_mutexLock (&pool->lock);
  if (pool->freelist)
  {
    m = (struct nn_xmsg *) ((char *) pool->freelist - offsetof (struct nn_xmsg, link));
    pool->freelist = pool->freelist->next;
    pool->nfree--;
    os_mutexUnlock (&pool->lock);
    nn_xmsg_reinit (m);
  }
  else
  {
    pool->nalloced++;
    os_mutexUnlock (&pool->lock);
    if ((m = nn_xmsg_allocnew (pool, expected_size)) == NULL)
      return NULL;
  }
  m->sock = sock;
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
    unref_addrset (m->dstaddr.as);
  os_mutexLock (&pool->lock);
#ifndef NDEBUG
  {
    struct nn_xmsg_chain_elem *b;
    for (b = pool->freelist; b && b != &m->link; b = b->next)
      ;
    assert (b == NULL);
  }
#endif
  m->link.next = pool->freelist;
  pool->freelist = &m->link;
  pool->nfree++;
  os_mutexUnlock (&pool->lock);
}

/************************************************/

void *nn_xmsg_payload (unsigned *sz, struct nn_xmsg *m)
{
  *sz = m->sz;
  return m->data->payload;
}

void nn_xmsg_submsg_init (struct nn_xmsg *msg, struct nn_xmsg_marker marker, SubmessageKind_t smkind)
{
  SubmessageHeader_t *hdr = (SubmessageHeader_t *) (msg->data->payload + marker.offset);
  hdr->submessageId = smkind;
  hdr->flags = PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0;
  hdr->octetsToNextHeader = 0;
}

void nn_xmsg_submsg_setnext (struct nn_xmsg *msg, struct nn_xmsg_marker marker)
{
  SubmessageHeader_t *hdr = (SubmessageHeader_t *) (msg->data->payload + marker.offset);
  unsigned plsize = msg->refd_payload ? msg->refd_payload_iov.iov_len : 0;
  assert ((msg->sz % 4) == 0);
  assert ((plsize % 4) == 0);
  assert ((unsigned) (msg->data->payload + msg->sz + plsize - (char *) hdr) >= RTPS_SUBMESSAGE_HEADER_SIZE);
  hdr->octetsToNextHeader =
    (msg->data->payload + msg->sz + plsize - (char *) hdr) - RTPS_SUBMESSAGE_HEADER_SIZE;
}

void *nn_xmsg_append_aligned (struct nn_xmsg *m, struct nn_xmsg_marker *marker, unsigned sz, unsigned a)
{
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
    if (ndata == NULL)
      return NULL;
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
  if ((ts = (InfoTimestamp_t *) nn_xmsg_append_aligned (m, &sm, sizeof (InfoTimestamp_t), 4)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  nn_xmsg_submsg_init (m, sm, SMID_INFO_TS);
  ts->time = nn_to_ddsi_time (t);
  nn_xmsg_submsg_setnext (m, sm);
  return 0;
}

int nn_xmsg_serdata (struct nn_xmsg *m, serdata_t serdata, unsigned off, unsigned len)
{
  assert (m->refd_payload == NULL);
  m->refd_payload = serdata_ref (serdata);
  m->refd_payload_iov.iov_base = (char *) &m->refd_payload->hdr + off;
  m->refd_payload_iov.iov_len = ALIGN4 (len);
  return 0;
}

int nn_xmsg_setdst1 (struct nn_xmsg *m, const nn_guid_prefix_t *gp, const nn_locator_udpv4_t *addr)
{
  assert (m->dstmode == NN_XMSG_DST_UNSET);
  m->dstmode = NN_XMSG_DST_ONE;
  m->dstaddr.loc = *addr;
  m->data->dst.guid_prefix = nn_hton_guid_prefix (*gp);
  return 0;
}

int nn_xmsg_setdstPRD (struct nn_xmsg *m, const struct proxy_reader *prd)
{
  nn_locator_udpv4_t loc;
  if (addrset_any_uc (prd->c.as, &loc) || addrset_any_mc (prd->c.as, &loc))
    nn_xmsg_setdst1 (m, &prd->c.guid.prefix, &loc);
  return 0;
}

int nn_xmsg_setdstN (struct nn_xmsg *m, struct addrset *as)
{
  assert (m->dstmode == NN_XMSG_DST_UNSET || m->dstmode == NN_XMSG_DST_ONE);
  m->dstmode = NN_XMSG_DST_ALL;
  m->dstaddr.as = ref_addrset (as);
  return 0;
}

void *nn_xmsg_addpar (struct nn_xmsg *m, int pid, int len)
{
  const int len4 = (len + 3) & -4; /* must alloc a multiple of 4 */
  nn_parameter_t *phdr;
  char *p;
  if ((phdr = nn_xmsg_append_aligned (m, NULL, sizeof (nn_parameter_t) + len4, 4)) == NULL)
    return NULL;
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
  int len = strlen (str) + 1;
  if ((p = nn_xmsg_addpar (m, pid, 4 + len)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  p->length = len;
  memcpy (p->contents, str, len);
  return 0;
}

int nn_xmsg_addpar_octetseq (struct nn_xmsg *m, int pid, const nn_octetseq_t *oseq)
{
  char *p;
  if ((p = nn_xmsg_addpar (m, pid, 4 + oseq->length)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
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
    int len1 = strlen (sseq->strs[i]) + 1;
    len += 4 + ALIGN4 (len1);
  }

  if ((tmp = nn_xmsg_addpar (m, pid, 4 + len)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;

  *((int *) tmp) = sseq->n;
  tmp += sizeof (int);
  for (i = 0; i < sseq->n; i++)
  {
    struct cdrstring *p = (struct cdrstring *) tmp;
    int len1 = strlen (sseq->strs[i]) + 1;
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
    return NN_ERR_OUT_OF_MEMORY;
  serdata_keyhash (serdata, p);
  return 0;
}

int nn_xmsg_addpar_guid (struct nn_xmsg *m, int pid, const nn_guid_t *guid)
{
  unsigned *pu;
  int i;
  if ((pu = nn_xmsg_addpar (m, pid, 16)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  for (i = 0; i < 3; i++)
    pu[i] = toBE4u (guid->prefix.u[i]);
  pu[i] = toBE4u (guid->entityid.u);
  return 0;
}

int nn_xmsg_addpar_reliability (struct nn_xmsg *m, int pid, const struct nn_reliability_qospolicy *rq)
{
  struct nn_external_reliability_qospolicy *p;
  if ((p = nn_xmsg_addpar (m, pid, sizeof (*p))) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
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
    return NN_ERR_OUT_OF_MEMORY;
  *p = x;
  return 0;
}

int nn_xmsg_addpar_BE4u (struct nn_xmsg *m, int pid, unsigned x)
{
  unsigned *p;
  if ((p = nn_xmsg_addpar (m, pid, 4)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  *p = toBE4u (x);
  return 0;
}

int nn_xmsg_addpar_statusinfo (struct nn_xmsg *m, unsigned statusinfo)
{
  return nn_xmsg_addpar_BE4u (m, PID_STATUSINFO, statusinfo);
}

int nn_xmsg_addpar_sentinel (struct nn_xmsg *m)
{
  if (nn_xmsg_addpar (m, PID_SENTINEL, 0) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  return 0;
}

int nn_xmsg_add_pl_cdr_header (struct nn_xmsg *m)
{
  struct CDRHeader *plhdr;
  if ((plhdr = nn_xmsg_append_aligned (m, NULL, sizeof (struct CDRHeader), 4)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  plhdr->identifier = PLATFORM_IS_LITTLE_ENDIAN ? PL_CDR_LE : PL_CDR_BE;
  plhdr->options = 0;
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
  while (chain->first)
  {
    struct nn_xmsg_chain_elem *ce = chain->first;
    struct nn_xmsg *m = (struct nn_xmsg *) ((char *) ce - offsetof (struct nn_xmsg, link));
    chain->first = ce->next;
    nn_xmsg_free (m);
  }
}

static void nn_xmsg_chain_add (struct nn_xmsg_chain *chain, struct nn_xmsg *m)
{
  m->link.next = NULL;
  if (chain->first)
    chain->last->next = &m->link;
  else
    chain->first = &m->link;
  chain->last = &m->link;
}

/* XPACK ---------------------------------------------------------------

   Queued messages are packed into xpacks (all by-ref, using iovecs).
   The xpack is sent to the union of all address sets provided in the
   message added to the xpack.  */

static void nn_xpack_reinit (struct nn_xpack *xp)
{
  xp->dstmode = NN_XMSG_DST_UNSET;
  xp->niov = 0;
  xp->sz = 0;
  xp->included_msgs.first = NULL;
}

struct nn_xpack *nn_xpack_new (void)
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

  nn_xpack_reinit (xp);
  return xp;
}

void nn_xpack_free (struct nn_xpack *xp)
{
  assert (os_threadEqual (os_threadIdSelf (), xp->owner_tid));
  assert (xp->niov == 0);
  assert (xp->included_msgs.first == NULL);
  os_free (xp);
}

struct xpack_send_arg {
  logbuf_t lb;
  os_socket sock;
  struct msghdr mhdr;
};

static size_t nn_xpack_send1 (const nn_locator_udpv4_t *addr, void *varg)
{
  struct xpack_send_arg *arg = varg;
  struct sockaddr_in s;
  ssize_t nbytes;

#if HAVE_VALGRIND
  memset (&s, 0, sizeof (s));
#endif
  s.sin_family = AF_INET;
  s.sin_addr.s_addr = addr->address;
  s.sin_port = htons (addr->port);

  if (config.enabled_logcats & LC_TRACE)
    nn_logb (arg->lb, LC_TRACE, " %s:%d", inet_ntoa (s.sin_addr), addr->port);
  if (config.xmit_lossiness > 0)
  {
    /* We drop APPROXIMATELY a fraction of xmit_lossiness * 10**(-3)
       of all packets to be sent */
    if (random () < config.xmit_lossiness * (RAND_MAX / 1000))
    {
      nn_logb (arg->lb, LC_TRACE, "(dropped)");
      return 0;
    }
  }

  arg->mhdr.msg_name = &s;
  nbytes = sendmsg (arg->sock, &arg->mhdr, 0);
  if (nbytes <= 0)
  {
    int err = os_sockError ();
    if (err != os_sockECONNRESET)
      nn_log (LC_ERROR, "sendmsg sock %d: %d errno %d\n", (int) arg->sock, nbytes, err);
    nbytes = 0;
  }

  return nbytes;
}

size_t nn_xpack_send (struct nn_xpack *xp)
{
  struct xpack_send_arg arg;
  size_t nbytes = 0; /* either this, or a default in the switch ... */
  LOGBUF_DECLNEW (lb);

  assert (os_threadEqual (os_threadIdSelf (), xp->owner_tid));
  assert (xp->niov <= NN_XMSG_MAX_MESSAGE_IOVECS);

  if (xp->niov == 0)
    return 0;

  assert (xp->dstmode != NN_XMSG_DST_UNSET);

  if (config.enabled_logcats & LC_TRACE)
  {
    int i;
    nn_logb (lb, LC_TRACE, "nn_xpack_send %u:", xp->sz);
    for (i = 0; i < (int) xp->niov; i++)
      nn_logb (lb, LC_TRACE, " %p:%u", (void *) xp->iov[i].iov_base, xp->iov[i].iov_len);
  }

  /* arg.mhdr.msg_name set by xpack_send1 */
  arg.mhdr.msg_namelen = sizeof (struct sockaddr_in);
  arg.mhdr.msg_iov = xp->iov;
  arg.mhdr.msg_iovlen = xp->niov;
#if SYSDEPS_MSGHDR_ACCRIGHTS
  arg.mhdr.msg_accrights = NULL;
  arg.mhdr.msg_accrightslen = 0;
#else
  arg.mhdr.msg_control = NULL;
  arg.mhdr.msg_controllen = 0;
#endif
#if SYSDEPS_MSGHDR_FLAGS
  /* flags is meaningless on send, but valgrind might complain about
     uninitialised bytes if we don't set it */
  arg.mhdr.msg_flags = 0;
#endif

  arg.lb = lb;
  arg.sock = xp->sock;

  nn_logb (lb, LC_TRACE, " [");
  switch (xp->dstmode)
  {
    case NN_XMSG_DST_UNSET:
      assert (0);
    case NN_XMSG_DST_ONE:
      nbytes = nn_xpack_send1 (&xp->dstaddr.loc, &arg);
      break;
    case NN_XMSG_DST_ALL:
      nbytes = addrset_forall_addresses (xp->dstaddr.as, nn_xpack_send1, &arg);
      unref_addrset (xp->dstaddr.as);
      break;
  }
  nn_logb (lb, LC_TRACE, " ]\n");
  nn_xmsg_chain_release (&xp->included_msgs);
  nn_xpack_reinit (xp);
  nn_logb_flush (lb);
  LOGBUF_FREE (lb);
  return nbytes;
}

static void copy_addressing_info (struct nn_xpack *xp, const struct nn_xmsg *m)
{
  xp->sock = m->sock;
  xp->dstmode = m->dstmode;
  switch (m->dstmode)
  {
    case NN_XMSG_DST_UNSET:
      assert (0);
    case NN_XMSG_DST_ONE:
      xp->dstaddr.loc = m->dstaddr.loc;
      break;
    case NN_XMSG_DST_ALL:
      xp->dstaddr.as = ref_addrset (m->dstaddr.as);
      break;
  }
}

static int addressing_info_eq (const struct nn_xpack *xp, const struct nn_xmsg *m)
{
  if (xp->sock != m->sock)
    return 0;
  if (xp->dstmode != m->dstmode)
    return 0;
  switch (xp->dstmode)
  {
    case NN_XMSG_DST_UNSET:
      assert (0);
    case NN_XMSG_DST_ONE:
      return (memcmp (&xp->dstaddr.loc, &m->dstaddr.loc, sizeof (xp->dstaddr.loc)) == 0);
    case NN_XMSG_DST_ALL:
      return addrset_eq_maybeimprecise (xp->dstaddr.as, m->dstaddr.as);
  }
  assert (0);
  return 0;
}

static int nn_xpack_mayaddmsg (const struct nn_xpack *xp, const struct nn_xmsg *m)
{
  if (xp->niov == 0)
    return 1;
  assert (xp->included_msgs.first != NULL);
  if (xp->niov + NN_XMSG_MAX_SUBMESSAGE_IOVECS > NN_XMSG_MAX_MESSAGE_IOVECS)
    return 0;
  if (xp->sz + m->sz > NN_XMSG_MAX_MESSAGE_SIZE)
    return 0;
  return addressing_info_eq (xp, m);
}

static int guid_prefix_eq (const nn_guid_prefix_t *a, const nn_guid_prefix_t *b)
{
  return a->u[0] == b->u[0] && a->u[1] == b->u[1] && a->u[2] == b->u[2];
}

int nn_xpack_addmsg (struct nn_xpack *xp, struct nn_xmsg *m)
{
  static InfoDST_t static_zero_dst;
  InfoDST_t *dst;
  int niov, sz;
  int result = 0;

  assert (os_threadEqual (os_threadIdSelf (), xp->owner_tid));
  assert (m->sz > 0);
  assert (m->dstmode != NN_XMSG_DST_UNSET);

  /* Submessage offset must be a multiple of 4 to meet alignment
     requirement (DDSI 2.1, 9.4.1).  If we keep everything 4-byte
     aligned all the time, we don't need to check for padding here. */
  assert ((xp->sz % 4) == 0);
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
  sz = xp->sz;

  /* We try to merge iovecs, but we can never merge across messages
     because of all the headers. So we can speculatively start adding
     the submessage to the pack, and if we can't transmit and restart.
     But do make sure we can't run out of iovecs. */
  assert (niov + NN_XMSG_MAX_SUBMESSAGE_IOVECS <= NN_XMSG_MAX_MESSAGE_IOVECS);

  nn_log (LC_TRACE, "xpack_addmsg %p %p: niov %d sz %d", (void *) xp, (void *) m, niov, sz);

  /* If a fresh xp has been provided, add an RTPS header */
  if (niov == 0)
  {
    copy_addressing_info (xp, m);
    xp->hdr.guid_prefix = m->data->src.guid_prefix;
    xp->iov[niov].iov_base = &xp->hdr;
    xp->iov[niov].iov_len = sizeof (xp->hdr);
    sz = xp->iov[niov].iov_len;
    niov++;
    xp->last_src = &xp->hdr.guid_prefix;
    xp->last_dst = NULL;
  }
  else if (!guid_prefix_eq (xp->last_src, &m->data->src.guid_prefix))
  {
    /* If m's source participant differs from that of the source
       currently set in the packed message, add an InfoSRC note. */
    xp->iov[niov].iov_base = &m->data->src;
    xp->iov[niov].iov_len = sizeof (m->data->src);
    sz += sizeof (m->data->src);
    xp->last_src = &m->data->src.guid_prefix;
    niov++;
  }

  /* We try to merge iovecs by checking iov[niov-1] */
  assert (addressing_info_eq (xp, m));
  assert (niov >= 1);

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
      xp->iov[niov-1].iov_len += sizeof (*dst);
    else
    {
      xp->iov[niov].iov_base = dst;
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
  xp->sz = sz;
  xp->niov = niov;
  nn_xmsg_chain_add (&xp->included_msgs, m);

  nn_log (LC_TRACE, " => now niov %d sz %d\n", niov, sz);
  return result;
}
