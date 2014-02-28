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

struct nn_xmsgpool;
struct nn_xmsg_data;
struct nn_xmsg;
struct nn_xpack;

struct nn_xmsg_marker {
  os_uint32 offset;
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
struct nn_xmsg *nn_xmsg_new (struct nn_xmsgpool *pool, const nn_guid_prefix_t *src_guid_prefix, unsigned expected_size, enum nn_xmsg_kind kind);

/* For sending to a particular destination (participant) */
int nn_xmsg_setdst1 (struct nn_xmsg *m, const nn_guid_prefix_t *gp, const os_sockaddr_storage *addr);

/* For sending to a particular proxy reader; this is a convenience
   routine that extracts a suitable address from the proxy reader's
   address sets and calls setdst1. */
int nn_xmsg_setdstPRD (struct nn_xmsg *m, const struct proxy_reader *prd);

/* For sending to all in the address set AS -- typically, the writer's
   address set to multicast to all matched readers */
int nn_xmsg_setdstN (struct nn_xmsg *msg, struct addrset *as);

int nn_xmsg_setmaxdelay (struct nn_xmsg *msg, os_int64 maxdelay);


/* Sets the location of the destination readerId within the message
   (address changes because of reallocations are handled correctly).
   M must be a rexmit, and for all rexmits this must be called.  It is
   a separate function because the location may only become known at a
   late-ish stage in the construction of the message. */
int nn_xmsg_set_data_readerId (struct nn_xmsg *m, nn_entityid_t *readerId);

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
int nn_xmsg_setwriterseq (struct nn_xmsg *msg, const nn_guid_t *wrguid, os_int64 wrseq);
int nn_xmsg_setwriterseq_fragid (struct nn_xmsg *msg, const nn_guid_t *wrguid, os_int64 wrseq, nn_fragment_number_t wrfragid);

/* Comparison function for retransmits: orders messages on writer
   guid, sequence number and fragment id */
int nn_xmsg_compare_fragid (const struct nn_xmsg *a, const struct nn_xmsg *b);

void nn_xmsg_free (struct nn_xmsg *msg);
unsigned nn_xmsg_size (const struct nn_xmsg *m);
void *nn_xmsg_payload (unsigned *sz, struct nn_xmsg *m);
enum nn_xmsg_kind nn_xmsg_kind (const struct nn_xmsg *m);
void nn_xmsg_guid_seq_fragid (const struct nn_xmsg *m, nn_guid_t *wrguid, os_int64 *wrseq, nn_fragment_number_t *wrfragid);

void *nn_xmsg_append (struct nn_xmsg *m, struct nn_xmsg_marker *marker, unsigned sz);
void nn_xmsg_shrink (struct nn_xmsg *m, struct nn_xmsg_marker marker, unsigned sz);
int nn_xmsg_serdata (struct nn_xmsg *m, struct serdata *serdata, unsigned off, unsigned len);
void nn_xmsg_submsg_setnext (struct nn_xmsg *msg, struct nn_xmsg_marker marker);
void nn_xmsg_submsg_init (struct nn_xmsg *msg, struct nn_xmsg_marker marker, SubmessageKind_t smkind);
int nn_xmsg_add_timestamp (struct nn_xmsg *m, os_int64 t);
void *nn_xmsg_addpar (struct nn_xmsg *m, int pid, int len);
int nn_xmsg_addpar_string (struct nn_xmsg *m, int pid, const char *str);
int nn_xmsg_addpar_octetseq (struct nn_xmsg *m, int pid, const nn_octetseq_t *oseq);
int nn_xmsg_addpar_stringseq (struct nn_xmsg *m, int pid, const nn_stringseq_t *sseq);
int nn_xmsg_addpar_guid (struct nn_xmsg *m, int pid, const nn_guid_t *guid);
int nn_xmsg_addpar_BE4u (struct nn_xmsg *m, int pid, unsigned x);
int nn_xmsg_addpar_4u (struct nn_xmsg *m, int pid, unsigned x);
int nn_xmsg_addpar_keyhash (struct nn_xmsg *m, const struct serdata *serdata);
int nn_xmsg_addpar_statusinfo (struct nn_xmsg *m, unsigned statusinfo);
int nn_xmsg_addpar_reliability (struct nn_xmsg *m, int pid, const struct nn_reliability_qospolicy *rq);

struct nn_prismtech_writer_info;
int nn_xmsg_addpar_wrinfo (struct nn_xmsg *m, const struct nn_prismtech_writer_info *wri);
struct nn_prismtech_participant_version_info;
int nn_xmsg_addpar_parvinfo (struct nn_xmsg *m, int pid, const struct nn_prismtech_participant_version_info *pvi);
int nn_xmsg_addpar_sentinel (struct nn_xmsg *m);
int nn_xmsg_addpar_sentinel_ifparam (struct nn_xmsg *m);

/* XPACK */

struct nn_xpack * nn_xpack_new (ddsi_tran_conn_t conn);
void nn_xpack_free (struct nn_xpack *xp);
void nn_xpack_send (struct nn_xpack *xp);
int nn_xpack_addmsg (struct nn_xpack *xp, struct nn_xmsg *m);
os_int64 nn_xpack_maxdelay (const struct nn_xpack *xp);
unsigned nn_xpack_packetid (const struct nn_xpack *xp);

#if defined (__cplusplus)
}
#endif

#endif /* NN_XMSG_H */

/* SHA1 not available (unoffical build.) */
