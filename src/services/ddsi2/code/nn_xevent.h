#ifndef NN_XEVENT_H
#define NN_XEVENT_H

#include "nn_avl.h"

struct nn_xmsg;
struct addrset;
struct participant;
struct writer;
struct rhc_writers_node;
struct proxy_reader;

enum xeventkind {
  XEVK_MSG,
  XEVK_HEARTBEAT,
  XEVK_ACKNACK,
  XEVK_SPDP,
  XEVK_PMD_UPDATE,
  XEVK_INFO,
  XEVK_END_STARTUP_MODE
};

struct xevent {
  STRUCT_AVLNODE (xevent_avlnode, struct xevent *) avlnode;
  struct xeventq *evq;
  os_int64 tsched;
  os_int64 min_tsched; /* in subtree */
  int size; /* number of events in subtree scheduled for ~immediate transmission */
  enum xeventkind kind;
  union {
    struct {
      struct nn_xmsg *msg;
    } msg;
    struct {
      struct writer *wr;
    } heartbeat;
    struct {
      struct rhc_writers_node *rwn;
    } acknack;
    struct {
      struct participant *pp;
      struct proxy_participant *dest;
    } spdp;
    struct {
      struct participant *pp;
    } pmd_update;
#if 0
    struct {
    } info;
#endif
#if 0
    struct {
    } end_startup_mode;
#endif
  } u;
};

struct xeventq {
  STRUCT_AVLTREE (xeventq_avltree, struct xevent *) xevents;
  int oldsize; /* number of events in subtree scheduled for ~immediate transmission */
};

void delete_xevent (struct xevent *ev);
void resched_xevent (struct xevent *ev, os_int64 tsched);
int resched_xevent_if_earlier (struct xevent *ev, os_int64 tsched);
struct xevent *qxev_msg (struct xeventq *evq, os_int64 tsched, struct nn_xmsg *msg);
struct xevent *qxev_heartbeat (struct xeventq *evq, os_int64 tsched, struct writer *wr);
struct xevent *qxev_acknack (struct xeventq *evq, os_int64 tsched, struct rhc_writers_node *rwn);
struct xevent *qxev_spdp (struct xeventq *evq, os_int64 tsched, struct participant *pp, struct proxy_participant *dest);
struct xevent *qxev_pmd_update (struct xeventq *evq, os_int64 tsched, struct participant *pp);
struct xevent *qxev_info (struct xeventq *evq, os_int64 tsched);
struct xevent *qxev_end_startup_mode (struct xeventq *evq, os_int64 tsched);
struct xeventq *new_xeventq (void);
void free_xeventq (struct xeventq *evq);
int xeventq_size (const struct xeventq *evq);
int xeventq_must_throttle (struct xeventq *evq);
int xeventq_may_continue (struct xeventq *evq);
void xeventq_adjust_throttle (struct xeventq *evq);
os_int64 earliest_in_xeventq (struct xeventq *evq);
struct xevent *next_from_xeventq (struct xeventq *evq);
struct xevent *eventq_peeksucc (struct xevent *ev);

#endif /* NN_XEVENT_H */
