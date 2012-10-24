#ifndef NN_RADMIN_H
#define NN_RADMIN_H

struct nn_rbufpool;
struct nn_rbuf;
struct nn_rmsg;
struct nn_rdata;
struct nn_rsample;
struct nn_rsample_chain;
struct nn_rsample_info;
struct nn_defrag;
struct nn_reorder;
struct nn_dqueue;

struct receiver_state;
struct proxy_writer;

typedef int (*nn_dqueue_handler_t) (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, void *qarg);

struct nn_rmsg_chunk {
  struct nn_rbuf *rbuf;
  struct nn_rmsg_chunk *next;

  /* Size is 0 after initial allocation, must be set with
     nn_rmsg_setsize after receiving a packet from the kernel and
     before processing it.  */
  os_uint32 size;
  union {
    /* payload array stretched to whatever it really is */
    char payload[1];

    /* to ensure reasonable alignment of payload[] */
    os_int64 l;
    double d;
    void *p;
  } u;
};

struct nn_rmsg {
  /* Reference count: all references to rdatas of this message are
     counted. The rdatas themselves do not have a reference count.

     The refcount is biased by RMSG_REFCOUNT_UNCOMMITED_BIAS while
     still being inserted to allow verifying it is still uncommitted
     when allocating memory, increasing refcounts, &c.

     Each rdata adds RMS_REFCOUNT_RDATA_BIAS when it leaves
     defragmentation until it has been rejected by reordering or has
     been scheduled for delivery.  This allows delaying the
     decrementing of refcounts until after a sample has been added to
     all radmins even though be delivery of it may take place in
     concurrently. */
  os_uint32 refcount;

  /* Worst-case memory requirement is gigantic (64kB UDP packet, only
     1-byte final fragments, each of one a new interval, or maybe 1
     byte messages, destined for many readers and in each case
     introducing a new interval, with receiver state changes in
     between, &c.), so we can either:

     - allocate a _lot_ and cover the worst case

     - allocate enough for all "reasonable" cases, discarding data when that limit is hit

     - dynamically add chunks of memory, and even entire receive buffers.

     The latter seems the best approach, especially because it also
     covers the second one.  We treat the other chunks specially,
     which is not strictly required but also not entirely
     unreasonable, considering that the first chunk has the refcount &
     the real packet. */
  struct nn_rmsg_chunk *lastchunk;

  struct nn_rmsg_chunk chunk;
};
#define NN_RMSG_PAYLOAD(m) ((m)->chunk.u.payload)
#define NN_RMSG_PAYLOADOFF(m, o) (NN_RMSG_PAYLOAD (m) + (o))

struct nn_rsample_info {
  os_int64 seq;
  struct receiver_state *rst;
  os_uint32 size;
  os_uint32 fragsize;
  unsigned statusinfo: 2;       /* just the two defined bits from the status info */
  unsigned pt_wr_info_zoff: 16; /* PrismTech writer info offset */
  unsigned bswap: 1;            /* so we can extract well formatted writer info quicker */
  unsigned complex_qos: 1;      /* includes QoS other than keyhash, 2-bit statusinfo, PT writer info */
};

struct nn_rdata {
  struct nn_rmsg *rmsg;         /* received (and refcounted) in rmsg */
  struct nn_rdata *nextfrag;    /* fragment chain */
  os_uint32 min, maxp1;         /* fragment as byte offsets */
  os_ushort submsg_zoff;        /* offset to submessage from packet start, or 0 */
  os_ushort payload_zoff;       /* offset to payload from packet start */
#ifndef NDEBUG
  os_uint32 refcount_bias_added;
#endif
};

/* All relative offsets in packets that we care about (submessage
   header, payload, writer info) are at multiples of 4 bytes and
   within 64kB, so technically we can make do with 14 bits instead of
   16, in case we run out of space.

   If we _really_ need to squeeze out every last bit, only the submsg
   offset really requires 14 bits, the for the others we could use an
   offset relative to the submessage header so that it is limited by
   the maximum size of the inline QoS ...  Defining the macros now, so
   we have the option to do wild things. */
#ifndef NDEBUG
#define NN_ZOFF_TO_OFF(zoff) (zoff)
#define NN_OFF_TO_ZOFF(off) (assert ((unsigned) (off) < 65536 && ((off) % 4) == 0), (off))
#else
#define NN_ZOFF_TO_OFF(zoff) (zoff)
#define NN_OFF_TO_ZOFF(off) (off)
#endif
#define NN_SAMPLEINFO_HAS_WRINFO(rsi) ((rsi)->pt_wr_info_zoff != NN_OFF_TO_ZOFF (0))
#define NN_SAMPLEINFO_WRINFO_OFF(rsi) NN_ZOFF_TO_OFF ((rsi)->pt_wr_info_zoff)
#define NN_RDATA_PAYLOAD_OFF(rdata) NN_ZOFF_TO_OFF ((rdata)->payload_zoff)
#define NN_RDATA_SUBMSG_OFF(rdata) NN_ZOFF_TO_OFF ((rdata)->submsg_zoff)

struct nn_rsample_chain_elem {
  /* FIXME: evidently smaller than a defrag_iv, but maybe better to
     merge it with defrag_iv in a union anyway. */
  struct nn_rdata *fragchain;
  struct nn_rsample_chain_elem *next;
  /* Gaps have sampleinfo = NULL, but nonetheless a fragchain with 1
     rdata with min=maxp1 (length 0) and valid rmsg pointer.  And (see
     DQUEUE) its lsb gets abused so we can queue "bubbles" in addition
     to data). */
  struct nn_rsample_info *sampleinfo;
};

struct nn_rsample_chain {
  struct nn_rsample_chain_elem *first;
  struct nn_rsample_chain_elem *last;
};

enum nn_reorder_mode {
  NN_REORDER_MODE_NORMAL,
  NN_REORDER_MODE_MONOTONICALLY_INCREASING,
  NN_REORDER_MODE_ALWAYS_DELIVER
};

enum nn_defrag_drop_mode {
  NN_DEFRAG_DROP_OLDEST,        /* (believed to be) best for unreliable */
  NN_DEFRAG_DROP_LATEST         /* (...) best for reliable  */
};

enum nn_reorder_result {
  NN_REORDER_DELIVER,           /* returning a sample chain for delivery */
  NN_REORDER_TOO_OLD,           /* discarded because it was too old */
  NN_REORDER_REJECT,            /* caller may reuse memory ("real" reject for data, "fake" for gap) */
  NN_REORDER_ACCEPT             /* accepted/stored */
};

typedef void (*nn_dqueue_callback_t) (void *arg);

struct nn_rbufpool *nn_rbufpool_new (int rbuf_size, int max_rmsg_size);
void nn_rbufpool_free (struct nn_rbufpool *rbp);

struct nn_rmsg *nn_rmsg_new (struct nn_rbufpool *rbufpool);
void nn_rmsg_setsize (struct nn_rmsg *rmsg, os_uint32 size);
void nn_rmsg_commit (struct nn_rmsg *rmsg);
void nn_rmsg_free (struct nn_rmsg *rmsg);
void *nn_rmsg_alloc (struct nn_rmsg *rmsg, os_uint32 size);

struct nn_rdata *nn_rdata_new (struct nn_rmsg *rmsg, os_uint32 start, os_uint32 endp1, os_uint32 submsg_offset, os_uint32 payload_offset);
struct nn_rdata *nn_rdata_newgap (struct nn_rmsg *rmsg);
void nn_fragchain_adjust_refcount (struct nn_rdata *frag, int adjust);
void nn_fragchain_unref (struct nn_rdata *frag);

struct nn_defrag *nn_defrag_new (enum nn_defrag_drop_mode drop_mode, os_uint32 max_samples);
void nn_defrag_free (struct nn_defrag *defrag);
struct nn_rsample *nn_defrag_rsample (struct nn_defrag *defrag, struct nn_rdata *rdata, const struct nn_rsample_info *sampleinfo);
void nn_defrag_notegap (struct nn_defrag *defrag, os_int64 min, os_int64 maxp1);
int nn_defrag_nackmap (struct nn_defrag *defrag, os_int64 seq, os_uint32 maxfragnum, struct nn_fragment_number_set *map, os_uint32 maxsz);

struct nn_reorder *nn_reorder_new (enum nn_reorder_mode mode, os_uint32 max_samples);
void nn_reorder_free (struct nn_reorder *r);
struct nn_rsample *nn_reorder_rsample_dup (struct nn_rmsg *rmsg, struct nn_rsample *rsampleiv);
struct nn_rdata *nn_rsample_fragchain (struct nn_rsample *rsample);
enum nn_reorder_result nn_reorder_rsample (struct nn_rsample_chain *sc, struct nn_reorder *reorder, struct nn_rsample *rsampleiv, int *refcount_adjust);
enum nn_reorder_result nn_reorder_gap (struct nn_rsample_chain *sc, struct nn_reorder *reorder, struct nn_rdata *rdata, os_int64 min, os_int64 maxp1, int *refcount_adjust);
int nn_reorder_nackmap (struct nn_reorder *reorder, os_int64 maxseq, struct nn_sequence_number_set *map, os_uint32 maxsz);
os_int64 nn_reorder_next_seq (const struct nn_reorder *reorder);

struct nn_dqueue *nn_dqueue_new (const char *name, nn_dqueue_handler_t handler, void *arg);
void nn_dqueue_free (struct nn_dqueue *q);
void nn_dqueue_enqueue (struct nn_dqueue *q, struct nn_rsample_chain *sc);
void nn_dqueue_enqueue_callback (struct nn_dqueue *q, nn_dqueue_callback_t cb, void *arg);

#endif /* NN_RADMIN_H */
