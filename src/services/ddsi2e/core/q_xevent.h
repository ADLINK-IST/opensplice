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
struct xevent;
struct xeventq;
struct proxy_writer;
struct proxy_reader;

struct xeventq *xeventq_new
(
  struct ddsi_tran_conn * conn,
  size_t max_queued_rexmit_bytes,
  size_t max_queued_rexmit_msgs,
  os_uint32 auxiliary_bandwidth_limit
);

/* xeventq_free calls callback handlers with t = T_NEVER, at which point they are required to free
   whatever memory is claimed for the argument and call delete_xevent. */
void xeventq_free (struct xeventq *evq);
int xeventq_start (struct xeventq *evq, const char *name); /* <0 => error, =0 => ok */
void xeventq_stop (struct xeventq *evq);

void qxev_msg (struct xeventq *evq, struct nn_xmsg *msg);
void qxev_pwr_entityid (struct proxy_writer * pwr, nn_guid_prefix_t * id);
void qxev_prd_entityid (struct proxy_reader * prd, nn_guid_prefix_t * id);

/* Returns 1 if queued, 0 otherwise (no point in returning the
   event, you can't do anything with it anyway) */
int qxev_msg_rexmit_wrlock_held (struct xeventq *evq, struct nn_xmsg *msg, int force);

/* All of the following lock EVQ for the duration of the operation */
void delete_xevent (struct xevent *ev);
int resched_xevent_if_earlier (struct xevent *ev, nn_mtime_t tsched);

struct xevent *qxev_heartbeat (struct xeventq *evq, nn_mtime_t tsched, const nn_guid_t *wr_guid);
struct xevent *qxev_acknack (struct xeventq *evq, nn_mtime_t tsched, const nn_guid_t *pwr_guid, const nn_guid_t *rd_guid);
struct xevent *qxev_spdp (nn_mtime_t tsched, const nn_guid_t *pp_guid, const nn_guid_t *proxypp_guid);
struct xevent *qxev_pmd_update (nn_mtime_t tsched, const nn_guid_t *pp_guid);
struct xevent *qxev_end_startup_mode (nn_mtime_t tsched);
struct xevent *qxev_delete_writer (nn_mtime_t tsched, const nn_guid_t *guid);

/* cb will be called with now = T_NEVER if the event is still enqueued when when xeventq_free starts cleaning up */
struct xevent *qxev_callback (nn_mtime_t tsched, void (*cb) (struct xevent *xev, void *arg, nn_mtime_t now), void *arg);

#if defined (__cplusplus)
}
#endif
#endif /* NN_XEVENT_H */
