/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <stddef.h>

#include "os_heap.h"
#include "os_mutex.h"

#include "nn_mlv.h"
#include "nn_unused.h"

#if USE_MLV
#if __linux__
#define HAVE_BACKTRACE 1
#endif

#if HAVE_BACKTRACE
#include <execinfo.h>

#define TRACE_DEPTH 4
#define TRACE_PRINTF_FMT "%p %p %p %p"
#define TRACE_PRINTF_ARGS(h) (h)->trace[0], (h)->trace[1], (h)->trace[2], (h)->trace[3]
#endif /* HAVE_BACKTRACE */

struct mlvhdr {
  struct mlvhdr *plive, *nlive;
  os_uint64 seq;
  unsigned chk;
  unsigned sz;
#if HAVE_BACKTRACE
  void *trace[TRACE_DEPTH];
#endif
};

struct mlvtrl {
  os_uint64 seq;
};

static volatile int mlvenabled;
static os_mutex mlvlock;
static struct mlvhdr *live = NULL;
#endif /* USE_MLV */

static struct mlv_stats stats;

#if USE_MLV
static void *mlv_malloc (os_size_t sz);
static void *mlv_realloc (void *blk, os_size_t sz);
static void mlv_free (void *blk);

static int check_isbyte (const unsigned char *x, unsigned char val, int n)
{
  int i;
  for (i = 0; i < n && x[i] == val; i++);
  return i == n;
}

static int check (const struct mlvhdr *hdr)
{
  struct mlvtrl strl;
  assert (hdr->chk == ((unsigned) hdr ^ hdr->sz));
  memcpy (&strl, (char *) (hdr + 1) + hdr->sz, sizeof (strl));
  if (check_isbyte ((unsigned char *) hdr, 0xaa, offsetof (struct mlvhdr, chk)) &&
      check_isbyte ((unsigned char *) &strl, 0xaa, sizeof (strl)))
  {
    /* presumably allocated while mlvenabled <= 0 */
    return 0;
  }
  else
  {
    assert (hdr->seq == strl.seq);
    return 1;
  }
}

static void update_stats (struct mlvhdr *hdr, int en, int dir)
{
  assert (dir == 1 || dir == -1);
  if (en)
  {
    stats.current += dir * (int) hdr->sz;
    stats.nblocks += dir;
    if (hdr->sz == 0)
    {
      stats.nzeroblocks += dir;
    }
  }
  else
  {
    /* note: imprecise cos of race condition */
    stats.nwhiledisabled += dir;
  }
}

static void *mlv_malloc (os_size_t osz)
{
  struct mlvhdr *nhdr;
  unsigned sz;
  sz = (unsigned) osz;
  assert (sz < 0x80000000);
  nhdr = malloc (sizeof (struct mlvhdr) + sz + sizeof (struct mlvtrl));
  if (nhdr == NULL)
    return NULL;
  else
  {
    if (mlvenabled <= 0)
    {
      memset (nhdr, 0xaa, sizeof (struct mlvhdr));
      nhdr->sz = sz;
      nhdr->chk = (unsigned) nhdr ^ sz;
      memset ((char *) (nhdr + 1) + sz, 0xaa, sizeof (struct mlvtrl));
      update_stats (nhdr, 0, 1);
    }
    else
    {
      struct mlvtrl strl;
      os_mutexLock (&mlvlock);
      strl.seq = stats.seq++;
      memcpy ((char *) (nhdr + 1) + sz, &strl, sizeof (strl));
#if HAVE_BACKTRACE
      backtrace (&nhdr->trace[-1], TRACE_DEPTH+1); /* <<<=== !!! */
#endif
      nhdr->sz = sz;
      nhdr->chk = (unsigned) nhdr ^ sz;
      nhdr->seq = strl.seq;
      nhdr->plive = NULL;
      nhdr->nlive = live;
      if (live)
        nhdr->nlive->plive = nhdr;
      live = nhdr;
      update_stats (nhdr, 1, 1);
      os_mutexUnlock (&mlvlock);
    }
    return nhdr + 1;
  }
}

static void *mlv_realloc (void *blk, os_size_t sz)
{
  if (blk == NULL)
    return mlv_malloc (sz);
  else
  {
    struct mlvhdr *ohdr = (struct mlvhdr *) blk - 1;
    void *nblk;
    check (ohdr);
    nblk = mlv_malloc (sz);
    if (nblk == NULL)
      return NULL;
    else
    {
      unsigned szmin;
      assert ((unsigned) sz < 0x80000000);
      szmin = ((unsigned) sz < ohdr->sz) ? (unsigned) sz : ohdr->sz;
      memcpy (nblk, blk, szmin);
      mlv_free (blk);
      return nblk;
    }
  }
}

static void mlv_free (void *blk)
{
  struct mlvhdr *ohdr = (struct mlvhdr *) blk - 1;
  int chk = check (ohdr);
  if (mlvenabled > 0)
  {
    if (chk)
    {
      os_mutexLock (&mlvlock);
      update_stats (ohdr, 1, -1);
      if (ohdr->plive)
        ohdr->plive->nlive = ohdr->nlive;
      else
        live = ohdr->nlive;
      if (ohdr->nlive)
        ohdr->nlive->plive = ohdr->plive;
      os_mutexUnlock (&mlvlock);
    }
    else
    {
      update_stats (ohdr, 0, -1);
    }
  }
  else
  {
    if (chk)
      mlvenabled = -1;
    update_stats (ohdr, 0, -1);
  }
  ohdr->chk = ~ohdr->chk;
  free (ohdr);
}
#endif /* USE_MLV */

void mlv_init (void)
{
#if USE_MLV
  os_heapSetService (mlv_malloc, mlv_realloc, mlv_free);
#endif
}

void mlv_setforreal (UNUSED_ARG_NDEBUG (int enable))
{
  assert (enable == 0 || enable == 1);
#if USE_MLV
  if (enable && mlvenabled == 0)
  {
    os_mutexAttr mattr;
    os_mutexAttrInit (&mattr);
    mattr.scopeAttr = OS_SCOPE_PRIVATE;
    os_mutexInit (&mlvlock, &mattr);
    mlvenabled = 1;
  }
  else if (!enable && mlvenabled > 0)
  {
    os_mutexDestroy (&mlvlock);
    mlvenabled = 0;
  }
#endif
}

void mlv_fini (void)
{
#if USE_MLV
  mlv_setforreal (0);
#if 0
  /* Can't restore the malloc/realloc/free functions underlying the
     os_... wrappers to their defaults: the exit handlers will call
     os_free ... */
  os_heapSetService (0, 0, 0);
#endif
#endif
}

void mlv_stats (struct mlv_stats *st)
{
#if USE_MLV
  if (mlvenabled > 0)
  {
    os_mutexLock (&mlvlock);
    *st = stats;
    os_mutexUnlock (&mlvlock);
  }
  else
  {
    *st = stats;
  }
#else
  *st = stats;
#endif
}

#if USE_MLV
void mlv_printlive (os_uint64 seq0, os_uint64 seq1, int (*print) (const char *fmt, ...))
{
  if (mlvenabled > 0)
  {
    struct mlvhdr *hdr;
    os_mutexLock (&mlvlock);
    for (hdr = live; hdr && hdr->seq >= seq1; hdr = hdr->nlive)
      ;
    for (; hdr && hdr->seq >= seq0; hdr = hdr->nlive)
    {
#if HAVE_BACKTRACE
      print ("%p sz %u seq %llu " TRACE_PRINTF_FMT "\n", (void *) (hdr+1), hdr->sz, hdr->seq, TRACE_PRINTF_ARGS (hdr));
#else
      print ("%p sz %u seq %llu\n", (void *) (hdr+1), hdr->sz, hdr->seq);
#endif
    }
    os_mutexUnlock (&mlvlock);
  }
}
#else /* USE_MLV */
void mlv_printlive (UNUSED_ARG (os_uint64 seq0), UNUSED_ARG (os_uint64 seq1), UNUSED_ARG (int (*print) (const char *fmt, ...)))
{
}
#endif /* USE_MLV */
