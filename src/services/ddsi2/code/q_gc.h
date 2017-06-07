/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef Q_GC_H
#define Q_GC_H

#include "q_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct gcreq;
struct gcreq_queue;

struct writer;
struct reader;
struct proxy_writer;
struct proxy_reader;

typedef void (*gcreq_cb_t) (struct gcreq *gcreq);

struct idx_vtime {
  unsigned idx;
  vtime_t vtime;
};

struct gcreq {
  struct gcreq *next;
  struct gcreq_queue *queue;
  gcreq_cb_t cb;
  void *arg;
  unsigned nvtimes;
  struct idx_vtime vtimes[1 /* really a flex ary */];
};

struct gcreq_queue *gcreq_queue_new (void);
void gcreq_queue_free (struct gcreq_queue *q);

struct gcreq *gcreq_new (struct gcreq_queue *gcreq_queue, gcreq_cb_t cb);
void gcreq_free (struct gcreq *gcreq);
void gcreq_enqueue (struct gcreq *gcreq);
int gcreq_requeue (struct gcreq *gcreq, gcreq_cb_t cb);

#if defined (__cplusplus)
}
#endif

#endif /* Q_GC_H */

/* SHA1 not available (unoffical build.) */
