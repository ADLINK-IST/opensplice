#ifndef Q_RECEIVE_H
#define Q_RECEIVE_H

struct nn_rbufpool;
struct nn_rsample_info;
struct nn_rdata;

void *recv_thread (struct nn_rbufpool *rbpool);
int user_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, void *qarg);

#endif /* Q_RECEIVE_H */

/* SHA1 not available (unoffical build.) */
