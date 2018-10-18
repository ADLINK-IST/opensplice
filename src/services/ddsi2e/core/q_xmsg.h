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
#ifndef NN_XMSG_H
#define NN_XMSG_H

#include <stddef.h>

#include "q_protocol.h" /* for, e.g., SubmessageKind_t */
#include "q_xqos.h" /* for, e.g., octetseq, stringseq */
#include "ddsi_tran.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct serdata;
struct addrset;
struct proxy_reader;
struct proxy_writer;

struct nn_prismtech_participant_version_info;
struct nn_prismtech_writer_info;
struct nn_xmsgpool;
struct nn_xmsg_data;
struct nn_xmsg;
struct nn_xpack;

struct nn_xmsg_marker {
  size_t offset;
};

enum nn_xmsg_kind {
  NN_XMSG_KIND_CONTROL,
  NN_XMSG_KIND_DATA,
  NN_XMSG_KIND_DATA_REXMIT
};

/* XMSGPOOL */

struct nn_xmsgpool *nn_xmsgpool_new (void);
void nn_xmsgpool_free (struct nn_xmsgpool *pool);

/* XMSG */

/* To allocate a new xmsg from the pool; if expected_size is NOT
   exceeded, no reallocs will be performed, else the address of the
   xmsg may change because of reallocing when appending to it. */
struct nn_xmsg *nn_xmsg_new (struct nn_xmsgpool *pool, const nn_guid_prefix_t *src_guid_prefix, size_t expected_size, enum nn_xmsg_kind kind);

/* For sending to a particular destination (participant) */
void nn_xmsg_setdst1 (struct nn_xmsg *m, const nn_guid_prefix_t *gp, const nn_locator_t *addr);

/* For sending to a particular proxy reader; this is a convenience
   routine that extracts a suitable address from the proxy reader's
   address sets and calls setdst1. */
int nn_xmsg_setdstPRD (struct nn_xmsg *m, const struct proxy_reader *prd);
int nn_xmsg_setdstPWR (struct nn_xmsg *m, const struct proxy_writer *pwr);

/* For sending to all in the address set AS -- typically, the writer's
   address set to multicast to all matched readers */
void nn_xmsg_setdstN (struct nn_xmsg *msg, struct addrset *as, struct addrset *as_group);

int nn_xmsg_setmaxdelay (struct nn_xmsg *msg, os_int64 maxdelay);

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
int nn_xmsg_setencoderid (struct nn_xmsg *msg, os_uint32 encoderid);
#endif

/* Sets the location of the destination readerId within the message
   (address changes because of reallocations are handled correctly).
   M must be a rexmit, and for all rexmits this must be called.  It is
   a separate function because the location may only become known at a
   late-ish stage in the construction of the message. */
void nn_xmsg_set_data_readerId (struct nn_xmsg *m, nn_entityid_t *readerId);

/* If M and MADD are both xmsg's containing the same retransmit
   message, this will merge the destination embedded in MADD into M.
   Typically, this will cause the readerId of M to be cleared and the
   destination to change to the writer's address set.

   M and MADD *must* contain the same sample/fragment of a sample.

   Returns 1 if merge was successful, else 0.  On failure, neither
   message will have been changed and both should be sent as if there
   had been no merging. */
int nn_xmsg_merge_rexmit_destinations_wrlock_held (struct nn_xmsg *m, const struct nn_xmsg *madd);

/* To set writer ids for updating last transmitted sequence number;
   wrfragid is 0 based, unlike DDSI but like other places where
   fragment numbers are handled internally. */
void nn_xmsg_setwriterseq (struct nn_xmsg *msg, const nn_guid_t *wrguid, os_int64 wrseq);
void nn_xmsg_setwriterseq_fragid (struct nn_xmsg *msg, const nn_guid_t *wrguid, os_int64 wrseq, nn_fragment_number_t wrfragid);

/* Comparison function for retransmits: orders messages on writer
   guid, sequence number and fragment id */
int nn_xmsg_compare_fragid (const struct nn_xmsg *a, const struct nn_xmsg *b);

void nn_xmsg_free (struct nn_xmsg *msg);
size_t nn_xmsg_size (const struct nn_xmsg *m);
void *nn_xmsg_payload (size_t *sz, struct nn_xmsg *m);
enum nn_xmsg_kind nn_xmsg_kind (const struct nn_xmsg *m);
void nn_xmsg_guid_seq_fragid (const struct nn_xmsg *m, nn_guid_t *wrguid, os_int64 *wrseq, nn_fragment_number_t *wrfragid);

void *nn_xmsg_submsg_from_marker (struct nn_xmsg *msg, struct nn_xmsg_marker marker);
void *nn_xmsg_append (struct nn_xmsg *m, struct nn_xmsg_marker *marker, size_t sz);
void nn_xmsg_shrink (struct nn_xmsg *m, struct nn_xmsg_marker marker, size_t sz);
void nn_xmsg_serdata (struct nn_xmsg *m, struct serdata *serdata, unsigned off, unsigned len);
void nn_xmsg_submsg_setnext (struct nn_xmsg *msg, struct nn_xmsg_marker marker);
void nn_xmsg_submsg_init (struct nn_xmsg *msg, struct nn_xmsg_marker marker, SubmessageKind_t smkind);
void nn_xmsg_add_timestamp (struct nn_xmsg *m, nn_wctime_t t);
void nn_xmsg_add_entityid (struct nn_xmsg * m);
void *nn_xmsg_addpar (struct nn_xmsg *m, unsigned pid, size_t len);
void nn_xmsg_addpar_string (struct nn_xmsg *m, unsigned pid, const char *str);
void nn_xmsg_addpar_octetseq (struct nn_xmsg *m, unsigned pid, const nn_octetseq_t *oseq);
void nn_xmsg_addpar_stringseq (struct nn_xmsg *m, unsigned pid, const nn_stringseq_t *sseq);
void nn_xmsg_addpar_guid (struct nn_xmsg *m, unsigned pid, const nn_guid_t *guid);
void nn_xmsg_addpar_BE4u (struct nn_xmsg *m, unsigned pid, unsigned x);
void nn_xmsg_addpar_4u (struct nn_xmsg *m, unsigned pid, unsigned x);
void nn_xmsg_addpar_keyhash (struct nn_xmsg *m, const struct serdata *serdata);
void nn_xmsg_addpar_statusinfo (struct nn_xmsg *m, unsigned statusinfo);
void nn_xmsg_addpar_reliability (struct nn_xmsg *m, unsigned pid, const struct nn_reliability_qospolicy *rq);
void nn_xmsg_addpar_share (struct nn_xmsg *m, unsigned pid, const struct nn_share_qospolicy *rq);
void nn_xmsg_addpar_subscription_keys (struct nn_xmsg *m, unsigned pid, const struct nn_subscription_keys_qospolicy *rq);

#if !LITE
void nn_xmsg_addpar_wrinfo (struct nn_xmsg *m, const struct nn_prismtech_writer_info *wri);
#endif
void nn_xmsg_addpar_parvinfo (struct nn_xmsg *m, unsigned pid, const struct nn_prismtech_participant_version_info *pvi);
void nn_xmsg_addpar_eotinfo (struct nn_xmsg *m, unsigned pid, const struct nn_prismtech_eotinfo *txnid);
void nn_xmsg_addpar_sentinel (struct nn_xmsg *m);
int nn_xmsg_addpar_sentinel_ifparam (struct nn_xmsg *m);

/* XPACK */

struct nn_xpack * nn_xpack_new (ddsi_tran_conn_t conn, os_uint32 bw_limit);
void nn_xpack_free (struct nn_xpack *xp);
void nn_xpack_send (struct nn_xpack *xp);
int nn_xpack_addmsg (struct nn_xpack *xp, struct nn_xmsg *m, const os_uint32 flags);
os_int64 nn_xpack_maxdelay (const struct nn_xpack *xp);
unsigned nn_xpack_packetid (const struct nn_xpack *xp);

#if defined (__cplusplus)
}
#endif
#endif /* NN_XMSG_H */
