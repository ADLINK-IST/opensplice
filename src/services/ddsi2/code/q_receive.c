/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "os_heap.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_thread.h"
#include "os_socket.h"
#include "os_stdlib.h"
#include "os_if.h"

#include "v_group.h"
#include "v_partition.h"
#include "v_groupSet.h"
#include "v_entity.h"
#include "v_state.h"

#include "q_md5.h"
#include "q_avl.h"
#include "q_osplser.h"
#include "q_protocol.h"
#include "q_rtps.h"
#include "q_misc.h"
#include "q_config.h"
#include "q_log.h"
#include "q_mlv.h"
#include "q_plist.h"
#include "q_unused.h"
#include "q_groupset.h"
#include "q_bswap.h"
#include "q_lat_estim.h"
#include "q_bitset.h"
#include "q_xevent.h"
#include "q_align.h"
#include "q_addrset.h"
#include "q_ddsi_discovery.h"
#include "q_radmin.h"
#include "q_error.h"
#include "q_thread.h"
#include "q_ephash.h"
#include "q_lease.h"
#include "q_gc.h"
#include "q_whc.h"
#include "q_entity.h"
#include "q_xmsg.h"
#include "q_receive.h"
#include "q_transmit.h"
#include "q_osplserModule.h"
#include "q_globals.h"
#include "q_pcap.h"
#include "q_static_assert.h"

#include "sysdeps.h"

/*
Notes:

- for now, the safer option is usually chosen: hold a lock even if it
  isn't strictly necessary in the particular configuration we have
  (such as one receive thread vs. multiple receive threads)

- nn_dqueue_enqueue may be called with pwr->e.lock held

- deliver_user_data_synchronously may be called with pwr->e.lock held,
  which is needed if IN-ORDER synchronous delivery is desired when
  there are also multiple receive threads

*/

static void deliver_user_data_synchronously (struct nn_rsample_chain *sc);

static void maybe_set_reader_in_sync (struct pwr_rd_match *wn)
{
  assert (!wn->in_sync);
  if (nn_reorder_next_seq (wn->u.not_in_sync.reorder) > wn->u.not_in_sync.end_of_tl_seq)
    wn->in_sync = 1;
}

static int valid_sequence_number_set (const nn_sequence_number_set_t *snset)
{
  if (fromSN (snset->bitmap_base) <= 0)
    return 0;
  if (snset->numbits <= 0 || snset->numbits > 256)
    return 0;
  return 1;
}

static int valid_fragment_number_set (const nn_fragment_number_set_t *fnset)
{
  if (fnset->bitmap_base <= 0)
    return 0;
  if (fnset->numbits <= 0 || fnset->numbits > 256)
    return 0;
  return 1;
}

static int valid_AckNack (AckNack_t *msg, int size, int byteswap)
{
  nn_count_t *count; /* this should've preceded the bitmap */
  if (size < (int) ACKNACK_SIZE (0))
    /* note: sizeof(*msg) is not sufficient verification, but it does
       suffice for verifying all fixed header fields exist */
    return 0;
  if (byteswap)
  {
    bswap_sequence_number_set_hdr (&msg->readerSNState);
    /* bits[], count deferred until validation of fixed part */
  }
  msg->readerId = nn_ntoh_entityid (msg->readerId);
  msg->writerId = nn_ntoh_entityid (msg->writerId);
  /* Validation following 8.3.7.1.3 + 8.3.5.5 */
  if (!valid_sequence_number_set (&msg->readerSNState))
  {
    if (NN_STRICT_P)
      return 0;
    else
    {
      /* RTI generates AckNacks with bitmapBase = 0 and numBits = 0
         (and possibly others that I don't know about) - their
         Wireshark RTPS dissector says that such a message has a
         length-0 bitmap, which is to expected given the way the
         length is computed from numbits */
      if (fromSN (msg->readerSNState.bitmap_base) == 0 &&
          msg->readerSNState.numbits == 0)
        ; /* accept this one known case */
      else if (msg->readerSNState.numbits == 0)
        ; /* maybe RTI, definitely Twinoaks */
      else
        return 0;
    }
  }
  /* Given the number of bits, we can compute the size of the AckNack
     submessage, and verify that the submessage is large enough */
  if (size < (int) ACKNACK_SIZE (msg->readerSNState.numbits))
    return 0;
  count = (nn_count_t *) ((char *) &msg->readerSNState +
                          NN_SEQUENCE_NUMBER_SET_SIZE (msg->readerSNState.numbits));
  if (byteswap)
  {
    bswap_sequence_number_set_bitmap (&msg->readerSNState);
    *count = bswap4 (*count);
  }
  return 1;
}

static int valid_Gap (Gap_t *msg, int size, int byteswap)
{
  if (size < (int) GAP_SIZE (0))
    return 0;
  if (byteswap)
  {
    bswapSN (&msg->gapStart);
    bswap_sequence_number_set_hdr (&msg->gapList);
  }
  msg->readerId = nn_ntoh_entityid (msg->readerId);
  msg->writerId = nn_ntoh_entityid (msg->writerId);
  if (fromSN (msg->gapStart) <= 0)
    return 0;
  if (!valid_sequence_number_set (&msg->gapList))
  {
    if (NN_STRICT_P || msg->gapList.numbits != 0)
      return 0;
  }
  if (size < (int) GAP_SIZE (msg->gapList.numbits))
    return 0;
  if (byteswap)
    bswap_sequence_number_set_bitmap (&msg->gapList);
  return 1;
}

static int valid_InfoDST (InfoDST_t *msg, int size, UNUSED_ARG (int byteswap))
{
  if (size < (int) sizeof (*msg))
    return 0;
  return 1;
}

static int valid_InfoSRC (InfoSRC_t *msg, int size, UNUSED_ARG (int byteswap))
{
  if (size < (int) sizeof (*msg))
    return 0;
  return 1;
}

static int valid_InfoTS (InfoTS_t *msg, int size, int byteswap)
{
  if (size < (int) sizeof (InfoTS_t))
    return 0;
  else if (msg->smhdr.flags & INFOTS_INVALIDATE_FLAG)
    return 1;
  else
  {
    if (byteswap)
    {
      msg->time.seconds = bswap4 (msg->time.seconds);
      msg->time.fraction = bswap4u (msg->time.fraction);
    }
    return valid_ddsi_timestamp (msg->time);
  }
}

static int valid_PT_InfoContainer (PT_InfoContainer_t *msg, int size, int byteswap)
{
  if (size < (int) sizeof (PT_InfoContainer_t))
    return 0;
#if 0
  if (msg->smhdr.flags)
    return 0;
#endif
  if (byteswap)
    msg->id = bswap4u (msg->id);
  return 1;
}

static int valid_Heartbeat (Heartbeat_t *msg, int size, int byteswap)
{
  if (size < (int) sizeof (*msg))
    return 0;
  if (byteswap)
  {
    bswapSN (&msg->firstSN);
    bswapSN (&msg->lastSN);
    msg->count = bswap4 (msg->count);
  }
  msg->readerId = nn_ntoh_entityid (msg->readerId);
  msg->writerId = nn_ntoh_entityid (msg->writerId);
  /* Validation following 8.3.7.5.3 */
  if (fromSN (msg->firstSN) <= 0 ||
      /* fromSN (msg->lastSN) <= 0 || -- implicit in last < first */
      fromSN (msg->lastSN) < fromSN (msg->firstSN))
  {
    if (NN_STRICT_P)
      return 0;
    else
    {
      /* Note that we don't actually know the set of all possible
         malformed messages that we have to process, so we stick to
         the ones we've seen */
      if (fromSN (msg->firstSN) == fromSN (msg->lastSN) + 1)
        ; /* ok */
      else
        return 0;
    }
  }
  return 1;
}

static int valid_HeartbeatFrag (HeartbeatFrag_t *msg, int size, int byteswap)
{
  if (size < (int) sizeof (*msg))
    return 0;
  if (byteswap)
  {
    bswapSN (&msg->writerSN);
    msg->lastFragmentNum = bswap4u (msg->lastFragmentNum);
    msg->count = bswap4 (msg->count);
  }
  msg->readerId = nn_ntoh_entityid (msg->readerId);
  msg->writerId = nn_ntoh_entityid (msg->writerId);
  if (fromSN (msg->writerSN) <= 0 || msg->lastFragmentNum == 0)
    return 0;
  return 1;
}

static int valid_NackFrag (NackFrag_t *msg, int size, int byteswap)
{
  nn_count_t *count; /* this should've preceded the bitmap */
  if (size < (int) NACKFRAG_SIZE (0))
    /* note: sizeof(*msg) is not sufficient verification, but it does
       suffice for verifying all fixed header fields exist */
    return 0;
  if (byteswap)
  {
    bswapSN (&msg->writerSN);
    bswap_fragment_number_set_hdr (&msg->fragmentNumberState);
    /* bits[], count deferred until validation of fixed part */
  }
  msg->readerId = nn_ntoh_entityid (msg->readerId);
  msg->writerId = nn_ntoh_entityid (msg->writerId);
  /* Validation following 8.3.7.1.3 + 8.3.5.5 */
  if (!valid_fragment_number_set (&msg->fragmentNumberState))
    return 0;
  /* Given the number of bits, we can compute the size of the Nackfrag
     submessage, and verify that the submessage is large enough */
  if (size < (int) NACKFRAG_SIZE (msg->fragmentNumberState.numbits))
    return 0;
  count = (nn_count_t *) ((char *) &msg->fragmentNumberState +
                          NN_FRAGMENT_NUMBER_SET_SIZE (msg->fragmentNumberState.numbits));
  if (byteswap)
  {
    bswap_fragment_number_set_bitmap (&msg->fragmentNumberState);
    *count = bswap4 (*count);
  }
  return 1;
}

static int valid_Data (const struct receiver_state *rst, struct nn_rmsg *rmsg, Data_t *msg, int size, int byteswap, struct nn_rsample_info *sampleinfo, char **payloadp)
{
  /* on success: sampleinfo->{seq,rst,statusinfo,pt_wr_info_zoff,bswap,complex_qos} all set */
  nn_guid_t pwr_guid;
  char *ptr;

  if (size < (int) sizeof (*msg))
    return 0; /* too small even for fixed fields */
  /* D=1 && K=1 is invalid in this version of the protocol */
  if ((msg->x.smhdr.flags & (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)) ==
      (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG))
    return 0;
  if (byteswap)
  {
    msg->x.extraFlags = bswap2u (msg->x.extraFlags);
    msg->x.octetsToInlineQos = bswap2u (msg->x.octetsToInlineQos);
    bswapSN (&msg->x.writerSN);
  }
  msg->x.readerId = nn_ntoh_entityid (msg->x.readerId);
  msg->x.writerId = nn_ntoh_entityid (msg->x.writerId);
  pwr_guid.prefix = rst->src_guid_prefix;
  pwr_guid.entityid = msg->x.writerId;

  sampleinfo->rst = (struct receiver_state *) rst; /* drop const */
  sampleinfo->pwr = ephash_lookup_proxy_writer_guid (&pwr_guid);
  sampleinfo->seq = fromSN (msg->x.writerSN);
  sampleinfo->fragsize = 0; /* for unfragmented data, fragsize = 0 works swell */
  sampleinfo->bswap = byteswap;

  if ((msg->x.smhdr.flags & (DATA_FLAG_INLINE_QOS | DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)) == 0)
  {
    /* no QoS, no payload, so octetsToInlineQos will never be used
       though one would expect octetsToInlineQos and size to be in
       agreement or octetsToInlineQos to be 0 or so */
    *payloadp = NULL;
    sampleinfo->size = 0; /* size is full payload size, no payload & unfragmented => size = 0 */
    sampleinfo->statusinfo = 0;
    sampleinfo->pt_wr_info_zoff = NN_OFF_TO_ZOFF (0);
    sampleinfo->complex_qos = 0;
    return 1;
  }

  /* QoS and/or payload, so octetsToInlineQos must be within the
     msg; since the serialized data and serialized parameter lists
     have a 4 byte header, that one, too must fit */
  if ((int) (offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->x.octetsToInlineQos) + 4) > size)
    return 0;

  ptr = (char *) msg + offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->x.octetsToInlineQos) + msg->x.octetsToInlineQos;
  if (msg->x.smhdr.flags & DATA_FLAG_INLINE_QOS)
  {
    nn_plist_src_t src;
    src.protocol_version = rst->protocol_version;
    src.vendorid = rst->vendor;
    src.encoding = (msg->x.smhdr.flags & SMFLAG_ENDIANNESS) ? PL_CDR_LE : PL_CDR_BE;
    src.buf = ptr;
    src.bufsz = (char *) msg + size - src.buf; /* end of message, that's all we know */
    /* just a quick scan, gathering only what we _really_ need */
    if ((ptr = nn_plist_quickscan (sampleinfo, rmsg, &src)) == NULL)
      return 0;
  }
  else
  {
    sampleinfo->statusinfo = 0;
    sampleinfo->pt_wr_info_zoff = NN_OFF_TO_ZOFF (0);
    sampleinfo->complex_qos = 0;
  }

  if (!(msg->x.smhdr.flags & (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)))
  {
    /*TRACE (("no payload\n"));*/
    *payloadp = NULL;
    sampleinfo->size = 0;
  }
  else if ((char *) ptr + 4 - (char *) msg > size)
  {
    /* no space for the header */
    return 0;
  }
  else
  {
    struct CDRHeader *hdr;
    sampleinfo->size = (char *) msg + size - (char *) ptr;
    *payloadp = ptr;
    hdr = (struct CDRHeader *) ptr;
    switch (hdr->identifier)
    {
      case CDR_BE:
      case CDR_LE:
        break;
      case PL_CDR_BE:
      case PL_CDR_LE:
        break;
      default:
        return 0;
    }
  }
  return 1;
}

static int valid_DataFrag (const struct receiver_state *rst, struct nn_rmsg *rmsg, DataFrag_t *msg, int size, int byteswap, struct nn_rsample_info *sampleinfo, char **payloadp)
{
  /* on success: sampleinfo->{rst,statusinfo,pt_wr_info_zoff,bswap,complex_qos} all set */
  const int interpret_smhdr_flags_asif_data = config.buggy_datafrag_flags_mode;
  os_uint32 payloadsz;
  nn_guid_t pwr_guid;
  char *ptr;

  if (size < (int) sizeof (*msg))
    return 0; /* too small even for fixed fields */

  if (interpret_smhdr_flags_asif_data)
  {
    /* D=1 && K=1 is invalid in this version of the protocol */
    if ((msg->x.smhdr.flags & (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)) ==
        (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG))
      return 0;
  }

  if (byteswap)
  {
    msg->x.extraFlags = bswap2u (msg->x.extraFlags);
    msg->x.octetsToInlineQos = bswap2u (msg->x.octetsToInlineQos);
    bswapSN (&msg->x.writerSN);
    msg->fragmentStartingNum = bswap4u (msg->fragmentStartingNum);
    msg->fragmentsInSubmessage = bswap2u (msg->fragmentsInSubmessage);
    msg->fragmentSize = bswap2u (msg->fragmentSize);
    msg->sampleSize = bswap4u (msg->sampleSize);
  }
  msg->x.readerId = nn_ntoh_entityid (msg->x.readerId);
  msg->x.writerId = nn_ntoh_entityid (msg->x.writerId);
  pwr_guid.prefix = rst->src_guid_prefix;
  pwr_guid.entityid = msg->x.writerId;

  if (NN_STRICT_P && msg->fragmentSize <= 1024 && msg->fragmentSize < config.fragment_size)
  {
    /* Spec says fragments must > 1kB; not allowing 1024 bytes is IMHO
       totally ridiculous; and I really don't care how small the
       fragments anyway. And we're certainly not going too fail the
       message if it is as least as large as the configured fragment
       size. */
    return 0;
  }
  if (msg->fragmentSize == 0 || msg->fragmentStartingNum == 0 || msg->fragmentsInSubmessage == 0)
    return 0;
  if (NN_STRICT_P && msg->fragmentSize >= msg->sampleSize)
    /* may not fragment if not needed -- but I don't care */
    return 0;
  if ((msg->fragmentStartingNum + msg->fragmentsInSubmessage - 2) * msg->fragmentSize >= msg->sampleSize)
    /* starting offset of last fragment must be within sample, note:
       fragment numbers are 1-based */
    return 0;

  sampleinfo->rst = (struct receiver_state *) rst; /* drop const */
  sampleinfo->pwr = ephash_lookup_proxy_writer_guid (&pwr_guid);
  sampleinfo->seq = fromSN (msg->x.writerSN);
  sampleinfo->fragsize = msg->fragmentSize;
  sampleinfo->size = msg->sampleSize;
  sampleinfo->bswap = byteswap;

  if (interpret_smhdr_flags_asif_data)
  {
    if ((msg->x.smhdr.flags & (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)) == 0)
      /* may not fragment if not needed => surely _some_ payload must be present! */
      return 0;
  }

  /* QoS and/or payload, so octetsToInlineQos must be within the msg;
     since the serialized data and serialized parameter lists have a 4
     byte header, that one, too must fit */
  if ((int) (offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->x.octetsToInlineQos) + 4) >= size)
    return 0;

  /* Quick check inline QoS if present, collecting a little bit of
     information on it.  The only way to find the payload offset if
     inline QoSs are present. */
  ptr = (char *) msg + offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->x.octetsToInlineQos) + msg->x.octetsToInlineQos;
  if (msg->x.smhdr.flags & DATAFRAG_FLAG_INLINE_QOS)
  {
    nn_plist_src_t src;
    src.protocol_version = rst->protocol_version;
    src.vendorid = rst->vendor;
    src.encoding = (msg->x.smhdr.flags & SMFLAG_ENDIANNESS) ? PL_CDR_LE : PL_CDR_BE;
    src.buf = ptr;
    src.bufsz = (char *) msg + size - src.buf; /* end of message, that's all we know */
    /* just a quick scan, gathering only what we _really_ need */
    if ((ptr = nn_plist_quickscan (sampleinfo, rmsg, &src)) == NULL)
      return 0;
  }
  else
  {
    sampleinfo->statusinfo = 0;
    sampleinfo->pt_wr_info_zoff = NN_OFF_TO_ZOFF (0);
    sampleinfo->complex_qos = 0;
  }

  *payloadp = ptr;
  payloadsz = (char *) msg + size - (char *) ptr;
  if ((os_uint32) msg->fragmentsInSubmessage * msg->fragmentSize <= payloadsz)
    ; /* all spec'd fragments fit in payload */
  else if ((os_uint32) (msg->fragmentsInSubmessage - 1) * msg->fragmentSize >= payloadsz)
    return 0; /* I can live with a short final fragment, but _only_ the final one */
  else if ((os_uint32) ((msg->fragmentStartingNum - 1) * msg->fragmentSize + payloadsz >= msg->sampleSize))
    ; /* final fragment is long enough to cover rest of sample */
  else
    return 0;
  if (msg->fragmentStartingNum == 1)
  {
    struct CDRHeader *hdr = (struct CDRHeader *) ptr;
    if ((char *) ptr + 4 - (char *) msg > size)
    {
      /* no space for the header -- technically, allowing small
         fragments would also mean allowing a partial header, but I
         prefer this */
      return 0;
    }
    switch (hdr->identifier)
    {
      case CDR_BE:
      case CDR_LE:
        break;
      case PL_CDR_BE:
      case PL_CDR_LE:
        break;
      default:
        return 0;
    }
  }
  return 1;
}

static int add_Gap (struct nn_xmsg *msg, struct writer *wr, struct proxy_reader *prd, os_int64 start, os_int64 base, int numbits, const unsigned *bits)
{
  struct nn_xmsg_marker sm_marker;
  Gap_t *gap;
  ASSERT_MUTEX_HELD (wr->e.lock);
  assert (numbits > 0);
  if ((gap = nn_xmsg_append_aligned (msg, &sm_marker, GAP_SIZE (numbits), 4)) == NULL)
    return ERR_OUT_OF_MEMORY;
  nn_xmsg_submsg_init (msg, sm_marker, SMID_GAP);
  gap->readerId = nn_hton_entityid (prd->e.guid.entityid);
  gap->writerId = nn_hton_entityid (wr->e.guid.entityid);
  gap->gapStart = toSN (start);
  gap->gapList.bitmap_base = toSN (base);
  gap->gapList.numbits = numbits;
  memcpy (gap->gapList.bits, bits, NN_SEQUENCE_NUMBER_SET_BITS_SIZE (numbits));
  nn_xmsg_submsg_setnext (msg, sm_marker);
  return 0;
}

static void force_heartbeat_to_peer (struct writer *wr, struct proxy_reader *prd)
{
  struct nn_xmsg *m;

  ASSERT_MUTEX_HELD (&wr->e.lock);
  assert (wr->reliable);

  m = nn_xmsg_new (gv.xmsgpool, &wr->e.guid.prefix, 0, NN_XMSG_KIND_CONTROL);
  nn_xmsg_setdstPRD (m, prd);

  if (whc_empty (wr->whc) && !config.respond_to_rti_init_zero_ack_with_invalid_heartbeat)
  {
    /* If WHC is empty, we send a Gap combined with a Heartbeat.  The
       Gap reuses the latest sequence number (or consumes a new one if
       the writer hasn't sent anything yet), therefore for the reader
       it is as-if a Data submessage had once been sent with that
       sequence number and it now receives an unsollicited response to
       a NACK ... */
    unsigned bits = 0;
    os_int64 seq;
    if (wr->seq > 0)
      seq = wr->seq;
    else
    {
      /* never sent anything, pretend we did */
      seq = wr->seq = wr->seq_xmit = 1;
    }
    add_Gap (m, wr, prd, seq, seq+1, 1, &bits);
    add_Heartbeat (m, wr, 0, prd->e.guid.entityid, now (), 1);
    TRACE (("force_heartbeat_to_peer: %x:%x:%x:%x -> %x:%x:%x:%x - whc empty, queueing gap #%lld + heartbeat for transmit\n",
            PGUID (wr->e.guid), PGUID (prd->e.guid), seq));
  }
  else
  {
    /* Send a Heartbeat just to this peer */
    add_Heartbeat (m, wr, 0, prd->e.guid.entityid, now (), 0);
    TRACE (("force_heartbeat_to_peer: %x:%x:%x:%x -> %x:%x:%x:%x - queue for transmit\n",
            PGUID (wr->e.guid), PGUID (prd->e.guid)));
  }
  qxev_msg (wr->evq, m);
}

static os_int64 grow_gap_to_next_seq (const struct writer *wr, os_int64 seq)
{
  os_int64 next_seq = whc_next_seq (wr->whc, seq);
  if (next_seq == MAX_SEQ_NUMBER) /* no next sample */
    return wr->seq_xmit + 1;
  else if (next_seq > wr->seq_xmit)  /* next is beyond last actually transmitted */
    return wr->seq_xmit;
  else /* next one is already visible in the outside world */
    return next_seq;
}

static int acknack_is_nack (const AckNack_t *msg)
{
  unsigned x = 0, mask;
  int i;
  if (msg->readerSNState.numbits == 0)
    /* Disallowed by the spec, but RTI appears to require them (and so
       even we generate them) */
    return 0;
  for (i = 0; i < (int) NN_SEQUENCE_NUMBER_SET_BITS_SIZE (msg->readerSNState.numbits) / 4 - 1; i++)
    x |= msg->readerSNState.bits[i];
  if ((msg->readerSNState.numbits % 32) == 0)
    mask = ~0u;
  else
    mask = ~(~0u >> (msg->readerSNState.numbits % 32));
  x |= msg->readerSNState.bits[i] & mask;
  return x != 0;
}

static int accept_ack_or_hb_w_timeout (nn_count_t new_count, nn_count_t *exp_count, os_int64 tnow, os_int64 *t_last_accepted, int force_accept)
{
  /* AckNacks and Heartbeats with a sequence number (called "count"
     for some reason) equal to or less than the highest one received
     so far must be dropped.  However, we provide an override
     (force_accept) for pre-emptive acks and we accept ones regardless
     of the sequence number after a few seconds.

     This allows continuing after an asymmetrical disconnection if the
     re-connecting side jumps back in its sequence numbering.  DDSI2.1
     8.4.15.7 says: "New HEARTBEATS should have Counts greater than
     all older HEARTBEATs. Then, received HEARTBEATs with Counts not
     greater than any previously received can be ignored."  But it
     isn't clear whether that is about connections or entities, and
     besides there is an issue with the wrap around after 2**31-1.

     This combined procedure should give the best of all worlds, and
     is not more expensive in the common case. */
  const os_int64 timeout = 2 * T_SECOND;

  if (new_count < *exp_count && tnow - *t_last_accepted < timeout && !force_accept)
    return 0;

  *exp_count = new_count + 1;
  *t_last_accepted = tnow;
  return 1;
}

static int handle_AckNack (struct receiver_state *rst, os_int64 tnow, const AckNack_t *msg, nn_ddsi_time_t timestamp)
{
  struct proxy_reader *prd;
  struct wr_prd_match *rn;
  struct writer *wr;
  nn_guid_t src, dst;
  os_int64 seqbase;
  nn_count_t *countp;
  os_int64 gapstart = -1, gapend = -1;
  int gapnumbits = 0;
  unsigned gapbits[256 / 32];
  int accelerate_rexmit = 0;
  int is_pure_ack;
  int is_preemptive_ack;
  int enqueued;
  int numbits;
  int msgs_sent;
  os_int64 max_seq_in_reply;
  int i;
  int hb_sent_in_response = 0;
  memset (gapbits, 0, sizeof (gapbits));
  countp = (nn_count_t *) ((char *) msg + offsetof (AckNack_t, readerSNState) +
                           NN_SEQUENCE_NUMBER_SET_SIZE (msg->readerSNState.numbits));
  src.prefix = rst->src_guid_prefix;
  src.entityid = msg->readerId;
  dst.prefix = rst->dst_guid_prefix;
  dst.entityid = msg->writerId;
  TRACE (("ACKNACK(%s#%d:%lld/%d:", msg->smhdr.flags & ACKNACK_FLAG_FINAL ? "F" : "",
          *countp, fromSN (msg->readerSNState.bitmap_base), msg->readerSNState.numbits));
  for (i = 0; i < (int) msg->readerSNState.numbits; i++)
    TRACE (("%c", nn_bitset_isset (msg->readerSNState.numbits, msg->readerSNState.bits, i) ? '1' : '0'));
  seqbase = fromSN (msg->readerSNState.bitmap_base);

  if ((wr = ephash_lookup_writer_guid (&dst)) == NULL)
  {
    TRACE ((" %x:%x:%x:%x -> %x:%x:%x:%x?)", PGUID (src), PGUID (dst)));
    return 1;
  }
  /* Always look up the proxy reader -- even though we don't need for
     the normal pure ack steady state. If (a big "if"!) this shows up
     as a significant portion of the time, we can always rewrite it to
     only retrieve it when needed. */
  if ((prd = ephash_lookup_proxy_reader_guid (&src)) == NULL)
  {
    TRACE ((" %x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (src), PGUID (dst)));
    return 1;
  }
  if (!wr->reliable) /* note: reliability can't be changed */
  {
    TRACE ((" %x:%x:%x:%x -> %x:%x:%x:%x not a reliable writer!)", PGUID (src), PGUID (dst)));
    return 1;
  }

  os_mutexLock (&wr->e.lock);
  if ((rn = avl_lookup (&wr->readers, &src, NULL)) == NULL)
  {
    TRACE ((" %x:%x:%x:%x -> %x:%x:%x:%x not a connection)", PGUID (src), PGUID (dst)));
    goto out;
  }

  is_pure_ack = !acknack_is_nack (msg);
  is_preemptive_ack = seqbase <= 1 && is_pure_ack;
  if (!accept_ack_or_hb_w_timeout (*countp, &rn->next_acknack, tnow, &rn->t_acknack_accepted, is_preemptive_ack))
  {
    TRACE ((" [%x:%x:%x:%x -> %x:%x:%x:%x])", PGUID (src), PGUID (dst)));
    goto out;
  }
  TRACE ((" %x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst)));

  /* Update latency estimates if we have a timestamp -- won't actually
     work so well if the timestamp can be a left over from some other
     submessage -- but then, it is no more than a quick hack at the
     moment. */
  if (config.meas_hb_to_ack_latency && valid_ddsi_timestamp (timestamp))
  {
    os_int64 tstamp = now ();
    nn_lat_estim_update (&rn->hb_to_ack_latency, tstamp - nn_from_ddsi_time (timestamp));
    if ((config.enabled_logcats & (LC_TRACE | LC_INFO)) &&
        tstamp > rn->hb_to_ack_latency_tlastlog + 10 * T_SECOND)
    {
      if (config.enabled_logcats & LC_TRACE)
        nn_lat_estim_log (LC_TRACE, NULL, &rn->hb_to_ack_latency);
      else if (config.enabled_logcats & LC_INFO)
      {
        char tagbuf[2*(4*8+3) + 4 + 1];
        snprintf (tagbuf, sizeof (tagbuf), "%x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst));
        if (nn_lat_estim_log (LC_INFO, tagbuf, &rn->hb_to_ack_latency))
          nn_log (LC_INFO, "\n");
      }
      rn->hb_to_ack_latency_tlastlog = tstamp;
    }
  }

  /* First, the ACK part: if the AckNack advances the highest sequence
     number ack'd by the remote reader, update state & try dropping
     some messages */
  if (seqbase - 1 > rn->seq)
  {
    os_int64 n_ack = (seqbase - 1) - rn->seq;
    int n;
    rn->seq = seqbase - 1;
    avl_augment_update (&wr->readers, rn);
    n = remove_acked_messages (wr);
    TRACE ((" ACK%lld RM%d", n_ack, n));
  }

  /* If this reader was marked as "non-responsive" in the past, it's now responding again,
     so update its status */
  if (rn->seq == MAX_SEQ_NUMBER && prd->c.xqos->reliability.kind == NN_RELIABLE_RELIABILITY_QOS)
  {
    os_int64 oldest_seq;
    if (whc_empty (wr->whc))
      oldest_seq = wr->seq;
    else
      oldest_seq = whc_max_seq (wr->whc);
    rn->has_replied_to_hb = 1; /* was temporarily cleared to ensure heartbeats went out */
    rn->seq = seqbase - 1;
    if (oldest_seq > rn->seq) {
      /* Prevent a malicious reader from lowering the min. sequence number retained in the WHC. */
      rn->seq = oldest_seq;
    }
    avl_augment_update (&wr->readers, rn);
    NN_WARNING2 ("writer %x:%x:%x:%x considering reader %x:%x:%x:%x responsive again\n", PGUID (wr->e.guid), PGUID (rn->prd_guid));
  }

  /* Second, the NACK bits (literally, that is). To do so, attempt to
     classify the AckNack for reverse-engineered compatibility with
     RTI's invalid acks and sometimes slightly odd behaviour. */
  numbits = (int) msg->readerSNState.numbits;
  msgs_sent = 0;
  max_seq_in_reply = 0;
  if (!rn->has_replied_to_hb && seqbase > 1 && is_pure_ack)
  {
    TRACE ((" setting-has-replied-to-hb"));
    rn->has_replied_to_hb = 1;
    /* walk the whole tree to ensure all proxy readers for this writer
       have their unack'ed info updated */
    avl_augment_update (&wr->readers, rn);
  }
  if (is_preemptive_ack)
  {
    /* Pre-emptive nack: RTI uses (seqbase = 0, numbits = 0), we use
       (seqbase = 1, numbits = 1, bits = {0}).  Seqbase <= 1 and not a
       NACK covers both and is otherwise a useless message.  Sent on
       reader start-up and we respond with a heartbeat and, if we have
       data in our WHC, we start sending it regardless of whether the
       remote reader asked for it */
    TRACE ((" preemptive-nack"));
    if (whc_empty (wr->whc))
    {
      TRACE ((" whc-empty "));
      force_heartbeat_to_peer (wr, prd);
      hb_sent_in_response = 1;
    }
    else
    {
      TRACE ((" rebase "));
      force_heartbeat_to_peer (wr, prd);
      hb_sent_in_response = 1;
      numbits = config.accelerate_rexmit_block_size;
      seqbase = whc_min_seq (wr->whc);
    }
  }
  else if (!rn->assumed_in_sync)
  {
    /* We assume a remote reader that hasn't ever sent a pure Ack --
       an AckNack that doesn't NACK a thing -- is still trying to
       catch up, so we try to accelerate its attempts at catching up
       by a configurable amount. FIXME: what about a pulling reader?
       that doesn't play too nicely with this. */
    if (is_pure_ack)
    {
      TRACE ((" happy-now"));
      rn->assumed_in_sync = 1;
    }
    else if ((int) msg->readerSNState.numbits < config.accelerate_rexmit_block_size)
    {
      TRACE ((" accelerating"));
      accelerate_rexmit = 1;
      if (accelerate_rexmit && numbits < config.accelerate_rexmit_block_size)
        numbits = config.accelerate_rexmit_block_size;
    }
    else
    {
      TRACE ((" complying"));
    }
  }
  /* Retransmit requested messages, including whatever we decided to
     retransmit that the remote reader didn't ask for. While doing so,
     note any gaps in the sequence: if there are some, we transmit a
     Gap message as well.

     Note: ignoring retransmit requests for samples beyond the one we
     last transmitted, even though we may have more available.  If it
     hasn't been transmitted ever, the initial transmit should solve
     that issue; if it has, then the timing is terribly unlucky, but
     a future request'll fix it. */
  enqueued = 1;
  for (i = 0; i < numbits && seqbase + i <= wr->seq_xmit && enqueued; i++)
  {
    /* Accelerated schedule may run ahead of sequence number set
       contained in the acknack, and assumes all messages beyond the
       set are NACK'd -- don't feel like tracking where exactly we
       left off ... */
    if (i >= (int) msg->readerSNState.numbits || nn_bitset_isset (numbits, msg->readerSNState.bits, i))
    {
      os_int64 seq = seqbase + i;
      struct whc_node *whcn;
      if ((whcn = whc_findseq (wr->whc, seq)) != NULL)
      {
        if (config.retransmit_merging != REXMIT_MERGE_NEVER && rn->assumed_in_sync)
        {
          /* send retransmit to all receivers, but skip if recently done */
          os_int64 tstamp = now ();
          if (tstamp > whcn->last_rexmit_ts + config.retransmit_merging_period)
          {
            TRACE ((" RX%lld", seqbase + i));
            enqueued = (enqueue_sample_wrlock_held (wr, seq, whcn->serdata, NULL, 0) >= 0);
            max_seq_in_reply = seqbase + i;
            msgs_sent++;
            whcn->last_rexmit_ts = tstamp;
          }
          else
          {
            TRACE ((" RX%lld (merged)", seqbase + i));
          }
        }
        else
        {
          /* no merging, send directed retransmit */
          TRACE ((" RX%lld", seqbase + i));
          enqueued = (enqueue_sample_wrlock_held (wr, seq, whcn->serdata, prd, 0) >= 0);
          max_seq_in_reply = seqbase + i;
          msgs_sent++;
        }
      }
      else if (gapstart == -1)
      {
        TRACE ((" M%lld", seqbase + i));
        gapstart = seqbase + i;
        gapend = gapstart + 1;
      }
      else if (seqbase + i == gapend)
      {
        TRACE ((" M%lld", seqbase + i));
        gapend = seqbase + i + 1;
      }
      else if (seqbase + i - gapend < 256)
      {
        int idx = (int) (seqbase + i - gapend);
        TRACE ((" M%lld", seqbase + i));
        gapnumbits = idx + 1;
        nn_bitset_set (gapnumbits, gapbits, idx);
      }
    }
  }
  if (!enqueued)
    TRACE ((" rexmit-limit-hit"));
  /* Generate a Gap message if some of the sequence is missing */
  if (gapstart > 0)
  {
    struct nn_xmsg *m;
    if (gapend == seqbase + msg->readerSNState.numbits)
    {
      /* We automatically grow a gap as far as we can -- can't
         retransmit those messages anyway, so no need for round-trip
         to the remote reader. */
      gapend = grow_gap_to_next_seq (wr, gapend);
    }
    if (gapnumbits == 0)
    {
      /* Avoid sending an invalid bitset */
      gapnumbits = 1;
      nn_bitset_set (gapnumbits, gapbits, 0);
      gapend--;
    }
    /* The non-bitmap part of a gap message says everything <=
       gapend-1 is no more (so the maximum sequence number it informs
       the peer of is gapend-1); each bit adds one sequence number to
       that. */
    if (gapend-1 + gapnumbits > max_seq_in_reply)
      max_seq_in_reply = gapend-1 + gapnumbits;
    TRACE ((" XGAP%lld..%lld/%d:", gapstart, gapend, gapnumbits));
    for (i = 0; i < gapnumbits; i++)
      TRACE (("%c", nn_bitset_isset (gapnumbits, gapbits, i) ? '1' : '0'));
    m = nn_xmsg_new (gv.xmsgpool, &wr->e.guid.prefix, 0, NN_XMSG_KIND_CONTROL);
    nn_xmsg_setdstPRD (m, prd);
    add_Gap (m, wr, prd, gapstart, gapend, gapnumbits, gapbits);
    qxev_msg (wr->evq, m);
    msgs_sent++;
  }
  /* If rexmits and/or a gap message were sent, and if the last
     sequence number that we're informing the NACK'ing peer about is
     less than the last sequence number transmitted by the writer,
     tell the peer to acknowledge quickly. Not sure if that helps, but
     it might ... [NB writer->seq is the last msg sent so far] */
  if (msgs_sent && max_seq_in_reply < wr->seq_xmit)
  {
    TRACE ((" rexmit#%d maxseq:%lld<%lld<=%lld", msgs_sent, max_seq_in_reply, wr->seq_xmit, wr->seq));
    force_heartbeat_to_peer (wr, prd);
    hb_sent_in_response = 1;
  }
  /* If "final" flag not set, we must respond with a heartbeat. Do it
     now if we haven't done so already */
  if (!(msg->smhdr.flags & ACKNACK_FLAG_FINAL) && !hb_sent_in_response)
    force_heartbeat_to_peer (wr, prd);
  TRACE ((")"));
 out:
  os_mutexUnlock (&wr->e.lock);
  return 1;
}

static void handle_forall_destinations (const nn_guid_t *dst, struct proxy_writer *pwr, avlwalk_fun_t fun, void *arg)
{
  /* prefix:  id:   to:
     0        0     all matched readers
     0        !=0   all matched readers with entityid id
     !=0      0     to all matched readers in addressed participant
     !=0      !=0   to the one addressed reader
  */
  const int haveprefix =
    !(dst->prefix.u[0] == 0 && dst->prefix.u[1] == 0 && dst->prefix.u[2] == 0);
  const int haveid = !(dst->entityid.u == NN_ENTITYID_UNKNOWN);

  /* must have pwr->e.lock held for safely iterating over readers */
  ASSERT_MUTEX_HELD (&pwr->e.lock);

  switch ((haveprefix << 1) | haveid)
  {
    case (0 << 1) | 0: /* all: full treewalk */
      avl_walk (&pwr->readers, fun, arg);
      break;
    case (0 << 1) | 1: /* all with correct entityid: special filtering treewalk */
      {
        struct pwr_rd_match *wn;
        for (wn = avl_findmin (&pwr->readers); wn; wn = avl_findsucc (&pwr->readers, wn))
        {
          if (wn->rd_guid.entityid.u == dst->entityid.u)
            fun (wn, arg);
        }
      }
      break;
    case (1 << 1) | 0: /* all within one participant: walk a range of keyvalues */
      {
        nn_guid_t a, b;
        a = *dst; a.entityid.u = 0;
        b = *dst; b.entityid.u = ~0;
        avl_walkrange (&pwr->readers, &a, &b, fun, arg);
      }
      break;
    case (1 << 1) | 1: /* fully addressed: dst should exist (but for removal) */
      {
        struct pwr_rd_match *wn;
        if ((wn = avl_lookup (&pwr->readers, dst, NULL)) != NULL)
          fun (wn, arg);
      }
      break;
  }
}

struct handle_Heartbeat_helper_arg {
  struct receiver_state *rst;
  const Heartbeat_t *msg;
  struct proxy_writer *pwr;
  nn_ddsi_time_t timestamp;
  os_int64 tnow;
};

static void handle_Heartbeat_helper (struct pwr_rd_match * const wn, struct handle_Heartbeat_helper_arg * const arg)
{
  Heartbeat_t const * const msg = arg->msg;
  struct proxy_writer * const pwr = arg->pwr;
  const os_int64 tnow = arg->tnow;
  os_int64 refseq;

  ASSERT_MUTEX_HELD (&pwr->e.lock);

  /* Not supposed to respond to repeats and old heartbeats. */
  if (!accept_ack_or_hb_w_timeout (msg->count, &wn->next_heartbeat, tnow, &wn->t_heartbeat_accepted, 0))
  {
    TRACE ((" (%x:%x:%x:%x)", PGUID (wn->rd_guid)));
    return;
  }

  /* Reference sequence number for determining whether or not to
     Ack/Nack unfortunately depends on whether the reader is in
     sync. */
  if (wn->in_sync)
    refseq = nn_reorder_next_seq (pwr->reorder) - 1;
  else
    refseq = nn_reorder_next_seq (wn->u.not_in_sync.reorder) - 1;
  TRACE ((" %x:%x:%x:%x@%lld%s", PGUID (wn->rd_guid), refseq, wn->in_sync ? "(sync)" : ""));

  /* Reschedule AckNack transmit if deemed appropriate; unreliable
     readers have acknack_xevent == NULL and can't do this.

     There is no real need to send a nack from each reader that is in
     sync -- indeed, we could simply ignore the destination address in
     the messages we receive and only ever nack each sequence number
     once, regardless of which readers care about it. */
  if (wn->acknack_xevent)
  {
    if (pwr->last_seq > refseq || !(msg->smhdr.flags & HEARTBEAT_FLAG_FINAL))
    {
      int delay = !(msg->smhdr.flags & HEARTBEAT_FLAG_FINAL) ? 0 : config.nack_delay;
      if (pwr->last_seq > refseq)
        TRACE (("/NAK"));
      if (resched_xevent_if_earlier (wn->acknack_xevent, tnow + delay))
      {
        if (config.meas_hb_to_ack_latency && valid_ddsi_timestamp (arg->timestamp))
          wn->hb_timestamp = nn_from_ddsi_time (arg->timestamp);
      }
    }
    else
    {
      resched_xevent_if_earlier (wn->acknack_xevent, tnow + 7 * T_SECOND);
    }
  }
}

static int handle_Heartbeat (struct receiver_state *rst, os_int64 tnow, struct nn_rmsg *rmsg, const Heartbeat_t *msg, nn_ddsi_time_t timestamp)
{
  /* We now cheat: and process the heartbeat for _all_ readers,
     always, regardless of the destination address in the Heartbeat
     sub-message. This is to take care of the samples with sequence
     numbers that become deliverable because of the heartbeat.

     We do play by the book with respect to generating AckNacks in
     response -- done by handle_Heartbeat_helper.

     A heartbeat that states [a,b] is the smallest interval in which
     the range of available sequence numbers is is interpreted here as
     a gap [1,a). See also handle_Gap.  */
  const os_int64 firstseq = fromSN (msg->firstSN);
  struct handle_Heartbeat_helper_arg arg;
  struct proxy_writer *pwr;
  nn_guid_t src, dst;

  src.prefix = rst->src_guid_prefix;
  src.entityid = msg->writerId;
  dst.prefix = rst->dst_guid_prefix;
  dst.entityid = msg->readerId;

  TRACE (("HEARTBEAT(%s#%d:%lld..%lld ", msg->smhdr.flags & HEARTBEAT_FLAG_FINAL ? "F" : "",
          msg->count, firstseq, fromSN (msg->lastSN)));

  if ((pwr = ephash_lookup_proxy_writer_guid (&src)) == NULL)
  {
    TRACE (("%x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (src), PGUID (dst)));
    return 1;
  }

  TRACE (("%x:%x:%x:%x -> %x:%x:%x:%x:", PGUID (src), PGUID (dst)));

  os_mutexLock (&pwr->e.lock);

  pwr->have_seen_heartbeat = 1;
  if (fromSN (msg->lastSN) > pwr->last_seq)
  {
    pwr->last_seq = fromSN (msg->lastSN);
    pwr->last_fragnum = ~0u;
  }

  nn_defrag_notegap (pwr->defrag, 1, firstseq);

  {
    struct nn_rdata *gap;
    struct pwr_rd_match *wn;
    struct nn_rsample_chain sc;
    int refc_adjust = 0;
    nn_reorder_result_t res;
    gap = nn_rdata_newgap (rmsg);
    if ((res = nn_reorder_gap (&sc, pwr->reorder, gap, 1, firstseq, &refc_adjust)) > 0)
    {
      if (pwr->deliver_synchronously)
        deliver_user_data_synchronously (&sc);
      else
        nn_dqueue_enqueue (pwr->dqueue, &sc, res);
    }
    for (wn = avl_findmin (&pwr->readers); wn; wn = avl_findsucc (&pwr->readers, wn))
      if (!wn->in_sync)
      {
        struct nn_reorder *ro = wn->u.not_in_sync.reorder;
        if ((res = nn_reorder_gap (&sc, ro, gap, 1, firstseq, &refc_adjust)) > 0)
          nn_dqueue_enqueue (pwr->dqueue, &sc, res);
        maybe_set_reader_in_sync (wn);
      }
    nn_fragchain_adjust_refcount (gap, refc_adjust);
  }

  arg.rst = rst;
  arg.msg = msg;
  arg.pwr = pwr;
  arg.timestamp = timestamp;
  arg.tnow = tnow;
  handle_forall_destinations (&dst, pwr, (avlwalk_fun_t) handle_Heartbeat_helper, &arg);
  TRACE ((")"));

  os_mutexUnlock (&pwr->e.lock);
  return 1;
}

static int handle_HeartbeatFrag (struct receiver_state *rst, os_int64 tnow, const HeartbeatFrag_t *msg)
{
  const os_int64 seq = fromSN (msg->writerSN);
  const nn_fragment_number_t fragnum = msg->lastFragmentNum - 1; /* we do 0-based */
  nn_guid_t src, dst;
  struct proxy_writer *pwr;

  src.prefix = rst->src_guid_prefix;
  src.entityid = msg->writerId;
  dst.prefix = rst->dst_guid_prefix;
  dst.entityid = msg->readerId;

  TRACE (("HEARTBEATFRAG(#%d:%lld/[1,%u]", msg->count, seq, fragnum+1));

  if ((pwr = ephash_lookup_proxy_writer_guid (&src)) == NULL)
  {
    TRACE ((" %x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (src), PGUID (dst)));
    return 1;
  }

  TRACE ((" %x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst)));
  os_mutexLock (&pwr->e.lock);

  if (seq > pwr->last_seq)
  {
    pwr->last_seq = seq;
    pwr->last_fragnum = fragnum;
  }
  else if (seq == pwr->last_seq && fragnum > pwr->last_fragnum)
  {
    pwr->last_fragnum = fragnum;
  }

  /* Defragmenting happens at the proxy writer, readers have nothing
     to do with it.  Here we immediately respond with a NackFrag if we
     discover a missing fragment, which differs significantly from
     handle_Heartbeat's scheduling of an AckNack event when it must
     respond.  Why?  Just because. */
  if (avl_empty (&pwr->readers))
    TRACE ((" no readers"));
  else
  {
    /* Pick an arbitrary reliable reader's guid for the response --
       assuming a reliable writer -> unreliable reader is rare, and so
       scanning the readers is acceptable if the first guess fails */
    struct pwr_rd_match *m = pwr->readers.root;
    if (m->acknack_xevent == NULL)
    {
      m = avl_findmin (&pwr->readers);
      while (m && m->acknack_xevent == NULL)
        m = avl_findsucc (&pwr->readers, m);
    }
    if (m == NULL)
      TRACE ((" no reliable readers"));
    else
    {
      /* Check if we are missing something */
      union {
        struct nn_fragment_number_set set;
        char pad[NN_FRAGMENT_NUMBER_SET_SIZE (256)];
      } nackfrag;
      if (nn_defrag_nackmap (pwr->defrag, seq, fragnum, &nackfrag.set, 256) > 0)
      {
        /* Yes we are (note that this potentially also happens for
           samples we no longer care about) */
        os_int64 delay = config.nack_delay;
        TRACE (("/nackfrag"));
        resched_xevent_if_earlier (m->acknack_xevent, tnow + delay);
      }
    }
  }
  TRACE ((")"));
  os_mutexUnlock (&pwr->e.lock);
  return 1;
}

static int handle_NackFrag (struct receiver_state *rst, const NackFrag_t *msg)
{
  struct proxy_reader *prd;
  struct wr_prd_match *rn;
  struct writer *wr;
  struct whc_node *whcn;
  nn_guid_t src, dst;
  nn_count_t *countp;
  os_int64 seq = fromSN (msg->writerSN);
  int i;

  countp = (nn_count_t *) ((char *) msg + offsetof (NackFrag_t, fragmentNumberState) + NN_FRAGMENT_NUMBER_SET_SIZE (msg->fragmentNumberState.numbits));
  src.prefix = rst->src_guid_prefix;
  src.entityid = msg->readerId;
  dst.prefix = rst->dst_guid_prefix;
  dst.entityid = msg->writerId;

  TRACE (("NACKFRAG(#%d:%lld/%u/%d:", *countp, seq, msg->fragmentNumberState.bitmap_base, msg->fragmentNumberState.numbits));
  for (i = 0; i < (int) msg->fragmentNumberState.numbits; i++)
    TRACE (("%c", nn_bitset_isset (msg->fragmentNumberState.numbits, msg->fragmentNumberState.bits, i) ? '1' : '0'));

  if ((wr = ephash_lookup_writer_guid (&dst)) == NULL)
  {
    TRACE ((" %x:%x:%x:%x -> %x:%x:%x:%x?)", PGUID (src), PGUID (dst)));
    return 1;
  }
  /* Always look up the proxy reader -- even though we don't need for
     the normal pure ack steady state. If (a big "if"!) this shows up
     as a significant portion of the time, we can always rewrite it to
     only retrieve it when needed. */
  if ((prd = ephash_lookup_proxy_reader_guid (&src)) == NULL)
  {
    TRACE ((" %x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (src), PGUID (dst)));
    return 1;
  }
  if (!wr->reliable) /* note: reliability can't be changed */
  {
    TRACE ((" %x:%x:%x:%x -> %x:%x:%x:%x not a reliable writer)", PGUID (src), PGUID (dst)));
    return 1;
  }

  os_mutexLock (&wr->e.lock);
  if ((rn = avl_lookup (&wr->readers, &src, NULL)) == NULL)
  {
    TRACE ((" %x:%x:%x:%x -> %x:%x:%x:%x not a connection", PGUID (src), PGUID (dst)));
    goto out;
  }

  /* Ignore old NackFrags (see also handle_AckNack) */
  if (*countp < rn->next_nackfrag)
  {
    TRACE ((" [%x:%x:%x:%x -> %x:%x:%x:%x]", PGUID (src), PGUID (dst)));
    goto out;
  }
  rn->next_nackfrag = *countp + 1;
  TRACE ((" %x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst)));

  /* Resend the requested fragments if we still have the sample, send
     a Gap if we don't have them anymore. */
  if ((whcn = whc_findseq (wr->whc, seq)) == NULL)
  {
    static unsigned zero = 0;
    struct nn_xmsg *m;
    TRACE ((" msg not available: scheduling Gap\n"));
    m = nn_xmsg_new (gv.xmsgpool, &wr->e.guid.prefix, 0, NN_XMSG_KIND_CONTROL);
    nn_xmsg_setdstPRD (m, prd);
    /* length-1 bitmap with the bit clear avoids the illegal case of a
       length-0 bitmap */
    add_Gap (m, wr, prd, seq, seq+1, 1, &zero);
    qxev_msg (wr->evq, m);
  }
  else
  {
    const unsigned base = msg->fragmentNumberState.bitmap_base - 1;
    int enqueued = 1;
    TRACE ((" scheduling requested frags ...\n"));
    for (i = 0; i < (int) msg->fragmentNumberState.numbits && enqueued; i++)
    {
      if (nn_bitset_isset (msg->fragmentNumberState.numbits, msg->fragmentNumberState.bits, i))
      {
        struct nn_xmsg *reply;
        if (create_fragment_message (wr, seq, whcn->serdata, base + i, prd, &reply, 0) < 0)
          enqueued = 0;
        else
          enqueued = (qxev_msg_rexmit_wrlock_held (wr->evq, reply, 0) != NULL);
      }
    }
  }

 out:
  os_mutexUnlock (&wr->e.lock);
  TRACE ((")"));
  return 1;
}

static int handle_InfoDST (struct receiver_state *rst, const InfoDST_t *msg)
{
  rst->dst_guid_prefix = nn_ntoh_guid_prefix (msg->guid_prefix);
  TRACE (("INFODST(%x:%x:%x)", PGUIDPREFIX (rst->dst_guid_prefix)));
  return 1;
}

static int handle_InfoSRC (struct receiver_state *rst, const InfoSRC_t *msg)
{
  rst->src_guid_prefix = nn_ntoh_guid_prefix (msg->guid_prefix);
  rst->protocol_version = msg->version;
  rst->vendor = msg->vendorid;
  TRACE (("INFOSRC(%x:%x:%x vendor %d.%d)",
          PGUIDPREFIX (rst->src_guid_prefix), rst->vendor.id[0], rst->vendor.id[1]));
  return 1;
}

static int handle_InfoTS (const InfoTS_t *msg, nn_ddsi_time_t *timestamp)
{
  TRACE (("INFOTS("));
  if (msg->smhdr.flags & INFOTS_INVALIDATE_FLAG)
  {
    *timestamp = invalid_ddsi_timestamp;
    TRACE (("invalidate"));
  }
  else
  {
    *timestamp = msg->time;
    if (config.enabled_logcats & LC_TRACE)
    {
      os_int64 t = nn_from_ddsi_time (* timestamp);
      TRACE (("%d.%09d", (int) (t / 1000000000), (int) (t % 1000000000)));
    }
  }
  TRACE ((")"));
  return 1;
}

static void handle_one_gap (struct proxy_writer *pwr, struct pwr_rd_match *wn, os_int64 a, os_int64 b, struct nn_rdata *gap, int *refc_adjust)
{
  struct nn_rsample_chain sc;
  nn_reorder_result_t res;
  ASSERT_MUTEX_HELD (&pwr->e.lock);

  /* Clean up the defrag admin: no fragments of a missing sample will
     be arriving in the future */
  nn_defrag_notegap (pwr->defrag, a, b);

  /* Primary reorder: the gap message may cause some samples to become
     deliverable. */
  if ((res = nn_reorder_gap (&sc, pwr->reorder, gap, a, b, refc_adjust)) > 0)
  {
    if (pwr->deliver_synchronously)
      deliver_user_data_synchronously (&sc);
    else
      nn_dqueue_enqueue (pwr->dqueue, &sc, res);
  }

  /* Out-of-sync readers never deal with samples with a sequence
     number beyond end_of_tl_seq -- and so it needn't be bothered
     with gaps that start beyond that number */
  if (!wn->in_sync && a <= wn->u.not_in_sync.end_of_tl_seq)
  {
    if ((res = nn_reorder_gap (&sc, wn->u.not_in_sync.reorder, gap, a, b, refc_adjust)) > 0)
      nn_dqueue_enqueue (pwr->dqueue, &sc, res);

    /* Upon receipt of data a reader can only become in-sync if there
       is something to deliver; for missing data, you just don't know.
       The return value of reorder_gap _is_ sufficiently precise, but
       why not simply check?  It isn't a very expensive test. */
    maybe_set_reader_in_sync (wn);
  }
}

static int handle_Gap (struct receiver_state *rst, struct nn_rmsg *rmsg, const Gap_t *msg)
{
  /* Option 1: Process the Gap for the proxy writer and all
     out-of-sync readers: what do I care which reader is being
     addressed?  Either the sample can still be reproduced by the
     writer, or it can't be anymore.

     Option 2: Process the Gap for the proxy writer and for the
     addressed reader if it happens to be out-of-sync.

     Obviously, both options differ from the specification, but we
     don't have much choice: there is no way of addressing just a
     single in-sync reader, and if that's impossible than we might as
     well ignore the destination completely.

     Option 1 can be fairly expensive if there are many readers, so we
     do option 2. */

  struct proxy_writer *pwr;
  struct pwr_rd_match *wn;
  nn_guid_t src, dst;
  os_int64 gapstart, listbase, last_included_rel;
  int listidx;

  src.prefix = rst->src_guid_prefix;
  src.entityid = msg->writerId;
  dst.prefix = rst->dst_guid_prefix;
  dst.entityid = msg->readerId;
  gapstart = fromSN (msg->gapStart);
  listbase = fromSN (msg->gapList.bitmap_base);
  TRACE (("GAP(%lld..%lld/%d ", gapstart, listbase, msg->gapList.numbits));

  /* There is no _good_ reason for a writer to start the bitmap with a
     1 bit, but check for it just in case, to reduce the number of
     sequence number gaps to be processed. */
  for (listidx = 0; listidx < (int) msg->gapList.numbits; listidx++)
    if (!nn_bitset_isset (msg->gapList.numbits, msg->gapList.bits, listidx))
      break;
  last_included_rel = listidx - 1;

  if ((pwr = ephash_lookup_proxy_writer_guid (&src)) == NULL)
  {
    TRACE (("%x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (src), PGUID (dst)));
    return 1;
  }

  os_mutexLock (&pwr->e.lock);
  if ((wn = avl_lookup (&pwr->readers, &dst, NULL)) == NULL)
  {
    TRACE (("%x:%x:%x:%x -> %x:%x:%x:%x not a connection)", PGUID (src), PGUID (dst)));
    os_mutexUnlock (&pwr->e.lock);
    return 1;
  }
  TRACE (("%x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst)));

  /* Notify reordering in proxy writer & and the addressed reader (if
     it is out-of-sync, &c.), while delivering samples that become
     available because preceding ones are now known to be missing. */
  {
    int refc_adjust = 0;
    struct nn_rdata *gap;
    gap = nn_rdata_newgap (rmsg);
    handle_one_gap (pwr, wn, gapstart, listbase + listidx, gap, &refc_adjust);
    while (listidx < (int) msg->gapList.numbits)
    {
      if (!nn_bitset_isset (msg->gapList.numbits, msg->gapList.bits, listidx))
        listidx++;
      else
      {
        int j;
        for (j = listidx+1; j < (int) msg->gapList.numbits; j++)
          if (!nn_bitset_isset (msg->gapList.numbits, msg->gapList.bits, j))
            break;
        handle_one_gap (pwr, wn, listbase + listidx, listbase + j, gap, &refc_adjust);
        last_included_rel = j - 1;
        listidx = j;
      }
    }
    nn_fragchain_adjust_refcount (gap, refc_adjust);
  }

  /* If the last sequence number explicitly included in the set is
     beyond the last sequence number we know exists, update the
     latter.  Note that a sequence number _not_ included in the set
     doesn't tell us anything (which is something that RTI apparently
     got wrong in its interpetation of pure acks that do include a
     bitmap).  */
  if (listbase + last_included_rel > pwr->last_seq)
  {
    pwr->last_seq = listbase + last_included_rel;
    pwr->last_fragnum = ~0u;
  }
  TRACE ((")"));
  os_mutexUnlock (&pwr->e.lock);
  return 1;
}

static int defragment (char **datap, const struct nn_rdata *fragchain, os_uint32 sz)
{
  if (fragchain->nextfrag == NULL)
  {
    *datap = NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_PAYLOAD_OFF (fragchain));
    return 0;
  }
  else
  {
    char *buf;
    if ((buf = os_malloc (sz)) != NULL)
    {
      os_uint32 off = 0;
      while (fragchain)
      {
        assert (fragchain->min <= off);
        assert (fragchain->maxp1 <= sz);
        if (fragchain->maxp1 > off)
        {
          /* only copy if this fragment adds data */
          const char *payload = NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_PAYLOAD_OFF (fragchain));
          memcpy (buf + off, payload + off - fragchain->min, fragchain->maxp1 - off);
          off = fragchain->maxp1;
        }
        fragchain = fragchain->nextfrag;
      }
    }
    *datap = buf;
    return 1;
  }
}

static v_message extract_vmsg_from_data (const struct nn_rsample_info *sampleinfo, unsigned char data_smhdr_flags, const nn_plist_t *qos, const struct nn_rdata *fragchain, unsigned statusinfo, struct topic const * const topic)
{
  static const nn_guid_t null_guid;
  const char *failmsg = NULL;
  v_message vmsg = NULL;

  if (statusinfo == 0)
  {
    /* normal write */
    char *datap;
    int needs_free;
    if (!(data_smhdr_flags & DATA_FLAG_DATAFLAG) || sampleinfo->size == 0)
    {
      const struct proxy_writer *pwr = sampleinfo->pwr;
      nn_guid_t guid = pwr ? pwr->e.guid : null_guid; /* can't be null _yet_, but that might change some day */
      NN_WARNING6 ("data(application, vendor %d.%d): %x:%x:%x:%x #%lld: write without proper payload (data_smhdr_flags 0x%x size %u)\n",
                   sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
                   PGUID (guid), sampleinfo->seq,
                   data_smhdr_flags, sampleinfo->size);
      return NULL;
    }
    failmsg = "data";
    /* FIXME: defragment should be integrated into deserialize */
    needs_free = defragment (&datap, fragchain, sampleinfo->size);
    vmsg = deserialize (topic, datap, sampleinfo->size);
    if (needs_free) os_free (datap);
  }
  else if (sampleinfo->size)
  {
    /* dispose or unregister with included serialized key or data
       (data is a PrismTech extension) -- i.e., dispose or unregister
       as one would expect to receive */
    char *datap;
    int needs_free;
    needs_free = defragment (&datap, fragchain, sampleinfo->size);
    if (data_smhdr_flags & DATA_FLAG_KEYFLAG)
    {
      failmsg = "key";
      vmsg = deserialize_from_key (topic, datap, sampleinfo->size);
    }
    else
    {
      failmsg = "data";
      assert (data_smhdr_flags & DATA_FLAG_DATAFLAG);
      vmsg = deserialize (topic, datap, sampleinfo->size);
    }
    if (needs_free) os_free (datap);
  }
  else if (data_smhdr_flags & DATA_FLAG_INLINE_QOS)
  {
    /* RTI always tries to make us survive on the keyhash. RTI must
       mend its ways. */
    if (NN_STRICT_P)
      failmsg = "no content";
    else if (!(qos->present & PP_KEYHASH))
      failmsg = "qos present but without keyhash";
    else
    {
      failmsg = "keyhash";
      vmsg = deserialize_from_keyhash (topic, qos->keyhash.value, sizeof (qos->keyhash));
    }
  }
  else
  {
    failmsg = "no content whatsoever";
  }
  if (vmsg == NULL)
  {
    /* No message => error out */
    const struct proxy_writer *pwr = sampleinfo->pwr;
    nn_guid_t guid = pwr ? pwr->e.guid : null_guid; /* can't be null _yet_, but that might change some day */
    NN_WARNING7 ("data(application, vendor %d.%d): %x:%x:%x:%x #%lld: deserialization %s/%s failed (%s)\n",
                 sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
                 PGUID (guid), sampleinfo->seq,
                 topic_name (topic), topic_typename (topic),
                 failmsg ? failmsg : "for reasons unknown");
  }
  return vmsg;
}

static void nn_guid_to_ospl_gid (v_gid *gid, const nn_guid_t *guid)
{
  /* Try hard to fake a writer id for OpenSplice based on a GUID. All
     systems I know of have something resembling a host/system id in
     the first 32 bits, so copy that as the system id and copy half of
     an MD5 hash into the remaining 64 bits. Now if only OpenSplice
     would use all 96 bits as a key, we'd be doing reasonably well ...

     OpenSplice DDSI2 always copies the id using the
     PRISMTECH_WRITER_INFO parameter.

     FIXME: should assign a virtual GID to the proxy writer in
     discovery, and use that. */
  union { os_uint32 u[4]; unsigned char md5[16]; } hash;
  md5_state_t md5st;
  md5_init (&md5st);
  md5_append (&md5st, (unsigned char *) guid, sizeof (*guid));
  md5_finish (&md5st, hash.md5);
  gid->systemId = guid->prefix.u[0];
  gid->localId = hash.u[0];
  gid->serial = hash.u[1];
}

static int do_groupwrite (v_group g, void *vmsg)
{
  v_resendScope resendScope = V_RESEND_NONE; /* resendScope not yet used here beyond this function */
  v_message msg = vmsg;
  v_groupWrite (g, msg, NULL, gv.myNetworkId, &resendScope);
  return 0;
}

static void set_vmsg_header (const struct proxy_writer *pwr, v_message vmsg, const struct nn_prismtech_writer_info *wri, os_int64 tstamp, os_int64 seq, unsigned statusinfo, int have_data)
{
  /* NOTE: pwr need not be locked, provided a QoS change is handled
     with sufficient case ... we don't do QoS changes yet, so we're
     fine.

     FIXME: Eventually, QoS changes will need to be supported, and I
     can imagine that the practical way of doing that would be to
     store "c_keep(pwr->v_message_qos)" in a sampleinfo, and to do
     c_free()+c_new() in the QoS change code.  That would require a
     proper finalisation step for sampleinfo (not so great).  But it
     would then also be an option to allocate a v_message at the time
     the sampleinfo springs to life (which is somewhere deep down in
     the defrag code [nn_defrag_rsample_new()], so beware). */

  vmsg->writeTime.seconds = (c_long) (tstamp / 1000000000);
  vmsg->writeTime.nanoseconds = (c_ulong) (tstamp % 1000000000);
  vmsg->writerGID = wri->writerGID;
  vmsg->writerInstanceGID = wri->writerInstanceGID;
  vmsg->transactionId = wri->transactionId;
  vmsg->sequenceNumber = (c_ulong) seq;
  vmsg->qos = c_keep (pwr->v_message_qos);

  if (have_data)
    /* we now always set L_WRITE if there is real data, detecting a
       write_dispose based on just statusinfo is a bit of a problem
       ... */
    v_stateSet (v_nodeState (vmsg), L_WRITE);

  if (statusinfo == 0)
    assert (have_data);
  else
  {
    if (statusinfo == NN_STATUSINFO_UNREGISTER)
      v_stateSet (v_nodeState (vmsg), L_UNREGISTER);
    if (statusinfo == NN_STATUSINFO_DISPOSE)
      v_stateSet (v_nodeState (vmsg), L_DISPOSED);
  }

  assert ((statusinfo & ~(NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER)) == 0);
}

unsigned char normalize_data_datafrag_flags (const SubmessageHeader_t *smhdr, int datafrag_as_data)
{
  switch ((SubmessageKind_t) smhdr->submessageId)
  {
    case SMID_DATA:
      return smhdr->flags;
    case SMID_DATA_FRAG:
      if (datafrag_as_data)
        return smhdr->flags;
      else
      {
        unsigned char common = smhdr->flags & DATA_FLAG_INLINE_QOS;
        Q_STATIC_ASSERT_CODE (DATA_FLAG_INLINE_QOS == DATAFRAG_FLAG_INLINE_QOS);
        if (smhdr->flags & DATAFRAG_FLAG_KEYFLAG)
          return common | DATA_FLAG_KEYFLAG;
        else
          return common | DATA_FLAG_DATAFLAG;
      }
    default:
      assert (0);
      return 0;
  }
}

static int deliver_user_data (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain)
{
  struct receiver_state const * const rst = sampleinfo->rst;
  struct proxy_writer * const pwr = sampleinfo->pwr;
  struct topic const * const topic = pwr->c.topic;
  unsigned statusinfo;
  Data_DataFrag_common_t *msg;
  unsigned char data_smhdr_flags;
  v_message vmsg = NULL;
  nn_plist_t qos;
  int need_keyhash;

  if (pwr->ddsi2direct_cb)
  {
    pwr->ddsi2direct_cb (sampleinfo, fragchain, pwr->ddsi2direct_cbarg);
    return 0;
  }

  /* NOTE: pwr->e.lock need not be held for correct processing (though
     it may be useful to hold it for maintaining order all the way to
     v_groupWrite): guid is constant, set_vmsg_header() explains about
     the qos issue (and will have to deal with that); and
     pwr->groupset takes care of itself.  FIXME: groupset may be
     taking care of itself, but it is currently doing so in an
     annoyingly simplistic manner ...  */

  /* FIXME: fragments are now handled by copying the message to
     freshly malloced memory (see defragment()) ... that'll have to
     change eventually */
  assert (fragchain->min == 0);
  assert (!is_builtin_entityid (pwr->e.guid.entityid));
  /* Can only get here if at some point readers existed => topic can't
     still be NULL, even if there are no readers at the moment */
  assert (topic != NULL);

  /* Luckily, the Data header (up to inline QoS) is a prefix of the
     DataFrag header, so for the fixed-position things that we're
     interested in here, both can be treated as Data submessages. */
  msg = (Data_DataFrag_common_t *) NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_SUBMSG_OFF (fragchain));
  data_smhdr_flags = normalize_data_datafrag_flags (&msg->smhdr, config.buggy_datafrag_flags_mode);

  /* Extract QoS's to the extent necessary.  The expected case has all
     we need predecoded into a few bits in the sample info.

     If there is no payload, it is either a completely invalid message
     or a dispose/unregister in RTI style.  We assume the latter,
     consequently expect to need the keyhash.  Then, if sampleinfo
     says it is a complex qos, or the keyhash is required, extract all
     we need from the inline qos.

     Complex qos bit also gets set when statusinfo bits other than
     dispose/unregister are set.  They are not currently defined, but
     this may save us if they do get defined one day.  */
  need_keyhash = (sampleinfo->size == 0 || (data_smhdr_flags & (DATA_FLAG_KEYFLAG | DATA_FLAG_DATAFLAG)) == 0);
  if (!(sampleinfo->complex_qos || need_keyhash))
  {
    nn_plist_init_empty (&qos); /* so we can unconditionally call nn_plist_fini() */
    statusinfo = sampleinfo->statusinfo;
  }
  else
  {
    nn_plist_src_t src;
    int qos_offset = NN_RDATA_SUBMSG_OFF (fragchain) + offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->octetsToInlineQos) + msg->octetsToInlineQos;
    src.protocol_version = rst->protocol_version;
    src.vendorid = rst->vendor;
    src.encoding = (msg->smhdr.flags & SMFLAG_ENDIANNESS) ? PL_CDR_LE : PL_CDR_BE;
    src.buf = NN_RMSG_PAYLOADOFF (fragchain->rmsg, qos_offset);
    src.bufsz = NN_RDATA_PAYLOAD_OFF (fragchain) - qos_offset;
    if (nn_plist_init_frommsg (&qos, NULL, PP_STATUSINFO | PP_KEYHASH, 0, &src) < 0)
    {
      NN_WARNING4 ("data(application, vendor %d.%d): %x:%x:%x:%x #%lld: invalid inline qos\n",
                   src.vendorid.id[0], src.vendorid.id[1], PGUID (pwr->e.guid), sampleinfo->seq);
      return 0;
    }
    statusinfo = (qos.present & PP_STATUSINFO) ? qos.statusinfo : 0;
  }

  /* Note: deserializing done potentially many times for a historical
     data sample (once per reader that cares about that data).  For
     now, this is accepted as sufficiently abnormal behaviour to not
     worry about it. */
  if ((vmsg = extract_vmsg_from_data (sampleinfo, data_smhdr_flags, &qos, fragchain, statusinfo, topic)) == NULL)
  {
    nn_plist_fini (&qos);
    return 0;
  }

  /* If tracing, print the full contents */
  if (config.enabled_logcats & LC_TRACE)
  {
    char tmp[1024];
    int tmpsize = sizeof (tmp), res;
    if (data_smhdr_flags & DATA_FLAG_DATAFLAG)
    {
      serdata_t qq = serialize (gv.serpool, topic, vmsg);
      res = prettyprint_serdata (tmp, tmpsize, qq);
      serdata_unref (qq);
    }
    else
    {
      serdata_t qq = serialize_key (gv.serpool, topic, vmsg);
      res = prettyprint_serdata (tmp, tmpsize, qq);
      serdata_unref (qq);
    }
    assert (res >= 0);
    nn_log (LC_TRACE, "data(application, vendor %d.%d): %x:%x:%x:%x #%lld: %s/%s:%s%s\n",
            rst->vendor.id[0], rst->vendor.id[1],
            PGUID (pwr->e.guid), sampleinfo->seq, topic_name (topic), topic_typename (topic),
            tmp, res < tmpsize ? "" : " (trunc)");
  }

  /* Fill in the non-payload part of the v_message */
  {
    nn_prismtech_writer_info_t wri;
    os_int64 tstamp;
    if (NN_SAMPLEINFO_HAS_WRINFO (sampleinfo))
      nn_plist_extract_wrinfo (&wri, sampleinfo, fragchain);
    else
    {
      assert (!vendor_is_prismtech (rst->vendor));
      memset (&wri, 0, sizeof (wri));
      nn_guid_to_ospl_gid (&wri.writerGID, &pwr->e.guid);
    }
    tstamp = valid_ddsi_timestamp (sampleinfo->timestamp) ? nn_from_ddsi_time (sampleinfo->timestamp) : 0;

    set_vmsg_header (pwr, vmsg, &wri, tstamp, sampleinfo->seq, statusinfo, data_smhdr_flags & DATA_FLAG_DATAFLAG);
    TRACE ((" %lld(%p)=>%p\n", sampleinfo->seq, (void *) fragchain, pwr->groups));
    nn_groupset_foreach (pwr->groups, do_groupwrite, vmsg);
    atomic_store_u32 (&pwr->next_deliv_seq_lowword, (os_uint32) (sampleinfo->seq + 1));
  }
  c_free (vmsg);
  nn_plist_fini (&qos);
  return 0;
}

int user_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, UNUSED_ARG (void *qarg))
{
  int res;
  res = deliver_user_data (sampleinfo, fragchain);
  return res;
}

static void deliver_user_data_synchronously (struct nn_rsample_chain *sc)
{
  while (sc->first)
  {
    struct nn_rsample_chain_elem *e = sc->first;
    sc->first = e->next;
    if (e->sampleinfo != NULL)
    {
      /* must not try to deliver a gap -- possibly a FIXME for sample_lost events */
      deliver_user_data (e->sampleinfo, e->fragchain);
    }
    nn_fragchain_unref (e->fragchain);
  }
}

static void clean_defrag (struct proxy_writer *pwr)
{
  os_int64 seq = nn_reorder_next_seq (pwr->reorder);
  struct pwr_rd_match *wn;
  for (wn = avl_findmin (&pwr->readers); wn != NULL; wn = avl_findsucc (&pwr->readers, wn))
  {
    if (!wn->in_sync)
    {
      os_int64 seq1 = nn_reorder_next_seq (wn->u.not_in_sync.reorder);
      if (seq1 < seq)
        seq = seq1;
    }
  }
  nn_defrag_notegap (pwr->defrag, 1, seq);
}

static void handle_regular (struct receiver_state *rst, struct nn_rmsg *rmsg, const Data_DataFrag_common_t *msg, const struct nn_rsample_info *sampleinfo, os_uint32 fragnum, struct nn_rdata *rdata)
{
  struct proxy_writer *pwr;
  struct nn_rsample *rsample;
  nn_guid_t dst;

  dst.prefix = rst->dst_guid_prefix;
  dst.entityid = msg->readerId;

  pwr = sampleinfo->pwr;
  if (pwr == NULL)
  {
    nn_guid_t src;
    src.prefix = rst->src_guid_prefix;
    src.entityid = msg->writerId;
    TRACE ((" %x:%x:%x:%x? -> %x:%x:%x:%x", PGUID (src), PGUID (dst)));
    return;
  }

  /* liveliness is still only implemented partially (with all set to
     AUTOMATIC, BY_PARTICIPANT, &c.), so we simply renew the proxy
     participant's lease. */
  if (config.arrival_of_data_asserts_pp_and_ep_liveliness)
  {
    lease_renew (pwr->c.proxypp->lease, sampleinfo->reception_timestamp);
  }

  /* Shouldn't lock the full writer, but will do so for now */
  os_mutexLock (&pwr->e.lock);
  if (avl_empty (&pwr->readers))
  {
    os_mutexUnlock (&pwr->e.lock);
    TRACE ((" %x:%x:%x:%x -> %x:%x:%x:%x: no readers", PGUID (pwr->e.guid), PGUID (dst)));
    return;
  }

  /* Track highest sequence number we know of -- we track both
     sequence number & fragment number so that the NACK generation can
     do the Right Thing. */
  if (sampleinfo->seq > pwr->last_seq)
  {
    pwr->last_seq = sampleinfo->seq;
    pwr->last_fragnum = fragnum;
  }
  else if (sampleinfo->seq == pwr->last_seq && fragnum > pwr->last_fragnum)
  {
    pwr->last_fragnum = fragnum;
  }

  clean_defrag (pwr);

  if ((rsample = nn_defrag_rsample (pwr->defrag, rdata, sampleinfo)) != NULL)
  {
    int refc_adjust = 0;
    struct nn_rsample_chain sc;
    struct nn_rdata *fragchain = nn_rsample_fragchain (rsample);
    nn_reorder_result_t rres;

    rres = nn_reorder_rsample (&sc, pwr->reorder, rsample, &refc_adjust, nn_dqueue_is_full (pwr->dqueue));

    if (rres == NN_REORDER_ACCEPT && pwr->n_reliable_readers == 0)
    {
      /* If no reliable readers but the reorder buffer accepted the
         sample, it must be a reliable proxy writer with only
         unreliable readers.  "Inserting" a Gap [1, sampleinfo->seq)
         will force delivery of this sample, and not cause the gap to
         be added to the reorder admin. */
      int gap_refc_adjust = 0;
      rres = nn_reorder_gap (&sc, pwr->reorder, rdata, 1, sampleinfo->seq, &gap_refc_adjust);
      assert (rres > 0);
      assert (gap_refc_adjust == 0);
    }

    if (rres > 0)
    {
      /* Enqueue or deliver with pwr->e.lock held: to ensure no other
         receive thread's data gets interleaved -- arguably delivery
         needn't be exactly in-order, which would allow us to do this
         without pwr->e.lock held. */
      if (!pwr->deliver_synchronously)
        nn_dqueue_enqueue (pwr->dqueue, &sc, rres);
      else
        deliver_user_data_synchronously (&sc);
    }
    else if (rres == NN_REORDER_TOO_OLD)
    {
      struct pwr_rd_match *wn;
      struct nn_rsample *rsample_dup = NULL;
      int reuse_rsample_dup = 0;
      for (wn = avl_findmin (&pwr->readers); wn != NULL; wn = avl_findsucc (&pwr->readers, wn))
      {
        nn_reorder_result_t rres2;
        if (wn->in_sync || sampleinfo->seq > wn->u.not_in_sync.end_of_tl_seq)
          continue;
        if (!reuse_rsample_dup)
          rsample_dup = nn_reorder_rsample_dup (rmsg, rsample);
        rres2 = nn_reorder_rsample (&sc, wn->u.not_in_sync.reorder, rsample_dup, &refc_adjust, nn_dqueue_is_full (pwr->dqueue));
        switch (rres2)
        {
          case NN_REORDER_TOO_OLD:
          case NN_REORDER_REJECT:
            reuse_rsample_dup = 1;
            break;
          case NN_REORDER_ACCEPT:
            reuse_rsample_dup = 0;
            break;
          default:
            assert (rres2 > 0);
            /* note: can't deliver to a reader, only to a group */
            maybe_set_reader_in_sync (wn);
            reuse_rsample_dup = 0;
            /* No need to deliver old data to out-of-sync readers
               synchronously -- ordering guarantees don't change
               as fresh data will be delivered anyway and hence
               the old data will never be guaranteed to arrive
               in-order, and those few microseconds can't hurt in
               catching up on transient-local data.  See also
               NN_REORDER_DELIVER case in outer switch. */
            nn_dqueue_enqueue (pwr->dqueue, &sc, rres2);
            break;
        }
      }
    }
#ifndef NDEBUG
    else
    {
      assert (rres == NN_REORDER_ACCEPT || rres == NN_REORDER_REJECT);
    }
#endif
    nn_fragchain_adjust_refcount (fragchain, refc_adjust);
  }
  os_mutexUnlock (&pwr->e.lock);
}

static int handle_SPDP (const struct nn_rsample_info *sampleinfo, struct nn_rdata *rdata)
{
  struct nn_rsample *rsample;
  struct nn_rsample_chain sc;
  struct nn_rdata *fragchain;
  nn_reorder_result_t rres;
  int refc_adjust = 0;
  os_mutexLock (&gv.spdp_lock);
  rsample = nn_defrag_rsample (gv.spdp_defrag, rdata, sampleinfo);
  fragchain = nn_rsample_fragchain (rsample);
  if ((rres = nn_reorder_rsample (&sc, gv.spdp_reorder, rsample, &refc_adjust, nn_dqueue_is_full (gv.builtins_dqueue))) > 0)
    nn_dqueue_enqueue (gv.builtins_dqueue, &sc, rres);
  os_mutexUnlock (&gv.spdp_lock);
  nn_fragchain_adjust_refcount (fragchain, refc_adjust);
  return 0;
}

static int handle_Data (struct receiver_state *rst, struct nn_rmsg *rmsg, const Data_t *msg, int size, struct nn_rsample_info *sampleinfo, char *datap)
{
  struct nn_rdata *rdata;
  int submsg_offset, payload_offset;

  TRACE (("DATA(%x:%x:%x:%x -> %x:%x:%x:%x #%lld",
          PGUIDPREFIX (rst->src_guid_prefix), msg->x.writerId.u,
          PGUIDPREFIX (rst->dst_guid_prefix), msg->x.readerId.u,
          fromSN (msg->x.writerSN)));

  submsg_offset = (char *) msg - NN_RMSG_PAYLOAD (rmsg);
  if (datap)
    payload_offset = (char *) datap - NN_RMSG_PAYLOAD (rmsg);
  else
    payload_offset = submsg_offset + size;
  rdata = nn_rdata_new (rmsg, 0, sampleinfo->size, submsg_offset, payload_offset);

  if (msg->x.writerId.u == NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER)
    /* SPDP needs special treatment: there are no proxy writers for it
       and we accept data from unknown sources */
    handle_SPDP (sampleinfo, rdata);
  else
    handle_regular (rst, rmsg, &msg->x, sampleinfo, ~0u, rdata);
  TRACE ((")"));
  return 1;
}

static int handle_DataFrag (struct receiver_state *rst, struct nn_rmsg *rmsg, const DataFrag_t *msg, int size, struct nn_rsample_info *sampleinfo, char *datap)
{
  struct nn_rdata *rdata;
  int submsg_offset, payload_offset;
  os_uint32 begin, endp1;

  TRACE (("DATAFRAG(%x:%x:%x:%x -> %x:%x:%x:%x #%lld/[%u..%u]",
          PGUIDPREFIX (rst->src_guid_prefix), msg->x.writerId.u,
          PGUIDPREFIX (rst->dst_guid_prefix), msg->x.readerId.u,
          fromSN (msg->x.writerSN),
          msg->fragmentStartingNum, msg->fragmentStartingNum + msg->fragmentsInSubmessage - 1));

  if (is_builtin_entityid (msg->x.writerId))
  {
    NN_WARNING5 ("DATAFRAG(%x:%x:%x:%x #%lld -> %x:%x:%x:%x) - fragmented builtin data not yet supported\n",
                 PGUIDPREFIX (rst->src_guid_prefix), msg->x.writerId.u, fromSN (msg->x.writerSN),
                 PGUIDPREFIX (rst->dst_guid_prefix), msg->x.readerId.u);
    return 1;
  }

  submsg_offset = (char *) msg - NN_RMSG_PAYLOAD (rmsg);
  if (datap)
    payload_offset = (char *) datap - NN_RMSG_PAYLOAD (rmsg);
  else
    payload_offset = submsg_offset + size;

  begin = (unsigned) (msg->fragmentStartingNum - 1) * msg->fragmentSize;
  if (msg->fragmentSize * msg->fragmentsInSubmessage > ((char *) msg + size - datap)) {
    /* this happens for the last fragment (which usually is short) --
       and is included here merely as a sanity check, because that
       would mean the computed endp1'd be larger than the sample
       size */
    endp1 = begin + ((char *) msg + size - datap);
  } else {
    /* most of the time we get here, but this differs from the
       preceding only when the fragment size is not a multiple of 4
       whereas all the length of CDR data always is (and even then,
       you'd be fine as the defragmenter can deal with partially
       overlapping fragments ...) */
    endp1 = begin + msg->fragmentSize * msg->fragmentsInSubmessage;
  }
  if (endp1 > msg->sampleSize)
  {
    /* the sample size need not be a multiple of 4 so we can still get
       here */
    endp1 = msg->sampleSize;
  }
  TRACE (("/[%u..%u) of %u", begin, endp1, msg->sampleSize));

  rdata = nn_rdata_new (rmsg, begin, endp1, submsg_offset, payload_offset);

  /* Fragment numbers in DDSI2 internal representation are 0-based,
     whereas in DDSI they are 1-based.  The highest fragment number in
     the sample in internal representation is therefore START+CNT-2,
     rather than the expect START+CNT-1.  Nothing will go terribly
     wrong, it'll simply generate a request for retransmitting a
     non-existent fragment.  The other side SHOULD be capable of
     dealing with that. */
  handle_regular (rst, rmsg, &msg->x, sampleinfo, msg->fragmentStartingNum + msg->fragmentsInSubmessage - 2, rdata);
  TRACE ((")"));
  return 1;
}


static void malformed_packet_received_nosubmsg (const char *msg, int len, const char *state, nn_vendorid_t vendorid)
{
  char tmp[1024];
  int i, pos;

  /* Show beginning of message (as hex dumps) */
  pos = snprintf (tmp, sizeof (tmp), "malformed packet received from vendor %d.%d state %s <", vendorid.id[0], vendorid.id[1], state);
  for (i = 0; i < 32 && i < len; i++)
    pos += snprintf (tmp + pos, sizeof (tmp) - pos, "%s%02x", (i > 0 && (i%4) == 0) ? " " : "", (unsigned char) msg[i]);
  pos += snprintf (tmp + pos, sizeof (tmp) - pos, "> (note: maybe partially bswap'd)");
  assert (pos < (int) sizeof (tmp));
  NN_WARNING1 ("%s\n", tmp);
}

static void malformed_packet_received (const char *msg, const char *submsg, int len, const char *state, SubmessageKind_t smkind, nn_vendorid_t vendorid)
{
  char tmp[1024];
  int i, pos;
  unsigned smsize;
  assert (submsg >= msg && submsg < msg + len);

  /* Show beginning of message and of submessage (as hex dumps) */
  pos = snprintf (tmp, sizeof (tmp), "malformed packet received from vendor %d.%d state %s <", vendorid.id[0], vendorid.id[1], state);
  for (i = 0; i < 32 && i < len && msg + i < submsg; i++)
    pos += snprintf (tmp + pos, sizeof (tmp) - pos, "%s%02x", (i > 0 && (i%4) == 0) ? " " : "", (unsigned char) msg[i]);
  pos += snprintf (tmp + pos, sizeof (tmp) - pos, " @0x%x ", (int) (submsg - msg));
  for (i = 0; i < 32 && i < len - (int) (submsg - msg); i++)
    pos += snprintf (tmp + pos, sizeof (tmp) - pos, "%s%02x", (i > 0 && (i%4) == 0) ? " " : "", (unsigned char) submsg[i]);
  pos += snprintf (tmp + pos, sizeof (tmp) - pos, "> (note: maybe partially bswap'd)");
  assert (pos < (int) sizeof (tmp));

  /* Partially decode header if we have enough bytes available */
  smsize = (unsigned) (len - (int) (submsg - msg));
  switch (smkind)
  {
    case SMID_ACKNACK:
      if (smsize >= sizeof (AckNack_t))
      {
        const AckNack_t *x = (const AckNack_t *) submsg;
        (void) snprintf (tmp + pos, sizeof (tmp) - pos, " {{%x,%x,%d},%x,%x,%lld,%u}",
                         x->smhdr.submessageId, x->smhdr.flags, x->smhdr.octetsToNextHeader,
                         x->readerId.u, x->writerId.u, fromSN (x->readerSNState.bitmap_base),
                         x->readerSNState.numbits);
      }
      break;
    case SMID_HEARTBEAT:
      if (smsize >= sizeof (Heartbeat_t))
      {
        const Heartbeat_t *x = (const Heartbeat_t *) submsg;
        (void) snprintf (tmp + pos, sizeof (tmp) - pos, " {{%x,%x,%d},%x,%x,%lld,%lld}",
                         x->smhdr.submessageId, x->smhdr.flags, x->smhdr.octetsToNextHeader,
                         x->readerId.u, x->writerId.u, fromSN (x->firstSN), fromSN (x->lastSN));
      }
      break;
    case SMID_GAP:
      if (smsize >= sizeof (Gap_t))
      {
        const Gap_t *x = (const Gap_t *) submsg;
        (void) snprintf (tmp + pos, sizeof (tmp) - pos, " {{%x,%x,%d},%x,%x,%lld,%lld,%u}",
                         x->smhdr.submessageId, x->smhdr.flags, x->smhdr.octetsToNextHeader,
                         x->readerId.u, x->writerId.u, fromSN (x->gapStart),
                         fromSN (x->gapList.bitmap_base), x->gapList.numbits);
      }
      break;
    case SMID_NACK_FRAG:
      if (smsize >= sizeof (NackFrag_t))
      {
        const NackFrag_t *x = (const NackFrag_t *) submsg;
        (void) snprintf (tmp + pos, sizeof (tmp) - pos, " {{%x,%x,%d},%x,%x,%lld,%u,%u}",
                         x->smhdr.submessageId, x->smhdr.flags, x->smhdr.octetsToNextHeader,
                         x->readerId.u, x->writerId.u, fromSN (x->writerSN),
                         x->fragmentNumberState.bitmap_base, x->fragmentNumberState.numbits);
      }
      break;
    case SMID_HEARTBEAT_FRAG:
      if (smsize >= sizeof (HeartbeatFrag_t))
      {
        const HeartbeatFrag_t *x = (const HeartbeatFrag_t *) submsg;
        (void) snprintf (tmp + pos, sizeof (tmp) - pos, " {{%x,%x,%d},%x,%x,%lld,%u}",
                         x->smhdr.submessageId, x->smhdr.flags, x->smhdr.octetsToNextHeader,
                         x->readerId.u, x->writerId.u, fromSN (x->writerSN),
                         x->lastFragmentNum);
      }
      break;
    case SMID_DATA:
      if (smsize >= sizeof (Data_t))
      {
        const Data_t *x = (const Data_t *) submsg;
        (void) snprintf (tmp + pos, sizeof (tmp) - pos, " {{%x,%x,%d},%x,%d,%x,%x,%lld}",
                         x->x.smhdr.submessageId, x->x.smhdr.flags, x->x.smhdr.octetsToNextHeader,
                         x->x.extraFlags, x->x.octetsToInlineQos,
                         x->x.readerId.u, x->x.writerId.u, fromSN (x->x.writerSN));
      }
      break;
    case SMID_DATA_FRAG:
      if (smsize >= sizeof (DataFrag_t))
      {
        const DataFrag_t *x = (const DataFrag_t *) submsg;
        (void) snprintf (tmp + pos, sizeof (tmp) - pos, " {{%x,%x,%d},%x,%d,%x,%x,%lld,%u,%u,%u,%u}",
                         x->x.smhdr.submessageId, x->x.smhdr.flags, x->x.smhdr.octetsToNextHeader,
                         x->x.extraFlags, x->x.octetsToInlineQos,
                         x->x.readerId.u, x->x.writerId.u, fromSN (x->x.writerSN),
                         x->fragmentStartingNum, x->fragmentsInSubmessage, x->fragmentSize, x->sampleSize);
      }
      break;
    default:
      break;
  }

  NN_WARNING1 ("%s\n", tmp);
}

static struct receiver_state *rst_cow_if_needed (int *rst_live, struct nn_rmsg *rmsg, struct receiver_state *rst)
{
  if (! *rst_live)
    return rst;
  else
  {
    struct receiver_state *nrst = nn_rmsg_alloc (rmsg, sizeof (*nrst));
    *nrst = *rst;
    *rst_live = 0;
    return nrst;
  }
}

static int handle_submsg_sequence (struct thread_state1 * const self, os_int64 tnow, const nn_guid_prefix_t * const src_prefix, const nn_guid_prefix_t * const dst_prefix, char * const msg /* NOT const - we may byteswap it */, const int len, char *submsg /* aliases somewhere in msg */, struct nn_rmsg * const rmsg)
{
  const char *state;
  SubmessageKind_t state_smkind;
  Header_t *hdr = (Header_t *) msg;
  struct receiver_state *rst;
  int rst_live, ts_for_latmeas;
  nn_ddsi_time_t timestamp;
  int submsg_size;

  /* Receiver state is dynamically allocated with lifetime bound to
     the message.  Updates cause a new copy to be created if the
     current one is "live", i.e., possibly referenced by a
     submessage (for now, only Data(Frag)). */
  rst = nn_rmsg_alloc (rmsg, sizeof (*rst));
  memset (rst, 0, sizeof (*rst));
  rst->src_guid_prefix = *src_prefix;
  rst->dst_guid_prefix = *dst_prefix;
  rst->vendor = hdr->vendorid;
  rst->protocol_version = hdr->version;
  rst_live = 0;
  ts_for_latmeas = 0;
  timestamp = invalid_ddsi_timestamp;

  submsg_size = 0; /* so "short" error message has defined inputs */
  while (submsg <= msg + len - sizeof (SubmessageHeader_t))
  {
    Submessage_t *sm = (Submessage_t *) submsg;
    int byteswap;
    int octetsToNextHeader;

    state = "parse";
    state_smkind = SMID_PAD;

    if (sm->smhdr.flags & SMFLAG_ENDIANNESS)
      byteswap = ! PLATFORM_IS_LITTLE_ENDIAN;
    else
      byteswap = PLATFORM_IS_LITTLE_ENDIAN;
    if (byteswap)
    {
      sm->smhdr.octetsToNextHeader = bswap2u (sm->smhdr.octetsToNextHeader);
    }

    octetsToNextHeader = sm->smhdr.octetsToNextHeader;
    if (octetsToNextHeader != 0)
      submsg_size = RTPS_SUBMESSAGE_HEADER_SIZE + octetsToNextHeader;
    else if (sm->smhdr.submessageId == SMID_PAD ||
             sm->smhdr.submessageId == SMID_INFO_TS)
      submsg_size = RTPS_SUBMESSAGE_HEADER_SIZE;
    else
      submsg_size = msg + len - submsg;
    /*LC_TRACE (("submsg_size %d\n", submsg_size));*/

    if (submsg + submsg_size > msg + len)
    {
      TRACE ((" BREAK (%u %d: %u %d) \n", submsg, submsg_size, msg, len));
      break;
    }

    thread_state_awake (self);
    state_smkind = sm->smhdr.submessageId;
    switch (sm->smhdr.submessageId)
    {
      case SMID_PAD:
        TRACE (("PAD"));
        break;
      case SMID_ACKNACK:
        state = "parse:acknack";
        if (!valid_AckNack (&sm->acknack, submsg_size, byteswap))
          goto malformed;
        handle_AckNack (rst, tnow, &sm->acknack, ts_for_latmeas ? timestamp : invalid_ddsi_timestamp);
        ts_for_latmeas = 0;
        break;
      case SMID_HEARTBEAT:
        state = "parse:heartbeat";
        if (!valid_Heartbeat (&sm->heartbeat, submsg_size, byteswap))
          goto malformed;
        handle_Heartbeat (rst, tnow, rmsg, &sm->heartbeat, ts_for_latmeas ? timestamp : invalid_ddsi_timestamp);
        ts_for_latmeas = 0;
        break;
      case SMID_GAP:
        state = "parse:gap";
        /* Gap is handled synchronously in principle, but may
           sometimes have to record a gap in the reorder admin.  The
           first case by definition doesn't need to set "rst_live",
           the second one avoids that because it doesn't require the
           rst after inserting the gap in the admin. */
        if (!valid_Gap (&sm->gap, submsg_size, byteswap))
          goto malformed;
        handle_Gap (rst, rmsg, &sm->gap);
        ts_for_latmeas = 0;
        break;
      case SMID_INFO_TS:
        state = "parse:info_ts";
        if (!valid_InfoTS (&sm->infots, submsg_size, byteswap))
          goto malformed;
        handle_InfoTS (&sm->infots, &timestamp);
        ts_for_latmeas = 1;
        break;
      case SMID_INFO_SRC:
        state = "parse:info_src";
        if (!valid_InfoSRC (&sm->infosrc, submsg_size, byteswap))
          goto malformed;
        rst = rst_cow_if_needed (&rst_live, rmsg, rst);
        handle_InfoSRC (rst, &sm->infosrc);
        /* no effect on ts_for_latmeas */
        break;
      case SMID_INFO_REPLY_IP4:
        state = "parse:info_reply_ip4";
        TRACE (("INFO_REPLY_IP4"));
        /* no effect on ts_for_latmeas */
        break;
      case SMID_INFO_DST:
        state = "parse:info_dst";
        if (!valid_InfoDST (&sm->infodst, submsg_size, byteswap))
          goto malformed;
        rst = rst_cow_if_needed (&rst_live, rmsg, rst);
        handle_InfoDST (rst, &sm->infodst);
        /* no effect on ts_for_latmeas */
        break;
      case SMID_INFO_REPLY:
        state = "parse:info_reply";
        TRACE (("INFO_REPLY"));
        /* no effect on ts_for_latmeas */
        break;
      case SMID_NACK_FRAG:
        state = "parse:nackfrag";
        if (!valid_NackFrag (&sm->nackfrag, submsg_size, byteswap))
          goto malformed;
        handle_NackFrag (rst, &sm->nackfrag);
        ts_for_latmeas = 0;
        break;
      case SMID_HEARTBEAT_FRAG:
        state = "parse:heartbeatfrag";
        if (!valid_HeartbeatFrag (&sm->heartbeatfrag, submsg_size, byteswap))
          goto malformed;
        handle_HeartbeatFrag (rst, tnow, &sm->heartbeatfrag);
        ts_for_latmeas = 0;
        break;
      case SMID_DATA_FRAG:
        state = "parse:datafrag";
        {
          struct nn_rsample_info sampleinfo;
          char *datap;
          /* valid_DataFrag does not validate the payload */
          if (!valid_DataFrag (rst, rmsg, &sm->datafrag, submsg_size, byteswap, &sampleinfo, &datap))
            goto malformed;
          sampleinfo.timestamp = timestamp;
          sampleinfo.reception_timestamp = tnow;
          handle_DataFrag (rst, rmsg, &sm->datafrag, submsg_size, &sampleinfo, datap);
          rst_live = 1;
          ts_for_latmeas = 0;
        }
        break;
      case SMID_DATA:
        state = "parse:data";
        {
          struct nn_rsample_info sampleinfo;
          char *datap;
          /* valid_Data does not validate the payload */
          if (!valid_Data (rst, rmsg, &sm->data, submsg_size, byteswap, &sampleinfo, &datap))
            goto malformed;
          sampleinfo.timestamp = timestamp;
          sampleinfo.reception_timestamp = tnow;
          handle_Data (rst, rmsg, &sm->data, submsg_size, &sampleinfo, datap);
          rst_live = 1;
          ts_for_latmeas = 0;
        }
        break;

      case SMID_PT_INFO_CONTAINER:
        if (is_own_vendor (rst->vendor))
        {
          state = "parse:pt_info_container";
          TRACE (("PT_INFO_CONTAINER("));
          if (!valid_PT_InfoContainer (&sm->pt_infocontainer, submsg_size, byteswap))
            goto malformed;
          switch (sm->pt_infocontainer.id)
          {
            case PTINFO_ID_ENCRYPT:
              break;
            default:
              TRACE (("(unknown id %u?)\n"));
          }
        }
        break;

      default:
        state = "parse:undefined";
        TRACE (("UNDEFINED(%x)", sm->smhdr.submessageId));
        if (sm->smhdr.submessageId <= 0x7f)
        {
          /* Other submessages in the 0 .. 0x7f range may be added in
             future version of the protocol -- so an undefined code
             for the implemented version of the protocol indicates a
             malformed message. */
          if (rst->protocol_version.major < RTPS_MAJOR ||
              (rst->protocol_version.major == RTPS_MAJOR &&
               rst->protocol_version.minor <= RTPS_MINOR))
            goto malformed;
        }
        else if (is_own_vendor (rst->vendor))
        {
          /* One wouldn't expect undefined stuff from ourselves,
             except that we need to be up- and backwards compatible
             with ourselves, too! */
#if 0
          goto malformed;
#endif
        }
        else
        {
          /* Ignore other vendors' private submessages */
        }
        ts_for_latmeas = 0;
        break;
    }
    submsg += submsg_size;
    TRACE (("\n"));
  }
  if (submsg != msg + len)
  {
    state = "parse:shortmsg";
    state_smkind = SMID_PAD;
    TRACE (("short (size %d exp %d act %d)", submsg_size, submsg, msg + len));
    goto malformed;
  }
  return 0;

 malformed:
  malformed_packet_received (msg, submsg, len, state, state_smkind, hdr->vendorid);
  return -1;
}

static void handle_message (struct thread_state1 * const self, os_int64 tnow, const nn_guid_prefix_t * const dst_prefix, char * const msg /* NOT const - we may byteswap it */, const int len, struct nn_rmsg * const rmsg)
{
  Header_t *hdr = (Header_t *) msg;

  if (len < (int) RTPS_MESSAGE_HEADER_SIZE ||
      hdr->protocol.id[0] != 'R' || hdr->protocol.id[1] != 'T' ||
      hdr->protocol.id[2] != 'P' || hdr->protocol.id[3] != 'S' ||
      hdr->version.major != RTPS_MAJOR || hdr->version.minor != RTPS_MINOR)
  {
    malformed_packet_received_nosubmsg (msg, len, "header", hdr->vendorid);
    return;
  }

  hdr->guid_prefix = nn_ntoh_guid_prefix (hdr->guid_prefix);

  TRACE (("HDR(%x:%x:%x vendor %d.%d) len %d\n",
          PGUIDPREFIX (hdr->guid_prefix), hdr->vendorid.id[0], hdr->vendorid.id[1], len));
  if (config.coexistWithNativeNetworking && is_own_vendor (hdr->vendorid))
    ; /* ignore */
  else
  {
    char *submsg = msg + RTPS_MESSAGE_HEADER_SIZE;
    handle_submsg_sequence (self, tnow, &hdr->guid_prefix, dst_prefix, msg, len, submsg, rmsg);
  }
}

static void do_packet (struct thread_state1 *self, os_socket fd, const nn_guid_prefix_t *guidprefix, struct nn_rbufpool *rbpool)
{
  /* UDP max packet size is 64kB */
  const int maxsz = config.rmsg_chunk_size < 65536 ? config.rmsg_chunk_size : 65536;
  int sz;
  struct sockaddr_storage src;
  os_uint32 srclen = sizeof (src);
  struct nn_rmsg *rmsg;
  struct msghdr msghdr;
  struct iovec msg_iov;

  if ((rmsg = nn_rmsg_new (rbpool)) != NULL)
  {
    msg_iov.iov_base = NN_RMSG_PAYLOAD (rmsg);
    msg_iov.iov_len = maxsz;
  }
  else
  {
    /* Failed to claim memory for a message, so drop the packet;
       staticbuf is used because I can imagine (NULL, 0) being
       unacceptable to some kernels. */
    static char staticbuf[1];
    msg_iov.iov_base = staticbuf;
    msg_iov.iov_len = sizeof (staticbuf);
  }
  msghdr.msg_name = &src;
  msghdr.msg_namelen = srclen;
  msghdr.msg_iov = &msg_iov;
  msghdr.msg_iovlen = 1;
#if SYSDEPS_MSGHDR_FLAGS
  /* Turns out VxWorks 6.x wants msg_flags cleared, even though it is
     spec'd as an out parameter in POSIX */
  msghdr.msg_flags = 0;
#endif
#if SYSDEPS_MSGHDR_ACCRIGHTS
  msghdr.msg_accrights = NULL;
  msghdr.msg_accrightslen = 0;
#else
  msghdr.msg_control = NULL;
  msghdr.msg_controllen = 0;
#endif
  if ((sz = recvmsg (fd, &msghdr, 0)) < 0)
  {
    int err = os_sockError ();
    if (err != os_sockENOTSOCK && err != os_sockEINTR && err != os_sockEBADF && err != os_sockECONNRESET)
      NN_ERROR4 ("recvfrom sock %d: %d errno %d%s\n", fd, sz, err, rmsg ? "" : " and out of receive buffer space");
  }
  else
  {
    char addrbuf[INET6_ADDRSTRLEN_EXTENDED];

#if SYSDEPS_MSGHDR_FLAGS
    if (msghdr.msg_flags & MSG_TRUNC)
    {
      sockaddr_to_string_with_port (addrbuf, &src);
      NN_WARNING2 ("%s => truncated%s\n", addrbuf, rmsg ? "" : " - likely cause: out of receive buffer space");
    }
    else
    {
#endif
       if (rmsg == NULL)
       {
          sockaddr_to_string_with_port (addrbuf, &src);
          NN_WARNING1 ("%s => dropped: out of receive buffer space\n", addrbuf);
       }
       else
       {
          os_int64 tnow;

          nn_rmsg_setsize (rmsg, sz);
          assert (vtime_asleep_p (self->vtime));

          tnow = now ();
          if (config.enabled_logcats & LC_TRACE)
          {
             nn_log_set_tstamp (tnow);
             sockaddr_to_string_with_port (addrbuf, &src);
             TRACE (("%s => ", addrbuf));
          }
          if (gv.pcap_fp)
          {
             os_sockaddr_storage dstaddr;
             socklen_t dstaddrlen = sizeof (dstaddr);
             (void) getsockname (fd, (struct sockaddr *) &dstaddr, &dstaddrlen);
             write_pcap_received (gv.pcap_fp, tnow, &dstaddr, &msghdr, sz);
          }

          handle_message (self, tnow, guidprefix, NN_RMSG_PAYLOAD (rmsg), sz, rmsg);
          thread_state_asleep (self);
       }
#if SYSDEPS_MSGHDR_FLAGS
    }
#endif
  }

  if (rmsg) nn_rmsg_commit (rmsg);
}

struct local_participant_desc {
  os_socket sock;
  nn_guid_prefix_t guid_prefix;
};

static int local_participant_cmp (const void *va, const void *vb)
{
  const struct local_participant_desc *a = va;
  const struct local_participant_desc *b = vb;
  return (a->sock == b->sock) ? 0 : (a->sock < b->sock) ? -1 : 1;
}

static size_t dedup_sorted_array (void *base, size_t nel, size_t width, int (*compar) (const void *, const void *))
{
  if (nel <= 1)
    return nel;
  else
  {
    char * const end = (char *) base + nel * width;
    char *last_unique = base;
    char *cursor = (char *) base + width;
    size_t n_unique = 1;
    while (cursor != end)
    {
      if (compar (cursor, last_unique) != 0)
      {
        n_unique++;
        last_unique += width;
        if (last_unique != cursor)
          memcpy (last_unique, cursor, width);
      }
      cursor += width;
    }
    return n_unique;
  }
}

struct local_participant_set {
  struct local_participant_desc *ps;
  int nps;
  os_uint32 gen;
};

static void local_participant_set_init (struct local_participant_set *lps)
{
  lps->ps = NULL;
  lps->nps = 0;
  lps->gen = gv.participant_set_generation - 1;
}

static void local_participant_set_fini (struct local_participant_set *lps)
{
  os_free (lps->ps);
}

static void rebuild_local_participant_set (struct thread_state1 *self, struct local_participant_set *lps)
{
  struct ephash_enum_participant est;
  struct participant *pp;
  int nps_alloc;
  TRACE (("pp set gen changed: local %u global %u\n",
          lps->gen, gv.participant_set_generation));
  thread_state_awake (self);
 restart:
  lps->gen = gv.participant_set_generation;
  /* Actual local set of participants may never be older than the
     local generation count => membar to guarantee the ordering */
  pa_membar_consumer ();
  nps_alloc = gv.nparticipants;
  os_free (lps->ps);
  lps->nps = 0;
  lps->ps = os_malloc (nps_alloc * sizeof (*lps->ps));
  if (lps->ps == NULL && nps_alloc > 0)
    NN_FATAL1 ("ddsi2: out of memory (resizing participant set, n = %d)\n", nps_alloc);
  ephash_enum_participant_init (&est);
  while ((pp = ephash_enum_participant_next (&est)) != NULL)
  {
    if (lps->nps == nps_alloc)
    {
      /* New participants may get added while we do this (or
         existing ones removed), so we may have to restart if it
         turns out we didn't allocate enough memory [an
         alternative would be to realloc on the fly]. */
      ephash_enum_participant_fini (&est);
      TRACE (("  need more memory - restarting\n"));
      goto restart;
    }
    else
    {
      lps->ps[lps->nps].sock = pp->sock;
      lps->ps[lps->nps].guid_prefix = pp->e.guid.prefix;
      TRACE (("  pp %x:%x:%x:%x sock %d\n", PGUID (pp->e.guid), (int) pp->sock));
      lps->nps++;
    }
  }
  ephash_enum_participant_fini (&est);

  /* There is a (very small) probability of a participant
     disappearing and new one appearing with the same socket while
     we are enumerating, which would cause us to misinterpret the
     participant guid prefix for a directed packet without an
     explicit destination. Membar because we must have completed
     the loop before testing the generation again. */
  pa_membar_consumer ();
  if (lps->gen != gv.participant_set_generation)
  {
    TRACE (("  set changed - restarting\n"));
    goto restart;
  }
  thread_state_asleep (self);

  /* The definition of the hash enumeration allows visiting one
     participant multiple times, so guard against that, too.  Note
     that there's no requirement that the set be ordered on
     socket: it is merely a convenient way of finding
     duplicates. */
  qsort (lps->ps, lps->nps, sizeof (*lps->ps), local_participant_cmp);
  lps->nps = (int) dedup_sorted_array (lps->ps, lps->nps, sizeof (*lps->ps), local_participant_cmp);
  TRACE (("  nparticipants %d\n", lps->nps));
}

void *recv_thread (struct nn_rbufpool *rbpool)
{
  struct thread_state1 *self = lookup_thread_state ();
  struct local_participant_set lps;
  const int num_fixed = 4; /* (disc|data)_sock[um]c */
  os_int64 next_thread_cputime = 0;
  int i;

  local_participant_set_init (&lps);
  nn_rbufpool_setowner (rbpool, os_threadIdSelf ());

  os_sockWaitsetAddSocket (gv.sockws, gv.discsock_uc, OS_SOCKEVENT_READ);
  os_sockWaitsetAddSocket (gv.sockws, gv.discsock_mc, OS_SOCKEVENT_READ);
  os_sockWaitsetAddSocket (gv.sockws, gv.datasock_uc, OS_SOCKEVENT_READ);
  os_sockWaitsetAddSocket (gv.sockws, gv.datasock_mc, OS_SOCKEVENT_READ);

  while (gv.rtps_keepgoing)
  {
    LOG_THREAD_CPUTIME (next_thread_cputime);

    if (!config.many_sockets_mode)
      ; /* no other sockets to check */
    else if (gv.participant_set_generation == lps.gen)
      ; /* no change to the set of participants */
    else
    {
      /* something happened => rebuild local participant set */
      rebuild_local_participant_set (self, &lps);

      /* and rebuild waitset */
      os_sockWaitsetRemoveSockets (gv.sockws, num_fixed);
      for (i = 0; i < lps.nps; i++)
        os_sockWaitsetAddSocket (gv.sockws, lps.ps[i].sock, OS_SOCKEVENT_READ);
    }

    if (os_sockWaitsetWait (gv.sockws, -1) == os_resultFail)
      NN_FATAL0 ("ddsi2: sockWaitsetWait failed\n");
    else
    {
      int idx;
      os_socket sock;
      unsigned events;
      while ((idx = os_sockWaitsetNextEvent (gv.sockws, &sock, &events)) >= 0)
      {
        if (events & OS_SOCKEVENT_READ)
        {
          static const nn_guid_prefix_t null_guid_prefix;
          if (idx < num_fixed)
            do_packet (self, sock, &null_guid_prefix, rbpool);
          else if (!config.many_sockets_mode)
            NN_FATAL1 ("ddsi2: sockWaitsetNextEvent returned %d but ManySocketsMode disabled\n", idx);
          else if (idx - num_fixed >= lps.nps)
            NN_FATAL2 ("ddsi2: sockWaitsetNextEvent returned %d but only have %d participants\n", idx, lps.nps);
          else
            do_packet (self, sock, &lps.ps[idx - num_fixed].guid_prefix, rbpool);
        }
      }
    }
  }
  local_participant_set_fini (&lps);
  return NULL;
}

/* SHA1 not available (unoffical build.) */
