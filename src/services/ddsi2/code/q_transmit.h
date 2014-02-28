#ifndef Q_TRANSMIT_H
#define Q_TRANSMIT_H

#include "os_defs.h"
#include "q_rtps.h" /* for nn_entityid_t */

#if defined (__cplusplus)
extern "C" {
#endif

struct nn_xpack;
struct nn_xmsg;
struct writer;
struct proxy_reader;
struct serdata;

/* Writing new data; serdata_twrite (serdata) is assumed to be really
   recentish; serdata is unref'd.  If xp == NULL, data is queued, else
   packed. */
int write_sample (struct nn_xpack *xp, struct writer *wr, struct serdata *serdata);
int write_sample_kernel_seq (struct nn_xpack *xp, struct writer *wr, struct serdata *serdata, int have_kernel_seq, os_uint32 kernel_seq);

/* When calling the following functions, wr->lock must be held */
int create_fragment_message (struct writer *wr, os_int64 seq, struct serdata *serdata, unsigned fragnum, struct proxy_reader *prd,struct nn_xmsg **msg, int isnew);
int enqueue_sample_wrlock_held (struct writer *wr, os_int64 seq, struct serdata *serdata, struct proxy_reader *prd, int isnew);
int add_Heartbeat (struct nn_xmsg *msg, struct writer *wr, int hbansreq, nn_entityid_t dst, os_int64 tnow, int issync);

#if defined (__cplusplus)
}
#endif

#endif /* Q_TRANSMIT_H */

/* SHA1 not available (unoffical build.) */
