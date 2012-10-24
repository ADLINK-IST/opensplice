#ifndef NN_XMSG_H
#define NN_XMSG_H

#include <stddef.h>

#include "os_socket.h"

#include "nn_protocol.h" /* for, e.g., SubmessageKind_t */
#include "nn_xqos.h" /* for, e.g., octetseq, stringseq */

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

#define NN_XMSG_MAX_FRAGMENT_SIZE 1280

/* XMSGPOOL */

struct nn_xmsgpool *nn_xmsgpool_new (void);
void nn_xmsgpool_free (struct nn_xmsgpool *pool);

/* XMSG */

struct nn_xmsg *nn_xmsg_new (struct nn_xmsgpool *pool, os_socket sock, const nn_guid_prefix_t *src_guid_prefix, unsigned expected_size);
int nn_xmsg_setdst1 (struct nn_xmsg *m, const nn_guid_prefix_t *gp, const nn_locator_udpv4_t *addr);
int nn_xmsg_setdstPRD (struct nn_xmsg *m, const struct proxy_reader *prd);
int nn_xmsg_setdstN (struct nn_xmsg *msg, struct addrset *as);
void nn_xmsg_free (struct nn_xmsg *msg);
void *nn_xmsg_payload (unsigned *sz, struct nn_xmsg *m);

void *nn_xmsg_append_aligned (struct nn_xmsg *m, struct nn_xmsg_marker *marker, unsigned sz, unsigned a);
void nn_xmsg_shrink (struct nn_xmsg *m, struct nn_xmsg_marker marker, unsigned sz);
int nn_xmsg_serdata (struct nn_xmsg *m, struct serdata *serdata, unsigned off, unsigned len);
void nn_xmsg_submsg_setnext (struct nn_xmsg *msg, struct nn_xmsg_marker marker);
void nn_xmsg_submsg_init (struct nn_xmsg *msg, struct nn_xmsg_marker marker, SubmessageKind_t smkind);
int nn_xmsg_add_timestamp (struct nn_xmsg *m, os_int64 t);
int nn_xmsg_add_pl_cdr_header (struct nn_xmsg *m);
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
int nn_xmsg_addpar_sentinel (struct nn_xmsg *m);

/* XPACK */

struct nn_xpack *nn_xpack_new (void);
void nn_xpack_free (struct nn_xpack *xp);
size_t nn_xpack_send (struct nn_xpack *xp);
int nn_xpack_addmsg (struct nn_xpack *xp, struct nn_xmsg *m);

#endif /* NN_XMSG_H */
