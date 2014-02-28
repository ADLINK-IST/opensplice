#ifndef NN_XEVENT_H
#define NN_XEVENT_H

#if defined (__cplusplus)
extern "C" {
#endif

/* NOTE: xevents scheduled with the same tsched used to always be
   executed in the order of scheduling, but that is no longer true.
   With the messages now via the "untimed" path, that should not
   introduce any issues. */

struct writer;
struct pwr_rd_match;
struct participant;
struct proxy_participant;
struct ddsi_tran_conn;

struct xevent; /* timed xevents */
struct xeventq;

struct xeventq *xeventq_new
(
  struct ddsi_tran_conn * conn,
  unsigned max_queued_rexmit_bytes,
  unsigned max_queued_rexmit_msgs
);
void xeventq_free (struct xeventq *evq);
int xeventq_start (struct xeventq *evq, const char *name); /* <0 => error, =0 => ok */
void xeventq_stop (struct xeventq *evq);

/* These return 1 if queued, 0 otherwise (no point in returning the
   event, you can't do anything with it anyway) */
int qxev_msg (struct xeventq *evq, struct nn_xmsg *msg);
int qxev_msg_rexmit_wrlock_held (struct xeventq *evq, struct nn_xmsg *msg, int force);

/* All of the following lock EVQ for the duration of the operation */
void delete_xevent (struct xevent *ev);
int resched_xevent_if_earlier (struct xevent *ev, os_int64 tsched);

struct xevent *qxev_heartbeat (struct xeventq *evq, os_int64 tsched, const nn_guid_t *wr_guid);
struct xevent *qxev_acknack (struct xeventq *evq, os_int64 tsched, const nn_guid_t *pwr_guid, const nn_guid_t *rd_guid);
struct xevent *qxev_spdp (os_int64 tsched, const nn_guid_t *pp_guid, const nn_guid_t *proxypp_guid);
struct xevent *qxev_pmd_update (os_int64 tsched, const nn_guid_t *pp_guid);
struct xevent *qxev_end_startup_mode (os_int64 tsched);
struct xevent *qxev_delete_writer (os_int64 tsched, const nn_guid_t *guid);
struct xevent *qxev_callback (os_int64 tsched, void (*cb) (struct xevent *xev, void *arg, os_int64 now), void *arg);

#if defined (__cplusplus)
}
#endif

#endif /* NN_XEVENT_H */

/* SHA1 not available (unoffical build.) */
