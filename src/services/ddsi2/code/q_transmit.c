#include <assert.h>
#include <math.h>

#include "os_defs.h"
#include "os_stdlib.h"
#include "os_mutex.h"

#include "v_state.h"

#include "ut_avl.h"
#include "q_whc.h"
#include "q_entity.h"
#include "q_addrset.h"
#include "q_xmsg.h"
#include "q_osplser.h"
#include "q_bswap.h"
#include "q_misc.h"
#include "q_thread.h"
#include "q_xevent.h"
#include "q_time.h"
#include "q_config.h"
#include "q_globals.h"
#include "q_error.h"
#include "q_transmit.h"
#include "q_entity.h"
#include "q_unused.h"
#include "q_hbcontrol.h"
#include "q_static_assert.h"

#include "sysdeps.h"

#if __STDC_VERSION__ >= 199901L
#define POS_INFINITY_DOUBLE INFINITY
#elif defined HUGE_VAL
/* Hope for the best -- the only consequence of getting this wrong is
   that T_NEVER may be printed as a fugly value instead of as +inf. */
#define POS_INFINITY_DOUBLE (HUGE_VAL + HUGE_VAL)
#else
#define POS_INFINITY_DOUBLE 1e1000
#endif

static const os_int64 const_hb_intv = 100 * T_MILLISECOND;
static const os_int64 const_hb_intv_min = 20 * T_MILLISECOND;
static const os_int64 const_hb_intv_max = 8000 * T_MILLISECOND;

static const struct wr_prd_match *root_rdmatch (const struct writer *wr)
{
  return ut_avlRoot (&wr_readers_treedef, &wr->readers);
}

static int have_reliable_subs (const struct writer *wr)
{
  if (ut_avlIsEmpty (&wr->readers) || root_rdmatch (wr)->min_seq == MAX_SEQ_NUMBER)
    return 0;
  else
    return 1;
}

void writer_hbcontrol_init (struct hbcontrol *hbc)
{
  hbc->t_of_last_write = 0;
  hbc->t_of_last_hb = 0;
  hbc->t_of_last_ackhb = 0;
  hbc->tsched = T_NEVER;
  hbc->hbs_since_last_write = 0;
  hbc->last_packetid = 0;
}

static void writer_hbcontrol_note_hb (struct writer *wr, os_int64 tnow, int ansreq)
{
  struct hbcontrol * const hbc = &wr->hbcontrol;

  if (ansreq)
    hbc->t_of_last_ackhb = tnow;
  hbc->t_of_last_hb = tnow;

  /* Count number of heartbeats since last write, used to lower the
     heartbeat rate.  Overflow doesn't matter, it'll just revert to a
     highish rate for a short while. */
  hbc->hbs_since_last_write++;
}

os_int64 writer_hbcontrol_intv (const struct writer *wr, UNUSED_ARG (os_int64 tnow))
{
  struct hbcontrol const * const hbc = &wr->hbcontrol;
  os_int64 ret = const_hb_intv;
  int n_unacked;

  if (hbc->hbs_since_last_write > 2)
  {
    unsigned cnt = hbc->hbs_since_last_write;
    while (cnt-- > 2 && 2 * ret < const_hb_intv_max)
      ret *= 2;
  }

  n_unacked = writer_number_of_unacked_samples (wr);
  if (n_unacked >= config.whc_highwater_mark / 2)
    ret /= 4;
  else if (n_unacked >= config.whc_highwater_mark / 4)
    ret /= 2;
  if (wr->throttling)
    ret /= 2;
  if (ret < const_hb_intv_min)
    ret = const_hb_intv_min;
  return ret;
}

void writer_hbcontrol_note_asyncwrite (struct writer *wr, os_int64 tnow)
{
  struct hbcontrol * const hbc = &wr->hbcontrol;
  os_int64 tnext;

  /* Reset number of heartbeats since last write: that means the
     heartbeat rate will go back up to the default */
  hbc->hbs_since_last_write = 0;

  /* We know this is new data, so we want a heartbeat event after one
     base interval */
  tnext = tnow + const_hb_intv;
  if (tnext < hbc->tsched)
  {
    /* Insertion of a message with WHC locked => must now have at
       least one unacked msg if there are reliable readers, so must
       have a heartbeat scheduled.  Do so now */
    hbc->tsched = tnext;
    resched_xevent_if_earlier (wr->heartbeat_xevent, tnext);
  }
}

int writer_hbcontrol_must_send (const struct writer *wr, os_int64 tnow)
{
  struct hbcontrol const * const hbc = &wr->hbcontrol;
  return (tnow >= hbc->t_of_last_hb + writer_hbcontrol_intv (wr, tnow));
}

struct nn_xmsg *writer_hbcontrol_create_heartbeat (struct writer *wr, os_int64 tnow, int hbansreq, int issync)
{
  struct nn_xmsg *msg;
  const nn_guid_t *prd_guid;

  ASSERT_MUTEX_HELD (&wr->e.lock);
  assert (wr->reliable);
  assert (hbansreq >= 0);

  if ((msg = nn_xmsg_new (gv.xmsgpool, &wr->e.guid.prefix, sizeof (InfoTS_t) + sizeof (Heartbeat_t), NN_XMSG_KIND_CONTROL)) == NULL)
    /* out of memory at worst slows down traffic */
    return NULL;

  if (ut_avlIsEmpty (&wr->readers) || wr->num_reliable_readers == 0)
  {
    /* Not really supposed to come here, at least not for the first
       case. Secondly, there really seems to be little use for
       optimising reliable writers with only best-effort readers. And
       in any case, it is always legal to multicast a heartbeat from a
       reliable writer. */
    prd_guid = NULL;
  }
  else if (wr->seq != root_rdmatch (wr)->max_seq)
  {
    /* If the writer is ahead of its readers, multicast. Couldn't care
       less about the pessimal cases such as multicasting when there
       is one reliable reader & multiple best-effort readers. See
       comment above. */
    prd_guid = NULL;
  }
  else
  {
    const int n_unacked = wr->num_reliable_readers - root_rdmatch (wr)->num_reliable_readers_where_seq_equals_max;
    assert (n_unacked >= 0);
    if (n_unacked == 0)
      prd_guid = NULL;
    else
    {
      assert (root_rdmatch (wr)->arbitrary_unacked_reader.entityid.u != NN_ENTITYID_UNKNOWN);
      if (n_unacked > 1)
        prd_guid = NULL;
      else
        prd_guid = &(root_rdmatch (wr)->arbitrary_unacked_reader);
    }
  }

  TRACE (("writer_hbcontrol: wr %x:%x:%x:%x ", PGUID (wr->e.guid)));
  if (prd_guid == NULL)
    TRACE (("multicasting "));
  else
    TRACE (("unicasting to prd %x:%x:%x:%x ", PGUID (*prd_guid)));
  TRACE (("(rel-prd %d seq-eq-max %d seq %lld maxseq %lld)\n",
          wr->num_reliable_readers,
          ut_avlIsEmpty (&wr->readers) ? -1 : root_rdmatch (wr)->num_reliable_readers_where_seq_equals_max,
          wr->seq,
          ut_avlIsEmpty (&wr->readers) ? (os_int64) -1 : root_rdmatch (wr)->max_seq));

  if (prd_guid == NULL)
  {
    nn_xmsg_setdstN (msg, wr->as);
    if (add_Heartbeat (msg, wr, hbansreq, to_entityid (NN_ENTITYID_UNKNOWN), tnow, issync) < 0)
    {
      nn_xmsg_free (msg);
      return NULL;
    }
  }
  else
  {
    struct proxy_reader *prd;
    if ((prd = ephash_lookup_proxy_reader_guid (prd_guid)) == NULL)
    {
      TRACE (("writer_hbcontrol: wr %x:%x:%x:%x unknown prd %x:%x:%x:%x\n", PGUID (wr->e.guid), PGUID (*prd_guid)));
      nn_xmsg_free (msg);
      return NULL;
    }
    /* set the destination explicitly to the unicast destination and the fourth
       param of add_Heartbeat needs to be the guid of the reader */
    nn_xmsg_setdstPRD (msg, prd);
    if (add_Heartbeat (msg, wr, hbansreq, prd_guid->entityid, tnow, issync) < 0)
    {
      nn_xmsg_free (msg);
      return NULL;
    }
  }

  writer_hbcontrol_note_hb (wr, tnow, hbansreq);
  return msg;
}

static int writer_hbcontrol_ack_required_generic (const struct writer *wr, os_int64 tlast, os_int64 tnow, int piggyback)
{
  struct hbcontrol const * const hbc = &wr->hbcontrol;
  const os_int64 hb_intv_ack = const_hb_intv;

  if (piggyback)
  {
    /* If it is likely that a heartbeat requiring an ack will go out
       shortly after the sample was written, it is better to piggyback
       it onto the sample.  The current idea is that a write shortly
       before the next heartbeat will go out should have one
       piggybacked onto it, so that the scheduled heartbeat can be
       suppressed. */
    if (tnow >= tlast + 4 * hb_intv_ack / 5)
      return 2;
  }
  else
  {
    /* For heartbeat events use a slightly longer interval */
    if (tnow >= tlast + hb_intv_ack)
      return 2;
  }

  /* For writers near throttling, add heartbeats often.  Soon they
     will be unable to make progress data is ack'd, for which we need
     a heartbeat. */
  if (wr->throttling)
    return 2;
  if (writer_number_of_unacked_samples (wr) >= config.whc_highwater_mark / 4)
  {
    const os_int64 intv = T_MILLISECOND; /* writer_hbcontrol_intv (wr, tnow) */
    if (tnow >= hbc->t_of_last_ackhb + intv)
      return 2;
    else
      return 1;
  }

  return 0;
}

int writer_hbcontrol_ack_required (const struct writer *wr, os_int64 tnow)
{
  struct hbcontrol const * const hbc = &wr->hbcontrol;
  return writer_hbcontrol_ack_required_generic (wr, hbc->t_of_last_write, tnow, 0);
}

struct nn_xmsg *writer_hbcontrol_piggyback (struct writer *wr, os_int64 tnow, unsigned packetid, int *hbansreq)
{
  struct hbcontrol * const hbc = &wr->hbcontrol;
  unsigned last_packetid;
  os_int64 tlast;
  struct nn_xmsg *msg;

  tlast = hbc->t_of_last_write;
  last_packetid = hbc->last_packetid;

  hbc->t_of_last_write = tnow;
  hbc->last_packetid = packetid;

  /* Update statistics, intervals, scheduling of heartbeat event,
     &c. -- there's no real difference between async and sync so we
     reuse the async version. */
  writer_hbcontrol_note_asyncwrite (wr, tnow);

  *hbansreq = writer_hbcontrol_ack_required_generic (wr, tlast, tnow, 1);
  if (*hbansreq >= 2) {
    /* So we force a heartbeat in - but we also rely on our caller to
       send the packet out */
    msg = writer_hbcontrol_create_heartbeat (wr, tnow, *hbansreq, 1);
  } else if (last_packetid != packetid) {
    /* If we crossed a packet boundary since the previous write,
       piggyback a heartbeat, with *hbansreq determining whether or
       not an ACK is needed.  We don't force the packet out either:
       this is just to ensure a regular flow of ACKs for cleaning up
       the WHC & for allowing readers to NACK missing samples. */
    msg = writer_hbcontrol_create_heartbeat (wr, tnow, *hbansreq, 1);
  } else {
    *hbansreq = 0;
    msg = NULL;
  }

  if (msg)
  {
    TRACE (("heartbeat(wr %x:%x:%x:%x%s) piggybacked, resched in %g s (min-ack %lld%s, avail-seq %lld, xmit %lld)\n",
            PGUID (wr->e.guid),
            *hbansreq ? "" : " final",
            (hbc->tsched == T_NEVER) ? POS_INFINITY_DOUBLE : (hbc->tsched - tnow) / 1e9,
            ut_avlIsEmpty (&wr->readers) ? (os_int64) -1 : root_rdmatch (wr)->min_seq,
            ut_avlIsEmpty (&wr->readers) || root_rdmatch (wr)->all_have_replied_to_hb ? "" : "!",
            whc_empty (wr->whc) ? (os_int64) -1 : whc_max_seq (wr->whc), wr->seq_xmit));
  }

  return msg;
}

int add_Heartbeat (struct nn_xmsg *msg, struct writer *wr, int hbansreq, nn_entityid_t dst, os_int64 tnow, int issync)
{
  struct nn_xmsg_marker sm_marker;
  Heartbeat_t * hb;
  os_int64 max = 0, min = 1;

  ASSERT_MUTEX_HELD (&wr->e.lock);

  assert (wr->reliable);
  assert (hbansreq >= 0);

  if (config.meas_hb_to_ack_latency)
  {
    /* If configured to measure heartbeat-to-ack latency, we must add
       a timestamp.  No big deal if it fails. */
    nn_xmsg_add_timestamp (msg, tnow);
  }

  hb = nn_xmsg_append (msg, &sm_marker, sizeof (Heartbeat_t));
  nn_xmsg_submsg_init (msg, sm_marker, SMID_HEARTBEAT);

  if (!hbansreq)
    hb->smhdr.flags |= HEARTBEAT_FLAG_FINAL;

  hb->readerId = nn_hton_entityid (dst);
  hb->writerId = nn_hton_entityid (wr->e.guid.entityid);
  if (whc_empty (wr->whc))
  {
    /* Really don't have data.  Fake one at the current wr->seq.
       We're not really allowed to generate heartbeats when the WHC is
       empty, but it appears RTI sort-of needs them ...  Now we use
       GAPs, and allocate a sequence number specially for that. */
    assert (config.respond_to_rti_init_zero_ack_with_invalid_heartbeat || wr->seq >= 1);
    max = wr->seq;
    min = max;
    if (config.respond_to_rti_init_zero_ack_with_invalid_heartbeat)
    {
      min += 1;
    }
  }
  else
  {
    min = whc_min_seq (wr->whc);
    max = whc_max_seq (wr->whc);
    assert (min <= max);
    if (!issync && wr->seq_xmit < max)
    {
      /* When: queue data ; queue heartbeat ; transmit data ; update
         seq_xmit, max may be < min.  But we must never advertise the
         minimum available sequence number incorrectly! */
      if (wr->seq_xmit >= min) {
        /* Advertise some but not all data */
        max = wr->seq_xmit;
      } else if (config.respond_to_rti_init_zero_ack_with_invalid_heartbeat) {
        /* if we can generate an empty heartbeat => do so. */
        max = min - 1;
      } else {
        /* claim the existence of a sample we possibly haven't set
           yet, at worst this causes a retransmission (but the
           NackDelay usually takes care of that). */
        max = min;
      }
    }
  }
  hb->firstSN = toSN (min);
  hb->lastSN = toSN (max);

  if (wr->hbcount == DDSI_COUNT_MAX)
    NN_FATAL0 ("writer reached maximum heartbeat sequence number");
  hb->count = ++wr->hbcount;

  nn_xmsg_submsg_setnext (msg, sm_marker);
  return 0;
}

int create_fragment_message (struct writer *wr, os_int64 seq, struct serdata *serdata, unsigned fragnum, struct proxy_reader *prd, struct nn_xmsg **pmsg, int isnew)
{
  /* We always fragment into FRAGMENT_SIZEd fragments, which are near
     the smallest allowed fragment size & can't be bothered (yet) to
     put multiple fragments into one DataFrag submessage if it makes
     sense to send large messages, as it would e.g. on GigE with jumbo
     frames.  If the sample is small enough to fit into one Data
     submessage, we require fragnum = 0 & generate a Data instead of a
     DataFrag.

     Note: fragnum is 0-based here, 1-based in DDSI. But 0-based is
     much easier ...

     Expected inline QoS size: header(4) + statusinfo(8) + keyhash(20)
     + sentinel(4). Plus some spare cos I can't be bothered. */
  const int set_smhdr_flags_asif_data = config.buggy_datafrag_flags_mode;
  const int expected_inline_qos_size = 4+8+20+4 + 32;
  struct nn_xmsg_marker sm_marker;
  void *sm;
  Data_DataFrag_common_t *ddcmn;
  int fragging;
  unsigned fragstart, fraglen;
  enum nn_xmsg_kind xmsg_kind = isnew ? NN_XMSG_KIND_DATA : NN_XMSG_KIND_DATA_REXMIT;
  int ret = 0;

  ASSERT_MUTEX_HELD (&wr->e.lock);

  if (fragnum * config.fragment_size >= serdata_size (serdata))
  {
    /* This is the first chance to detect an attempt at retransmitting
       an non-existent fragment, which a malicious (or buggy) remote
       reader can trigger.  So we return an error instead of asserting
       as we used to. */
    return ERR_INVALID;
  }

  fragging = (config.fragment_size < serdata_size (serdata));

  if ((*pmsg = nn_xmsg_new (gv.xmsgpool, &wr->e.guid.prefix, sizeof (InfoTimestamp_t) + sizeof (DataFrag_t) + expected_inline_qos_size, xmsg_kind)) == NULL)
    return ERR_OUT_OF_MEMORY;


  if (prd)
  {
    nn_xmsg_setdstPRD (*pmsg, prd);
    /* retransmits: latency budget doesn't apply */
  }
  else
  {
    nn_xmsg_setdstN (*pmsg, wr->as);
    nn_xmsg_setmaxdelay (*pmsg, nn_from_ddsi_duration (wr->xqos->latency_budget.duration));
  }

  /* Timestamp only needed once, for the first fragment */
  if (fragnum == 0)
  {
    if (nn_xmsg_add_timestamp (*pmsg, serdata->v.msginfo.timestamp) < 0)
      goto outofmem;
  }

  sm = nn_xmsg_append (*pmsg, &sm_marker, fragging ? sizeof (DataFrag_t) : sizeof (Data_t));
  ddcmn = sm;

  if (!fragging)
  {
    const unsigned contentflag = serdata_is_key (serdata) ? DATA_FLAG_KEYFLAG : DATA_FLAG_DATAFLAG;
    Data_t *data = sm;
    nn_xmsg_submsg_init (*pmsg, sm_marker, SMID_DATA);
    ddcmn->smhdr.flags |= contentflag;

    fragstart = 0;
    fraglen = serdata_size (serdata);
    ddcmn->octetsToInlineQos = (unsigned short) ((char*) (data+1) - ((char*) &ddcmn->octetsToInlineQos + 2));

    if (wr->reliable)
      nn_xmsg_setwriterseq (*pmsg, &wr->e.guid, seq);
  }
  else
  {
    const unsigned contentflag =
      set_smhdr_flags_asif_data
      ? (serdata_is_key (serdata) ? DATA_FLAG_KEYFLAG : DATA_FLAG_DATAFLAG)
      : (serdata_is_key (serdata) ? DATAFRAG_FLAG_KEYFLAG : 0);
    DataFrag_t *frag = sm;
    nn_xmsg_submsg_init (*pmsg, sm_marker, SMID_DATA_FRAG);
    ddcmn->smhdr.flags |= contentflag;

    frag->fragmentStartingNum = fragnum + 1;
    frag->fragmentsInSubmessage = 1;
    frag->fragmentSize = config.fragment_size;
    frag->sampleSize = serdata_size (serdata);

    fragstart = fragnum * config.fragment_size;
#if MULTIPLE_FRAGS_IN_SUBMSG /* ugly hack for testing only */
    if (fragstart + config.fragment_size < serdata_size (serdata) &&
        fragstart + 2 * config.fragment_size >= serdata_size (serdata))
      frag->fragmentsInSubmessage++;
    ret = frag->fragmentsInSubmessage;
#endif

    fraglen = config.fragment_size * frag->fragmentsInSubmessage;
    if (fragstart + fraglen > serdata_size (serdata))
      fraglen = serdata_size (serdata) - fragstart;
    ddcmn->octetsToInlineQos = (unsigned short) ((char*) (frag+1) - ((char*) &ddcmn->octetsToInlineQos + 2));

    if (wr->reliable && (!isnew || fragstart + fraglen == serdata_size (serdata)))
    {
      /* only set for final fragment for new messages; for rexmits we
         want it set for all so we can do merging. FIXME: I guess the
         writer should track both seq_xmit and the fragment number
         ... */
      nn_xmsg_setwriterseq_fragid (*pmsg, &wr->e.guid, seq, fragnum + frag->fragmentsInSubmessage - 1);
    }
  }

  ddcmn->extraFlags = 0;
  ddcmn->readerId = nn_hton_entityid (prd ? prd->e.guid.entityid : to_entityid (NN_ENTITYID_UNKNOWN));
  ddcmn->writerId = nn_hton_entityid (wr->e.guid.entityid);
  ddcmn->writerSN = toSN (seq);

  if (xmsg_kind == NN_XMSG_KIND_DATA_REXMIT)
    nn_xmsg_set_data_readerId (*pmsg, &ddcmn->readerId);

  Q_STATIC_ASSERT_CODE (DATA_FLAG_INLINE_QOS == DATAFRAG_FLAG_INLINE_QOS);
  assert (!(ddcmn->smhdr.flags & DATAFRAG_FLAG_INLINE_QOS));
  if (fragnum == 0)
  {
    int rc;
    if (wr->include_keyhash && nn_xmsg_addpar_keyhash (*pmsg, serdata) < 0)
      goto outofmem;
    if (serdata->v.msginfo.statusinfo && nn_xmsg_addpar_statusinfo (*pmsg, serdata->v.msginfo.statusinfo) < 0)
      goto outofmem;
    /* If it's 0 or 1, we know the proper calls have been made */
    assert (serdata->v.msginfo.have_wrinfo == 0 || serdata->v.msginfo.have_wrinfo == 1);
    if (serdata->v.msginfo.have_wrinfo && nn_xmsg_addpar_wrinfo (*pmsg, &serdata->v.msginfo.wrinfo) < 0)
      goto outofmem;
    if ((rc = nn_xmsg_addpar_sentinel_ifparam (*pmsg)) < 0)
      goto outofmem;
    if (rc > 0)
      ddcmn->smhdr.flags |= DATAFRAG_FLAG_INLINE_QOS;
  }

  nn_xmsg_serdata (*pmsg, serdata, fragstart, fraglen);
  nn_xmsg_submsg_setnext (*pmsg, sm_marker);
#if 0
  TRACE (("queue data%s %x:%x:%x:%x #%lld/%u[%u..%u)\n",
          fragging ? "frag" : "", PGUID (wr->e.guid),
          seq, fragnum+1, fragstart, fragstart + fraglen));
#endif
  return ret;

 outofmem:
  nn_xmsg_free (*pmsg);
  *pmsg = NULL;
  return ERR_OUT_OF_MEMORY;
}

static void create_HeartbeatFrag (struct writer *wr, os_int64 seq, unsigned fragnum, struct proxy_reader *prd,struct nn_xmsg **pmsg)
{
  struct nn_xmsg_marker sm_marker;
  HeartbeatFrag_t *hbf;
  ASSERT_MUTEX_HELD (&wr->e.lock);
  if ((*pmsg = nn_xmsg_new (gv.xmsgpool, &wr->e.guid.prefix, sizeof (HeartbeatFrag_t), NN_XMSG_KIND_CONTROL)) == NULL)
    return; /* ignore out-of-memory: HeartbeatFrag is only advisory anyway */
  if (prd)
    nn_xmsg_setdstPRD (*pmsg, prd);
  else
    nn_xmsg_setdstN (*pmsg, wr->as);
  hbf = nn_xmsg_append (*pmsg, &sm_marker, sizeof (HeartbeatFrag_t));
  nn_xmsg_submsg_init (*pmsg, sm_marker, SMID_HEARTBEAT_FRAG);
  hbf->readerId = nn_hton_entityid (prd ? prd->e.guid.entityid : to_entityid (NN_ENTITYID_UNKNOWN));
  hbf->writerId = nn_hton_entityid (wr->e.guid.entityid);
  hbf->writerSN = toSN (seq);
  hbf->lastFragmentNum = fragnum + 1; /* network format is 1 based */

  if (wr->hbfragcount == DDSI_COUNT_MAX)
    NN_FATAL0 ("writer reached maximum heartbeat-frag sequence number");
  hbf->count = ++wr->hbfragcount;

  nn_xmsg_submsg_setnext (*pmsg, sm_marker);
}

#if 0
static int must_skip_frag (const char *frags_to_skip, int frag)
{
  /* one based, for easier reading of logs */
  char str[14];
  int n, m;
  if (frags_to_skip == NULL)
    return 0;
  n = snprintf (str, sizeof (str), ",%d,", frag + 1);
  if (strstr (frags_to_skip, str))
    return 1; /* somewhere in middle */
  if (strncmp (frags_to_skip, str+1, n-1) == 0)
    return 1; /* first in list */
  str[--n] = 0; /* drop trailing comma */
  if (strcmp (frags_to_skip, str+1) == 0)
    return 1; /* only one */
  m = strlen (frags_to_skip);
  if (m >= n && strcmp (frags_to_skip + m - n, str) == 0)
    return 1; /* last one in list */
  return 0;
}
#endif

static int transmit_sample (struct nn_xpack *xp, struct writer *wr, os_int64 seq, serdata_t serdata, struct proxy_reader *prd, int isnew)
{
  unsigned i, sz, nfrags;
#if 0
  const char *frags_to_skip = getenv ("SKIPFRAGS");
#endif
  assert(xp);

  sz = serdata_size (serdata);
  nfrags = (sz + config.fragment_size - 1) / config.fragment_size;
  for (i = 0; i < nfrags; i++)
  {
    struct nn_xmsg *fmsg = NULL;
    struct nn_xmsg *hmsg = NULL;
    int ret;
#if 0
    if (must_skip_frag (frags_to_skip, i))
      continue;
#endif
    /* Ignore out-of-memory errors: we can't do anything about it, and
       eventually we'll have to retry.  But if a packet went out and
       we haven't yet completed transmitting a fragmented message, add
       a HeartbeatFrag. */
    os_mutexLock (&wr->e.lock);
    ret = create_fragment_message (wr, seq, serdata, i, prd, &fmsg, isnew);
    if (ret >= 0)
    {
      if (nfrags > 1 && i + 1 < nfrags)
        create_HeartbeatFrag (wr, seq, i, prd, &hmsg);
    }
    os_mutexUnlock (&wr->e.lock);

    if(fmsg) nn_xpack_addmsg (xp, fmsg);
    if(hmsg) nn_xpack_addmsg (xp, hmsg);

#if MULTIPLE_FRAGS_IN_SUBMSG /* ugly hack for testing only */
    if (ret > 1)
      i += ret-1;
#endif
  }

  /* Note: wr->heartbeat_xevent != NULL <=> wr is reliable */
  if (wr->heartbeat_xevent)
  {
    struct nn_xmsg *msg = NULL;
    int hbansreq;
    os_mutexLock (&wr->e.lock);
    msg = writer_hbcontrol_piggyback (wr, serdata_twrite (serdata), nn_xpack_packetid (xp), &hbansreq);
    os_mutexUnlock (&wr->e.lock);
    if (msg)
    {
      nn_xpack_addmsg (xp, msg);
      if (hbansreq >= 2)
        nn_xpack_send (xp);
    }
  }

  return 0;
}

int enqueue_sample_wrlock_held (struct writer *wr, os_int64 seq, serdata_t serdata, struct proxy_reader *prd, int isnew)
{
  unsigned i, sz, nfrags;
  int enqueued = 1;

  ASSERT_MUTEX_HELD (&wr->e.lock);

  sz = serdata_size (serdata);
  nfrags = (sz + config.fragment_size - 1) / config.fragment_size;
  for (i = 0; i < nfrags && enqueued; i++)
  {
    struct nn_xmsg *fmsg = NULL;
    struct nn_xmsg *hmsg = NULL;
    /* Ignore out-of-memory errors: we can't do anything about it, and
       eventually we'll have to retry.  But if a packet went out and
       we haven't yet completed transmitting a fragmented message, add
       a HeartbeatFrag. */
    if (create_fragment_message (wr, seq, serdata, i, prd, &fmsg, isnew) >= 0)
    {
      if (nfrags > 1 && i + 1 < nfrags)
        create_HeartbeatFrag (wr, seq, i, prd, &hmsg);
    }
    if (isnew)
    {
      if(fmsg) qxev_msg (wr->evq, fmsg);
      if(hmsg) qxev_msg (wr->evq, hmsg);
    }
    else
    {
      /* Because of the way DDSI2 handles retransmitting fragmented data
         it must be able to queue the entire sample regardless of the
         queuing limits set.  By allowing the qxev_msg_rexmit call to
         drop the first fragment, but forcing it to accept all of them
         if it accepted the first, we can always enqueued any size sample
         while retaining some semblance of a limited-size queue. */
      const int force = (i != 0);
      if(fmsg)
      {
        enqueued = qxev_msg_rexmit_wrlock_held (wr->evq, fmsg, force);
      }
      /* Functioning of the system is not dependent on getting the
         HeartbeatFrags out, so never force them into the queue. */
      if(hmsg)
      {
        if (enqueued)
          qxev_msg (wr->evq, hmsg);
        else
          nn_xmsg_free (hmsg);
      }
    }
  }
  return enqueued ? 0 : -1;
}

static int insert_sample_in_whc (struct writer *wr, os_int64 * pseq, serdata_t serdata)
{
  /* DCPS write/dispose/writeDispose/unregister and the with_timestamp
     variants, all folded into a single function (all the message info
     and the payload is encoded in serdata).

     If sending fails (out-of-memory is the only real one), don't
     report an error, as we can always try again. If adding the sample
     to the WHC fails, however, we never accepted the sample and do
     return an error.

     If xp = NULL => queue the events, else simply pack them. */
  int res;
  *pseq = ++wr->seq;

  ASSERT_MUTEX_HELD (&wr->e.lock);

  assert (serdata_refcount_is_1 (serdata));
  if (config.enabled_logcats & LC_TRACE)
  {
    char ppbuf[1024];
    int tmp;
    const char *tname = wr->topic ? topic_name (wr->topic) : "(null)";
    const char *ttname = wr->topic ? topic_typename (wr->topic) : "(null)";
    tmp = prettyprint_serdata (ppbuf, sizeof (ppbuf), serdata);
    nn_log (LC_TRACE, "write_sample %x:%x:%x:%x #%lld: ST%d %s/%s:%s%s\n",
            PGUID (wr->e.guid), *pseq, serdata->v.msginfo.statusinfo,
            tname, ttname, ppbuf,
            tmp < (int) sizeof (ppbuf) ? "" : " (trunc)");
  }

  assert (wr->reliable || have_reliable_subs (wr) == 0);

  if ((wr->reliable && have_reliable_subs (wr)) || wr->handle_as_transient_local || wr->startup_mode)
    res = whc_insert (wr->whc, writer_max_drop_seq (wr), *pseq, serdata);
  else
    res = 0;

#ifndef NDEBUG
  if (wr->e.guid.entityid.u == NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER)
  {
    if (whc_findmax (wr->whc) == NULL)
      assert (wr->c.pp->builtins_deleted);
  }
#endif
  return res;
}

static int writer_must_throttle (const struct writer *wr)
{
  return writer_number_of_unacked_samples (wr) > config.whc_highwater_mark;
}

static int writer_may_continue (const struct writer *wr)
{
  return writer_number_of_unacked_samples (wr) < config.whc_lowwater_mark || (wr->state != WRST_OPERATIONAL);
}

static void throttle_helper (struct wr_prd_match *wprd, struct writer * const wr)
{
  /* Mark connected readers that haven't acked all data as "not
     responsive". Both wprd->seq (&c.) and wr->seq are protected by
     wr->e.lock, which must be held on entry. */
  ASSERT_MUTEX_HELD (&wr->e.lock);

  if (wprd->seq < wr->seq)
  {
    wprd->seq = MAX_SEQ_NUMBER;
    /* ensure heartbeats will be going out - else it might not have a
       chance to recover */
    wprd->has_replied_to_hb = 0;
    ut_avlAugmentUpdate (&wr_readers_treedef, wprd);
    NN_WARNING2 ("writer %x:%x:%x:%x considering reader %x:%x:%x:%x non-responsive\n",
                 PGUID ( wr->e.guid), PGUID (wprd->prd_guid));
  }
}

static int throttle_writer (struct nn_xpack *xp, struct writer *wr)
{
  /* We don't _really_ need to hold the lock if we can decide whether
     or not to throttle based on atomically updated state. Currently,
     this is the case, and if it remains that way, maybe we should
     eventually remove the precondition that the lock be held. */
  ASSERT_MUTEX_HELD (&wr->e.lock);
  assert (vtime_awake_p (lookup_thread_state ()->vtime));

  /* Sleep (cond_wait) without updating the thread's vtime: the
     garbage collector won't free the writer while we leave it
     unchanged.  Alternatively, we could decide to go back to sleep,
     allow garbage collection and check the writers existence every
     time we get woken up.  That would preclude the use of a condition
     variable embedded in "struct writer", of course.

     For normal data that would be okay, because the thread forwarding
     data from the network queue to rtps_write() simply uses the gid
     and doesn't mind if the writer is freed halfway through (although
     we would have to specify it may do so it!); but for internal
     data, it would be absolutely unacceptable if they were ever to
     take the path that would increase vtime.

     Currently, rtps_write/throttle_writer are used only by the normal
     data forwarding path, the internal ones use write_sample().  Not
     worth the bother right now.

     Therefore, we don't check the writer is still there after waking
     up.

     Used to block on a combination of |xeventq| and |whc|, but that
     is hard now that we use a per-writer condition variable.  So
     instead, wait until |whc| is small enough, then wait for
     |xeventq|.  The reasoning is that the WHC won't grow
     spontaneously the way the xevent queue does.

     If the |whc| is dropping with in a configurable timeframe
     (default 1 second) all connected readers that still haven't acked
     all data, are considered "non-responsive" and data is no longer
     resent to them, until a ACKNACK is received from that
     reader. This implicitly clears the whc and unblocks the
     writer. */
  if (!writer_must_throttle (wr))
    return 0;
  else
  {
    const os_int64 abstimeout = now() + config.responsiveness_timeout;
    int n_unacked;
    n_unacked = writer_number_of_unacked_samples (wr);
    TRACE (("writer %x:%x:%x:%x waiting for whc to shrink below low-water mark (whc %d)\n", PGUID (wr->e.guid), n_unacked));
    wr->throttling++;

    /* Force any outstanding packet out: there will be a heartbeat
       requesting an answer in it.  FIXME: obviously, this is doing
       things the wrong way round ... */
    if (xp)
    {
      os_mutexUnlock (&wr->e.lock);
      nn_xpack_send (xp);
      os_mutexLock (&wr->e.lock);
    }

    while (gv.rtps_keepgoing && !writer_may_continue (wr)) {
      const os_int64 reltimeout = abstimeout - now();
      os_result result;
      if (reltimeout <= 0)
        result = os_resultTimeout;
      else
      {
        os_time timeout;
        timeout.tv_sec = (os_int32) (reltimeout / T_SECOND);
        timeout.tv_nsec = (os_int32) (reltimeout % T_SECOND);
        result = os_condTimedWait (&wr->throttle_cond, &wr->e.lock, &timeout);
      }
      if (result == os_resultTimeout)
      {
        /* Walk over all connected readers and mark them "not
         responsive" if they have unacked data. */
        n_unacked = writer_number_of_unacked_samples (wr);
        TRACE (("writer %x:%x:%x:%x whc not shrunk enough after maximum blocking time (whc %d)\n", PGUID (wr->e.guid), n_unacked));
        ut_avlWalk (&wr_readers_treedef, &wr->readers, (ut_avlWalk_t) throttle_helper, wr);
        remove_acked_messages (wr);
        os_condBroadcast (&wr->throttle_cond);
      }
    }

    wr->throttling--;

    n_unacked = writer_number_of_unacked_samples (wr);
    TRACE (("writer %x:%x:%x:%x done waiting for whc to shrink below low-water mark (whc %d)\n", PGUID (wr->e.guid), n_unacked));
    return 1;
  }
}

int write_sample_kernel_seq (struct nn_xpack *xp, struct writer *wr, serdata_t serdata, int have_kernel_seq, os_uint32 kernel_seq)
{
  int r;
  os_int64 seq;
  os_int64 tnow;

  if (serdata_size (serdata) > config.max_sample_size)
  {
    char ppbuf[1024];
    int tmp;
    const char *tname = wr->topic ? topic_name (wr->topic) : "(null)";
    const char *ttname = wr->topic ? topic_typename (wr->topic) : "(null)";
    tmp = prettyprint_serdata (ppbuf, sizeof (ppbuf), serdata);
    NN_WARNING7 ("dropping oversize (%u > %u) sample from local writer %x:%x:%x:%x %s/%s:%s%s\n",
                 serdata_size (serdata), config.max_sample_size,
                 PGUID (wr->e.guid), tname, ttname, ppbuf,
                 tmp < (int) sizeof (ppbuf) ? "" : " (trunc)");
    r = ERR_INVALID_DATA;
    goto drop;
  }

  os_mutexLock (&wr->e.lock);

  if (config.forward_all_messages || !have_kernel_seq)
  {
    /* no filtering */
  }
  else if (wr->last_kernel_seq != kernel_seq)
  {
    wr->last_kernel_seq = kernel_seq;
  }
  else
  {
    os_mutexUnlock (&wr->e.lock);
    TRACE (("write_sample %x:%x:%x:%x - dropping kernel seq %u as duplicate\n",
            PGUID (wr->e.guid), kernel_seq));
    r = 0;
    goto drop;
  }

  /* If we blocked in throttle_writer, we must read the clock
     again. If we didn't but the timestamp in serdata's msginfo is
     not known to be the current time a few clock ticks ago, we must
     do so too. Only when we did not block & know that we have a
     good timestamp available we can avoid it. */
  if (throttle_writer (xp, wr) || !serdata->v.msginfo.timestamp_is_now)
  {
    tnow = now ();
  }
  else
  {
    tnow = serdata->v.msginfo.timestamp;
  }
  serdata_set_twrite (serdata, tnow);
  r = insert_sample_in_whc (wr, &seq, serdata);

  /* Note the subtlety of enqueueing with the lock held but
     transmitting without holding the lock. Still working on cleaning
     that up. */
  if (xp)
  {
    os_mutexUnlock (&wr->e.lock);
    if (r >= 0)
    {
      transmit_sample (xp, wr, seq, serdata, NULL, 1);
    }
  }
  else
  {
    if (r >= 0)
    {
      if (wr->heartbeat_xevent)
        writer_hbcontrol_note_asyncwrite (wr, tnow);
      enqueue_sample_wrlock_held (wr, seq, serdata, NULL, 1);
    }
    os_mutexUnlock (&wr->e.lock);
  }

drop:
  /* FIXME: shouldn't I move the serdata_unref call to the callers? */
  serdata_unref (serdata);
  return r;
}

int write_sample (struct nn_xpack *xp, struct writer *wr, serdata_t serdata)
{
  assert (is_builtin_entityid (wr->e.guid.entityid));
  return write_sample_kernel_seq (xp, wr, serdata, 0, 0);
}

/* SHA1 not available (unoffical build.) */
