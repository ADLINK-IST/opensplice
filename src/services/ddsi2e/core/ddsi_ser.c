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
#include "ddsi_ser.h"

#include <stddef.h>
#include <ctype.h>
#include <assert.h>

#include "os_stdlib.h"
#include "os_defs.h"
#include "os_thread.h"
#include "os_heap.h"
#include "os_atomics.h"
#include "sysdeps.h"
#include "q_md5.h"
#include "q_bswap.h"
#include "q_osplser.h"

#define MAX_POOL_SIZE 1024

#ifndef NDEBUG
static int ispowerof2_size (size_t x)
{
  return x > 0 && !(x & (x-1));
}
#endif

static size_t alignup_size (size_t x, size_t a);
static serstate_t serstate_allocnew (serstatepool_t pool, const struct sertopic * topic);

serstatepool_t ddsi_serstatepool_new (void)
{
  serstatepool_t pool;
  pool = os_malloc (sizeof (*pool));

#if USE_ATOMIC_LIFO
  os_atomic_lifo_init (&pool->freelist);
  pa_st32(&pool->approx_nfree, 0);
#else
  {
    os_mutexInit (&pool->lock, NULL);
    pool->freelist = NULL;
    pool->nalloced = 0;
    pool->nfree = 0;
  }
#endif
  return pool;
}

void ddsi_serstatepool_free (serstatepool_t pool)
{
#if USE_ATOMIC_LIFO
  serstate_t st;
  while ((st = os_atomic_lifo_pop (&pool->freelist, offsetof (struct serstate, next))) != NULL)
  {
    serstate_free (st);
  }
  TRACE (("ddsi_serstatepool_free(%p)\n", pool));
#else
  while (pool->freelist)
  {
    serstate_t st = pool->freelist;
    pool->freelist = st->next;
    serstate_free (st);
  }
  os_mutexDestroy (&pool->lock);
  nn_log (LC_TOPIC, "ddsi_serstatepool_free(%p) nalloced %d nfree %d\n", (void*)pool, pool->nalloced, pool->nfree);
#endif
  os_free (pool);
}

int ddsi_serdata_refcount_is_1 (serdata_t serdata)
{
  return (pa_ld32 (&serdata->v.st->refcount) == 1);
}

serdata_t ddsi_serdata_ref (serdata_t serdata)
{
  pa_inc32 (&serdata->v.st->refcount);
  return serdata;
}

void ddsi_serdata_unref (serdata_t serdata)
{
  ddsi_serstate_release (serdata->v.st);
}

nn_mtime_t ddsi_serdata_twrite (const struct serdata *serdata)
{
  return ddsi_serstate_twrite (serdata->v.st);
}

void ddsi_serdata_set_twrite (serdata_t serdata, nn_mtime_t twrite)
{
  ddsi_serstate_set_twrite (serdata->v.st, twrite);
}

serstate_t ddsi_serstate_new (serstatepool_t pool, const struct sertopic * topic)
{
  serstate_t st;
#if USE_ATOMIC_LIFO
  if ((st = os_atomic_lifo_pop (&pool->freelist, offsetof (struct serstate, next))) != NULL)
  {
    pa_dec32(&pool->approx_nfree);
    serstate_init (st, topic);
  }
  else
  {
    st = serstate_allocnew (pool, topic);
  }
#else
  os_mutexLock (&pool->lock);
  if (pool->freelist == NULL)
  {
    os_mutexUnlock (&pool->lock);
    st = serstate_allocnew (pool, topic);
  }
  else
  {
    st = pool->freelist;
    pool->freelist = st->next;
    pool->nfree--;
    os_mutexUnlock (&pool->lock);
    serstate_init (st, topic);
  }
#endif
  return st;
}

serdata_t ddsi_serstate_fix (serstate_t st)
{
  /* see serialize_raw_private() */
  ddsi_serstate_append_aligned (st, 0, 4);
  return st->data;
}

nn_mtime_t ddsi_serstate_twrite (const struct serstate *serstate)
{
  assert (serstate->twrite.v >= 0);
  return serstate->twrite;
}

void ddsi_serstate_set_twrite (serstate_t st, nn_mtime_t twrite)
{
  st->twrite = twrite;
}

void ddsi_serstate_append_blob (serstate_t st, size_t align, size_t sz, const void *data)
{
  char *p = ddsi_serstate_append_aligned (st, sz, align);
  memcpy (p, data, sz);
}

void ddsi_serstate_set_msginfo
(
  serstate_t st, unsigned statusinfo, nn_wctime_t timestamp,
#if LITE
  void * dummy
#else
  const struct nn_prismtech_writer_info *wri
#endif
)
{
  serdata_t d = st->data;
  d->v.msginfo.statusinfo = statusinfo;
  d->v.msginfo.timestamp = timestamp;
#if !LITE
  if (wri == NULL)
    d->v.msginfo.have_wrinfo = 0;
  else
  {
    d->v.msginfo.have_wrinfo = 1;
    d->v.msginfo.wrinfo = *wri;
  }
#endif
}

os_uint32 ddsi_serdata_size (const struct serdata *serdata)
{
  const struct serstate *st = serdata->v.st;
  if (serdata->v.st->kind == STK_EMPTY)
    return 0;
  else
    return (os_uint32) (sizeof (struct CDRHeader) + st->pos);
}

int ddsi_serdata_is_key (const struct serdata * serdata)
{
  return serdata->v.st->kind == STK_KEY;
}

int ddsi_serdata_is_empty (const struct serdata * serdata)
{
  return serdata->v.st->kind == STK_EMPTY;
}

/* Internal static functions */

static serstate_t serstate_allocnew (serstatepool_t pool, const struct sertopic * topic)
{
  serstate_t st = os_malloc (sizeof (*st));
  os_size_t size;

  memset (st, 0, sizeof (*st));

#if ! USE_ATOMIC_LIFO
  pool->nalloced++;
#endif
  st->size = 128;
  st->pool = pool;

  size = offsetof (struct serdata, data) + st->size;
  st->data = os_malloc (size);
  memset (st->data, 0, size);
  st->data->v.st = st;
  serstate_init (st, topic);
  return st;
}

void * ddsi_serstate_append (serstate_t st, size_t n)
{
  char *p;
  if (st->pos + n > st->size)
  {
    size_t size1 = alignup_size (st->pos + n, 128);
    serdata_t data1 = os_realloc (st->data, offsetof (struct serdata, data) + size1);
    st->data = data1;
    st->size = size1;
  }
  assert (st->pos + n <= st->size);
  p = st->data->data + st->pos;
  st->pos += n;
  return p;
}

void ddsi_serstate_release (serstate_t st)
{
  if (pa_dec32_nv (&st->refcount) == 0)
  {
    serstatepool_t pool = st->pool;
#if LITE
    sertopic_free ((sertopic_t) st->topic);
#endif
#if USE_ATOMIC_LIFO
    if (pa_inc32_nv(&pool->approx_nfree) <= MAX_POOL_SIZE)
      os_atomic_lifo_push (&pool->freelist, st, offsetof (struct serstate, next));
    else
    {
      pa_dec32(&pool->approx_nfree);
      serstate_free (st);
    }
#else
    os_mutexLock (&pool->lock);
#if 0
#ifndef NDEBUG
    {
      serstate_t b;
      for (b = pool->freelist; b && b != st; b = b->next)
        ;
      assert (b == NULL);
    }
#endif
#endif
    if (pool->nfree >= MAX_POOL_SIZE)
    {
      os_mutexUnlock (&pool->lock);
      serstate_free (st);
    }
    else
    {
      st->next = pool->freelist;
      pool->freelist = st;
      pool->nfree++;
      os_mutexUnlock (&pool->lock);
    }
#endif /* USE_ATOMIC_LIFO */
  }
}

void * ddsi_serstate_append_align (serstate_t st, size_t sz)
{
  return ddsi_serstate_append_aligned (st, sz, sz);
}

void * ddsi_serstate_append_aligned (serstate_t st, size_t n, size_t a)
{
  /* Simply align st->pos, without verifying it fits in the allocated
     buffer: ddsi_serstate_append() is called immediately afterward and will
     grow the buffer as soon as the end of the requested space no
     longer fits. */
  size_t pos0 = st->pos;
  char *p;
  assert (ispowerof2_size (a));
  st->pos = alignup_size (st->pos, a);
  p = ddsi_serstate_append (st, n);
  if (p && st->pos > pos0)
    memset (st->data->data + pos0, 0, st->pos - pos0);
  return p;
}

static size_t alignup_size (size_t x, size_t a)
{
  assert (ispowerof2_size (a));
  return -((-x) & (-a));
}
