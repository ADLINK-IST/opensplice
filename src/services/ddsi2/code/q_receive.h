#ifndef Q_RECEIVE_H
#define Q_RECEIVE_H

#if defined (__cplusplus)
extern "C" {
#endif

struct nn_rbufpool;
struct nn_rsample_info;
struct nn_rdata;

void *recv_thread (struct nn_rbufpool *rbpool);
int user_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, const nn_guid_t *rdguid, void *qarg);

#if defined (__cplusplus)
}
#endif

#endif /* Q_RECEIVE_H */

/* SHA1 not available (unoffical build.) */
