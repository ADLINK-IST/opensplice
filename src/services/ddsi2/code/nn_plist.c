#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "os_heap.h"

#include "nn_log.h"

#include "nn_bswap.h"
#include "nn_unused.h"
#include "nn_align.h"
#include "nn_error.h"
#include "nn_plist.h"
#include "nn_time.h"
#include "nn_xmsg.h"

#include "nn_config.h"
#include "nn_protocol.h" /* for NN_STATUSINFO_... */
#include "nn_radmin.h" /* for nn_plist_quickscan */

#include "nn_avl.h"
#include "nn_rtps_private.h" /* for vendor_is_... */

#define ENABLE_LOGGING 0

/* These are internal to the parameter list processing. We never
   generate them, and we never want to do see them anywhere outside
   the actual parsing of an incoming parameter list. (There are
   entries in nn_plist, but they are never to be inspected and
   the bits corresponding to them must be 0 except while processing an
   incoming parameter list.) */
#define PPTMP_MULTICAST_IPADDRESS               (1 << 0)
#define PPTMP_DEFAULT_UNICAST_IPADDRESS         (1 << 1)
#define PPTMP_DEFAULT_UNICAST_PORT              (1 << 2)
#define PPTMP_METATRAFFIC_UNICAST_IPADDRESS     (1 << 3)
#define PPTMP_METATRAFFIC_UNICAST_PORT          (1 << 4)
#define PPTMP_METATRAFFIC_MULTICAST_IPADDRESS   (1 << 5)
#define PPTMP_METATRAFFIC_MULTICAST_PORT        (1 << 6)

typedef struct nn_ipaddress_params_tmp {
  unsigned present;

  nn_ipv4address_t multicast_ipaddress;
  nn_ipv4address_t default_unicast_ipaddress;
  nn_port_t default_unicast_port;
  nn_ipv4address_t metatraffic_unicast_ipaddress;
  nn_port_t metatraffic_unicast_port;
  nn_ipv4address_t metatraffic_multicast_ipaddress;
  nn_port_t metatraffic_multicast_port;
} nn_ipaddress_params_tmp_t;

struct dd {
  const char *buf;
  unsigned bufsz;
  unsigned bswap: 1;
  nn_protocol_version_t protocol_version;
  nn_vendorid_t vendorid;
};

struct cdroctetseq {
  int len;
  unsigned char value[1];
};

static int protocol_version_is_newer (nn_protocol_version_t pv)
{
  const nn_protocol_version_t mv = NN_PROTOCOL_VERSION_INITIALIZER;
  return (pv.major < mv.major) ? 0 : (pv.major > mv.major) ? 1 : (pv.minor > mv.minor);
}

static int validate_string (const struct dd *dd)
{
  const struct cdrstring *x = (const struct cdrstring *) dd->buf;
  int len;
  if (dd->bufsz < sizeof (struct cdrstring))
    return NN_ERR_INVALID;
  len = (int) (dd->bswap ? bswap4u (x->length) : x->length);
  if (len < 1 || len > (int) (dd->bufsz - offsetof (struct cdrstring, contents)))
    return NN_ERR_INVALID;
  if (x->contents[len-1] != 0)
    return NN_ERR_INVALID;
  return len;
}

static int alias_string (const char **ptr, const struct dd *dd)
{
  int len;
  if ((len = validate_string (dd)) < 0)
    return len;
  else
  {
    const struct cdrstring *x = (const struct cdrstring *) dd->buf;
    *ptr = x->contents;
    return len;
  }
}

static int unalias_string (char **str, int bswap)
{
  const char *alias = *str;
  int len;
  if (bswap == 0 || bswap == 1)
  {
    const int *plen = (const int *) alias - 1;
    len = bswap ? bswap4 (*plen) : *plen;
  }
  else
  {
    len = strlen (alias) + 1;
  }
  if ((*str = os_malloc (len)) == NULL)
  {
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "UNALIAS_STRING alloc %d fail\n", len);
#endif
    return NN_ERR_OUT_OF_MEMORY;
  }
  else
  {
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "UNALIAS_STRING alloc %p\n", *str);
#endif
    memcpy (*str, alias, len);
    return 0;
  }
}

static int validate_octetseq (const struct dd *dd)
{
  const struct cdroctetseq *x = (const struct cdroctetseq *) dd->buf;
  int len;
  if (dd->bufsz < offsetof (struct cdroctetseq, value))
    return NN_ERR_INVALID;
  len = dd->bswap ? bswap4 (x->len) : x->len;
  if (len < 0 || len > (int) (dd->bufsz - offsetof (struct cdroctetseq, value)))
    return NN_ERR_INVALID;
  return len;
}

static int alias_octetseq (nn_octetseq_t *oseq, const struct dd *dd)
{
  int len;
  if ((len = validate_octetseq (dd)) < 0)
    return len;
  else
  {
    const struct cdroctetseq *x = (const struct cdroctetseq *) dd->buf;
    oseq->length = len;
    oseq->value = (len == 0) ? NULL : (unsigned char *) x->value;
    return len;
  }
}

static int unalias_octetseq (nn_octetseq_t *oseq, UNUSED_ARG (int bswap))
{
  if (oseq->length != 0)
  {
    unsigned char *vs;
    if ((vs = os_malloc (oseq->length)) == NULL)
    {
#if ENABLE_LOGGING
      nn_log (LC_TRACE, "UNALIAS_OCTETSEQ alloc %d fail\n", oseq->length);
#endif
      return NN_ERR_OUT_OF_MEMORY;
    }
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "UNALIAS_OCTETSEQ alloc %p\n", vs);
#endif
    memcpy (vs, oseq->value, oseq->length);
    oseq->value = vs;
  }
  return 0;
}

static int validate_stringseq (const struct dd *dd)
{
  const char *seq = dd->buf;
  const char *seqend = seq + dd->bufsz;
  struct dd dd1 = *dd;
  int i, n;
  if (dd->bufsz < sizeof (int))
    return NN_ERR_INVALID;
  memcpy (&n, seq, sizeof (n));
  if (dd->bswap)
    n = bswap4 (n);
  seq += sizeof (int);
  if (n < 0)
    return NN_ERR_INVALID;
  else if (n == 0)
    return 0;
  else
  {
    for (i = 0; i < n && seq <= seqend; i++)
    {
      int len1;
      dd1.buf = seq;
      dd1.bufsz = seqend - seq;
      if ((len1 = validate_string (&dd1)) < 0)
        return len1;
      seq += sizeof (unsigned) + ALIGN4 (len1);
    }
    if (i < n)
      return NN_ERR_INVALID;
  }
  /* Should I worry about junk between the last string & the end of
     the parameter? */
  return 0;
}

static int alias_stringseq (nn_stringseq_t *strseq, const struct dd *dd)
{
  /* Not truly an alias: it allocates an array of pointers that alias
     the individual null-terminated strings. Also: see
     validate_stringseq */
  const char *seq = dd->buf;
  const char *seqend = seq + dd->bufsz;
  struct dd dd1 = *dd;
  char **strs;
  int i, result;
  if (dd->bufsz < sizeof (int))
    return NN_ERR_INVALID;
  memcpy (&strseq->n, seq, sizeof (strseq->n));
  if (dd->bswap)
    strseq->n = bswap4 (strseq->n);
  seq += sizeof (int);
  if (strseq->n < 0)
    return NN_ERR_INVALID;
  else if (strseq->n == 0)
    strseq->strs = NULL;
  else
  {
    if ((strs = os_malloc (strseq->n * sizeof (*strs))) == NULL)
    {
#if ENABLE_LOGGING
      nn_log (LC_TRACE, "ALIAS_STRINGSEQ alloc %u fail\n", strseq->n * sizeof (*strs));
#endif
      return NN_ERR_OUT_OF_MEMORY;
    }
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "ALIAS_STRINGSEQ alloc %p\n", strs);
#endif
    for (i = 0; i < strseq->n && seq <= seqend; i++)
    {
      int len1;
      dd1.buf = seq;
      dd1.bufsz = seqend - seq;
      /* (const char **) to silence the compiler, unfortunately strseq
         can't have a const char **strs, that would require a const
         and a non-const version of it. */
      if ((len1 = alias_string ((const char **) &strs[i], &dd1)) < 0)
      {
        result = len1;
        goto fail;
      }
      seq += sizeof (unsigned) + ALIGN4 (len1);
    }
    if (i != strseq->n)
    {
      result = NN_ERR_INVALID;
      goto fail;
    }
    strseq->strs = strs;
  }
  return 0;
 fail:
#if ENABLE_LOGGING
  nn_log (LC_TRACE, "ALIAS_STRINGSEQ free %p\n", strs);
#endif
  os_free (strs);
  return result;
}

static void free_stringseq (nn_stringseq_t *strseq)
{
  int i;
  for (i = 0; i < strseq->n; i++)
  {
    if (strseq->strs[i])
    {
#if ENABLE_LOGGING
      nn_log (LC_TRACE, "NN_FREE_STRINGSEQ free %p\n", strseq->strs[i]);
#endif
      os_free (strseq->strs[i]);
    }
  }
#if ENABLE_LOGGING
  nn_log (LC_TRACE, "NN_FREE_STRINGSEQ free %p\n", strseq->strs);
#endif
  os_free (strseq->strs);
}

static int unalias_stringseq (nn_stringseq_t *strseq, int bswap)
{
  int i, res;
  char **strs;
  if (strseq->n != 0)
  {
    if ((strs = os_malloc (strseq->n * sizeof (*strs))) == NULL)
    {
#if ENABLE_LOGGING
      nn_log (LC_TRACE, "UNALIAS_STRINGSEQ alloc %u failed\n", strseq->n * sizeof (*strs));
#endif
      return NN_ERR_OUT_OF_MEMORY;
    }
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "UNALIAS_STRINGSEQ alloc %p\n", strs);
#endif
    for (i = 0; i < strseq->n; i++)
    {
      strs[i] = strseq->strs[i];
      if ((res = unalias_string (&strs[i], bswap)) < 0)
        goto fail;
    }
#if ENABLE_LOGGING
    nn_log (LC_TREE, "UNALIAS_STRINGSEQ free %p\n", strseq->strs);
    os_free (strseq->strs);
#endif
    strseq->strs = strs;
  }
  return 0;
 fail:
  /* free what has been unaliased so far, and zero all pointers to
     prevent confusion, but leave the strs because we can't clear the
     corresponding 'present' bit */
  for (--i; i >= 0; i--)
  {
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "UNALIAS_STRINGSEQ free %p\n", strs[i]);
#endif
    os_free (strs[i]);
  }
#if ENABLE_LOGGING
  nn_log (LC_TRACE, "UNALIAS_STRINGSEQ free %p\n", strs);
#endif
  os_free (strs);
  return res;
}

static int duplicate_stringseq (nn_stringseq_t *dest, const nn_stringseq_t *src)
{
  int i, res;
  dest->n = src->n;
  if (dest->n == 0)
  {
    dest->strs = NULL;
    return 0;
  }
  if ((dest->strs = os_malloc (dest->n * sizeof (*dest->strs))) == NULL)
  {
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "DUPLICATE_STRINGSEQ alloc %u failed\n", dest->n * sizeof (*dest->strs));
#endif
    return NN_ERR_OUT_OF_MEMORY;
  }
#if ENABLE_LOGGING
  nn_log (LC_TRACE, "DUPLICATE_STRINGSEQ alloc %p\n", dest->strs);
#endif
  for (i = 0; i < dest->n; i++)
  {
    dest->strs[i] = src->strs[i];
    if ((res = unalias_string (&dest->strs[i], -1)) < 0)
      goto fail;
  }
  return 0;
 fail:
  /* free what has been duplicateed so far, and zero all pointers to
     prevent confusion, but leave the strs because we can't clear the
     corresponding 'present' bit */
  for (--i; i >= 0; i--)
  {
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "DUPLICATE_STRINGSEQ free %p\n", dest->strs[i]);
#endif
    os_free (dest->strs[i]);
  }
#if ENABLE_LOGGING
  nn_log (LC_TRACE, "DUPLICATE_STRINGSEQ free %p\n", dest->strs);
#endif
  os_free (dest->strs);
  return res;
}

static void free_locators (nn_locators_t *locs)
{
  while (locs->first)
  {
    struct nn_locators_one *l = locs->first;
    locs->first = l->next;
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "FREE_LOCATORS free %p\n", l);
#endif
    os_free (l);
  }
}

static int unalias_locators (nn_locators_t *locs, UNUSED_ARG (int bswap))
{
  nn_locators_t newlocs;
  struct nn_locators_one *lold, **lnew_ptr;
  /* Copy it, without reversing the order. On failure, free the copy,
     on success overwrite *locs. */
  newlocs.n = locs->n;
  newlocs.first = NULL;
  lnew_ptr = &newlocs.first;
  for (lold = locs->first; lold != NULL; lold = lold->next)
  {
    struct nn_locators_one *n;
    if ((n = os_malloc (sizeof (*n))) == NULL)
    {
#if ENABLE_LOGGING
      nn_log (LC_TRACE, "UNALIAS_LOCATORS alloc %u fail\n", sizeof (*n));
#endif
      free_locators (&newlocs);
      return NN_ERR_OUT_OF_MEMORY;
    }
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "UNALIAS_LOCATORS alloc %p\n", n);
#endif
    n->next = NULL;
    n->loc = lold->loc;
    *lnew_ptr = n;
    lnew_ptr = &n->next;
  }
  *locs = newlocs;
  return 0;
}

void nn_plist_fini (nn_plist_t *ps)
{
  struct t { unsigned fl; size_t off; };
  static const struct t simple[] = {
    { PP_ENTITY_NAME, offsetof (nn_plist_t, entity_name) },
  };
  static const struct t locs[] = {
    { PP_UNICAST_LOCATOR, offsetof (nn_plist_t, unicast_locators) },
    { PP_MULTICAST_LOCATOR, offsetof (nn_plist_t, multicast_locators) },
    { PP_DEFAULT_UNICAST_LOCATOR, offsetof (nn_plist_t, default_unicast_locators) },
    { PP_DEFAULT_MULTICAST_LOCATOR, offsetof (nn_plist_t, default_multicast_locators) },
    { PP_METATRAFFIC_UNICAST_LOCATOR, offsetof (nn_plist_t, metatraffic_unicast_locators) },
    { PP_METATRAFFIC_MULTICAST_LOCATOR, offsetof (nn_plist_t, metatraffic_multicast_locators) }
  };
  int i;
#if ENABLE_LOGGING
  nn_log (LC_TRACE, "NN_PLIST_FINI\n");
#endif
  nn_xqos_fini (&ps->qos);
  for (i = 0; i < (int) (sizeof (simple) / sizeof (*simple)); i++)
  {
    if ((ps->present & simple[i].fl) && !(ps->aliased & simple[i].fl))
    {
      void **pp = (void **) ((char *) ps + simple[i].off);
#if ENABLE_LOGGING
      nn_log (LC_TRACE, "NN_PLIST_FINI free %p\n", *pp);
#endif
      os_free (*pp);
    }
  }
  for (i = 0; i < (int) (sizeof (locs) / sizeof (*locs)); i++)
  {
    if ((ps->present & locs[i].fl) && !(ps->aliased & locs[i].fl))
      free_locators ((nn_locators_t *) ((char *) ps + locs[i].off));
  }
  ps->present = 0;
}

int nn_plist_unalias (nn_plist_t *ps)
{
  int bswap = ps->unalias_needs_bswap;
  int res;
#if ENABLE_LOGGING
  nn_log (LC_TRACE, "NN_PLIST_UNALIAS (bswap %d)\n", bswap);
#endif
#define P(name_, func_, field_) do {                                    \
    if ((ps->present & PP_##name_) && (ps->aliased & PP_##name_)) {     \
      if ((res = unalias_##func_ (&ps->field_, bswap)) < 0)             \
        goto fail;                                                      \
      ps->aliased &= ~PP_##name_;                                       \
    }                                                                   \
  } while (0)
  if ((res = nn_xqos_unalias (&ps->qos)) < 0)
    goto fail;
  P (ENTITY_NAME, string, entity_name);
  P (UNICAST_LOCATOR, locators, unicast_locators);
  P (MULTICAST_LOCATOR, locators, multicast_locators);
  P (DEFAULT_UNICAST_LOCATOR, locators, default_unicast_locators);
  P (DEFAULT_MULTICAST_LOCATOR, locators, default_multicast_locators);
  P (METATRAFFIC_UNICAST_LOCATOR, locators, metatraffic_unicast_locators);
  P (METATRAFFIC_MULTICAST_LOCATOR, locators, metatraffic_multicast_locators);
#undef P
  assert (ps->aliased == 0);
  return 0;
 fail:
  nn_plist_fini (ps);
  return res;
}

static int do_octetseq (nn_octetseq_t *dst, unsigned *present, unsigned *aliased, unsigned wanted, unsigned fl, const struct dd *dd)
{
  int res;
  if (!(wanted & fl))
    return NN_STRICT_P ? validate_octetseq (dd) : 0;
  if ((res = alias_octetseq (dst, dd)) >= 0)
  {
    *present |= fl;
    *aliased |= fl;
  }
  return res;
}

static int do_string (char **dst, unsigned *present, unsigned *aliased, unsigned wanted, unsigned fl, const struct dd *dd)
{
  int res;
  if (!(wanted & fl))
    return NN_STRICT_P ? validate_string (dd) : 0;
  if ((res = alias_string ((const char **) dst, dd)) >= 0)
  {
    *present |= fl;
    *aliased |= fl;
  }
  return res;
}

static int do_stringseq (nn_stringseq_t *dst, unsigned *present, unsigned *aliased, unsigned wanted, unsigned fl, const struct dd *dd)
{
  int res;
  if (!(wanted & fl))
    return NN_STRICT_P ? validate_stringseq (dd) : 0;
  if ((res = alias_stringseq (dst, dd)) >= 0)
  {
    *present |= fl;
    *aliased |= fl;
  }
  return res;
}

static void bswap_time (nn_ddsi_time_t *t)
{
  t->seconds = bswap4 (t->seconds);
  t->fraction = bswap4u (t->fraction);
}

static int validate_time (const nn_ddsi_time_t *t)
{
  /* Accepter are zero, positive, infinite or invalid as defined in
     the DDS 2.1 spec, table 9.4. */
  if (t->seconds >= 0)
    return 0;
  else if (t->seconds == -1 && t->fraction == 0xffffffff)
    return 0;
  else
    return NN_ERR_INVALID;
}

#if DDSI_DURATION_ACCORDING_TO_SPEC
static void bswap_duration (nn_duration_t *d)
{
  /* Why o why is a Duration_t used for some things, and a (DDSI) time
     used for other things, where a duration is {sec,nsec} and a time
     is {sec,fraction}? */
  d->sec = bswap4 (d->sec);
  d->nanosec = bswap4 (d->nanosec);
}

static int validate_duration (const nn_duration_t *d)
{
  /* Accepted are zero, positive, infinite or invalid as defined in
     the DDS 1.2 spec. */
  if (d->sec >= 0 && d->nanosec >= 0 && d->nanosec < 1000000000)
    return 0;
  else if (d->sec == (int) 0x7fffffff && d->nanosec == (int) 0x7fffffff)
    return 0;
  else if (d->sec == -1 && d->nanosec == (int) 0xffffffff)
    return 0;
  else
    return NN_ERR_INVALID;
}
#else
static void bswap_duration (nn_duration_t *d)
{
  bswap_time (d);
}

static int validate_duration (const nn_duration_t *d)
{
  return validate_time (d);
}
#endif

static int do_duration (nn_duration_t *q, unsigned *present, unsigned fl, const struct dd *dd)
{
  int res;
  if (dd->bufsz < sizeof (*q))
    return NN_ERR_INVALID;
  memcpy (q, dd->buf, sizeof (*q));
  if (dd->bswap)
    bswap_duration (q);
  if ((res = validate_duration (q)) < 0)
    return res;
  *present |= fl;
  return 0;
}

static void bswap_durability_qospolicy (nn_durability_qospolicy_t *q)
{
  q->kind = bswap4 (q->kind);
}

static int validate_durability_qospolicy (const nn_durability_qospolicy_t *q)
{
  switch (q->kind)
  {
    case NN_VOLATILE_DURABILITY_QOS:
    case NN_TRANSIENT_LOCAL_DURABILITY_QOS:
    case NN_TRANSIENT_DURABILITY_QOS:
    case NN_PERSISTENT_DURABILITY_QOS:
      break;
    default:
      return NN_ERR_INVALID;
  }
  return 0;
}

static void bswap_history_qospolicy (nn_history_qospolicy_t *q)
{
  q->kind = bswap4 (q->kind);
  q->depth = bswap4 (q->depth);
}

static int validate_history_qospolicy (const nn_history_qospolicy_t *q)
{
  /* Validity of history setting and of resource limits are dependent,
     but we don't have access to the resource limits here ... the
     combination can only be validated once all the qos policies have
     been parsed.

     Why is KEEP_LAST n or KEEP_ALL instead of just KEEP_LAST n, with
     n possibly unlimited. */
  switch (q->kind)
  {
    case NN_KEEP_LAST_HISTORY_QOS:
    case NN_KEEP_ALL_HISTORY_QOS:
      break;
    default:
      return NN_ERR_INVALID;
  }
  /* Accept all values for depth if kind = ALL */
  if (q->kind == NN_KEEP_LAST_HISTORY_QOS)
  {
    if (q->depth < 1)
      return NN_ERR_INVALID;
  }
  return 0;
}

static void bswap_resource_limits_qospolicy (nn_resource_limits_qospolicy_t *q)
{
  q->max_samples = bswap4 (q->max_samples);
  q->max_instances = bswap4 (q->max_instances);
  q->max_samples_per_instance = bswap4 (q->max_samples_per_instance);
}

static int validate_resource_limits_qospolicy (const nn_resource_limits_qospolicy_t *q)
{
  const int unlimited = NN_DDS_LENGTH_UNLIMITED;
  /* Note: dependent on history setting as well (see
     validate_history_qospolicy). Verifying only the internal
     consistency of the resource limits. */
  if (q->max_samples < 1 && q->max_samples != unlimited)
    return NN_ERR_INVALID;
  if (q->max_instances < 1 && q->max_instances != unlimited)
    return NN_ERR_INVALID;
  if (q->max_samples_per_instance < 1 && q->max_samples_per_instance != unlimited)
    return NN_ERR_INVALID;
  if (q->max_samples != unlimited && q->max_samples_per_instance != unlimited)
  {
    /* Interpreting 7.1.3.19 as if "unlimited" is meant to mean "don't
       care" and any conditions related to it must be ignored. */
    if (q->max_samples < q->max_samples_per_instance)
      return NN_ERR_INVALID;
  }
  return 0;
}

static int validate_history_and_resource_limits (const nn_history_qospolicy_t *qh, const nn_resource_limits_qospolicy_t *qr)
{
  const int unlimited = NN_DDS_LENGTH_UNLIMITED;
  int res;
  if ((res = validate_history_qospolicy (qh)) < 0)
    return res;
  if ((res = validate_resource_limits_qospolicy (qr)) < 0)
    return res;
  switch (qh->kind)
  {
    case NN_KEEP_ALL_HISTORY_QOS:
#if 0 /* See comment in validate_resource_limits, ref'ing 7.1.3.19 */
      if (qr->max_samples_per_instance != unlimited)
        return NN_ERR_INVALID;
#endif
      break;
    case NN_KEEP_LAST_HISTORY_QOS:
      if (qr->max_samples_per_instance != unlimited && qh->depth > qr->max_samples_per_instance)
        return NN_ERR_INVALID;
      break;
  }
  return 0;
}

static void bswap_durability_service_qospolicy (nn_durability_service_qospolicy_t *q)
{
  bswap_duration (&q->service_cleanup_delay);
  bswap_history_qospolicy (&q->history);
  bswap_resource_limits_qospolicy (&q->resource_limits);
}

static int validate_durability_service_qospolicy (const nn_durability_service_qospolicy_t *q)
{
  int res;
  if ((res = validate_duration (&q->service_cleanup_delay)) < 0)
    return res;
  if ((res = validate_history_and_resource_limits (&q->history, &q->resource_limits)) < 0)
    return res;
  return 0;
}

static void bswap_liveliness_qospolicy (nn_liveliness_qospolicy_t *q)
{
  q->kind = bswap4 (q->kind);
  bswap_duration (&q->lease_duration);
}

static int validate_liveliness_qospolicy (const nn_liveliness_qospolicy_t *q)
{
  switch (q->kind)
  {
    case NN_AUTOMATIC_LIVELINESS_QOS:
    case NN_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
    case NN_MANUAL_BY_TOPIC_LIVELINESS_QOS:
      return validate_duration (&q->lease_duration);
    default:
      return NN_ERR_INVALID;
  }
}

static void bswap_external_reliability_qospolicy (nn_external_reliability_qospolicy_t *qext)
{
  qext->kind = bswap4 (qext->kind);
  bswap_duration (&qext->max_blocking_time);
}

static int validate_xform_reliability_qospolicy (nn_reliability_qospolicy_t *qdst, const nn_external_reliability_qospolicy_t *qext)
{
  qdst->max_blocking_time = qext->max_blocking_time;
  if (NN_PEDANTIC_P)
  {
    switch (qext->kind)
    {
      case NN_PEDANTIC_BEST_EFFORT_RELIABILITY_QOS:
        qdst->kind = NN_BEST_EFFORT_RELIABILITY_QOS;
        return 0;
      case NN_PEDANTIC_RELIABLE_RELIABILITY_QOS:
        qdst->kind = NN_RELIABLE_RELIABILITY_QOS;
        return validate_duration (&qdst->max_blocking_time);
      default:
        return NN_ERR_INVALID;
    }
  }
  else
  {
    switch (qext->kind)
    {
      case NN_INTEROP_BEST_EFFORT_RELIABILITY_QOS:
        qdst->kind = NN_BEST_EFFORT_RELIABILITY_QOS;
        return 0;
      case NN_INTEROP_RELIABLE_RELIABILITY_QOS:
        qdst->kind = NN_RELIABLE_RELIABILITY_QOS;
        return validate_duration (&qdst->max_blocking_time);
      default:
        return NN_ERR_INVALID;
    }
  }
}

static void bswap_destination_order_qospolicy (nn_destination_order_qospolicy_t *q)
{
  q->kind = bswap4 (q->kind);
}

static int validate_destination_order_qospolicy (const nn_destination_order_qospolicy_t *q)
{
  switch (q->kind)
  {
    case NN_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
    case NN_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
      return 0;
    default:
      return NN_ERR_INVALID;
  }
}

static void bswap_ownership_qospolicy (nn_ownership_qospolicy_t *q)
{
  q->kind = bswap4 (q->kind);
}

static int validate_ownership_qospolicy (const nn_ownership_qospolicy_t *q)
{
  switch (q->kind)
  {
    case NN_SHARED_OWNERSHIP_QOS:
    case NN_EXCLUSIVE_OWNERSHIP_QOS:
      return 0;
    default:
      return NN_ERR_INVALID;
  }
}

static void bswap_ownership_strength_qospolicy (nn_ownership_strength_qospolicy_t *q)
{
  q->value = bswap4 (q->value);
}

static int validate_ownership_strength_qospolicy (UNUSED_ARG (const nn_ownership_strength_qospolicy_t *q))
{
  return 1;
}

static void bswap_presentation_qospolicy (nn_presentation_qospolicy_t *q)
{
  q->access_scope = bswap4 (q->access_scope);
}

static int validate_presentation_qospolicy (const nn_presentation_qospolicy_t *q)
{
  switch (q->access_scope)
  {
    case NN_INSTANCE_PRESENTATION_QOS:
    case NN_TOPIC_PRESENTATION_QOS:
    case NN_GROUP_PRESENTATION_QOS:
      break;
    default:
      return NN_ERR_INVALID;
  }
  /* Bools must be 0 or 1, i.e., only the lsb may be set */
  if (q->coherent_access & ~1)
    return NN_ERR_INVALID;
  if (q->ordered_access & ~1)
    return NN_ERR_INVALID;
  /* coherent_access & ordered_access are a bit irrelevant for
     instance presentation qos, but it appears as if their values are
     not prescribed in that case. */
  return 0;
}

static void bswap_transport_priority_qospolicy (nn_transport_priority_qospolicy_t *q)
{
  q->value = bswap4 (q->value);
}

static int validate_transport_priority_qospolicy (UNUSED_ARG (const nn_transport_priority_qospolicy_t *q))
{
  return 1;
}

static int add_locator (nn_locators_t *ls, unsigned *present, unsigned wanted, unsigned fl, const nn_locator_t *loc)
{
  if (wanted & fl)
  {
    struct nn_locators_one *nloc;
    if (!(*present & fl))
    {
      ls->n = 0;
      ls->first = NULL;
      ls->last = NULL;
    }
    if ((nloc = os_malloc (sizeof (*nloc))) == NULL)
    {
#if ENABLE_LOGGING
      nn_log (LC_TRACE, "ADD_LOCATOR malloc %d fail\n", sizeof (*nloc));
#endif
      return NN_ERR_OUT_OF_MEMORY;
    }
#if ENABLE_LOGGING
    nn_log (LC_TRACE, "ADD_LOCATOR malloc %p\n", nloc);
#endif
    nloc->loc = *loc;
    nloc->next = NULL;
    if (ls->first == NULL)
      ls->first = nloc;
    else
    {
      assert (ls->last != NULL);
      ls->last->next = nloc;
    }
    ls->last = nloc;
    ls->n++;
    *present |= fl;
  }
  return 0;
}

static int locator_address_prefix12_zero (const nn_locator_t *loc)
{
  /* loc has has 32 bit ints preceding the address, hence address is
     4-byte aligned; reading char* as unsigneds isn't illegal type
     punning */
  const unsigned *u = (const unsigned *) loc->address;
  return (u[0] == 0 && u[1] == 0 && u[2] == 0);
}

static int locator_address_zero (const nn_locator_t *loc)
{
  /* see locator_address_prefix12_zero */
  const unsigned *u = (const unsigned *) loc->address;
  return (u[0] == 0 && u[1] == 0 && u[2] == 0 && u[3] == 0);
}

static int do_locator (nn_locators_t *ls, unsigned *present, unsigned wanted, unsigned fl, const struct dd *dd)
{
  nn_locator_t loc;
  if (dd->bufsz < sizeof (loc))
    return NN_ERR_INVALID;
  memcpy (&loc, dd->buf, sizeof (loc));
  if (dd->bswap)
  {
    loc.kind = bswap4 (loc.kind);
    loc.port = bswap4 (loc.port);
  }
  switch (loc.kind)
  {
    case NN_LOCATOR_KIND_UDPv4:
      if (loc.port <= 0 || loc.port > 65535)
        return NN_ERR_INVALID;
      if (!locator_address_prefix12_zero (&loc))
        return NN_ERR_INVALID;
      break;
    case NN_LOCATOR_KIND_UDPv6:
      if (loc.port <= 0 || loc.port > 65535)
        return NN_ERR_INVALID;
      break;
    case NN_LOCATOR_KIND_INVALID:
      if (!locator_address_zero (&loc))
        return NN_ERR_INVALID;
      if (loc.port != 0)
        return NN_ERR_INVALID;
      /* silently dropped correctly formatted "invalid" locators. */
      return 0;
    case NN_LOCATOR_KIND_RESERVED:
      /* silently dropped "reserved" locators. */
      return 0;
  }
  return add_locator (ls, present, wanted, fl, &loc);
}

static void locator_from_ipv4address_port (nn_locator_t *loc, const nn_ipv4address_t *a, const nn_port_t *p)
{
  loc->kind = NN_LOCATOR_KIND_UDPv4;
  loc->port = *p;
  memset (loc->address, 0, 12);
  memcpy (loc->address + 12, a, 4);
}

static int do_ipv4address (nn_plist_t *dest, nn_ipaddress_params_tmp_t *dest_tmp, unsigned wanted, unsigned fl, const struct dd *dd)
{
  nn_ipv4address_t *a;
  nn_port_t *p;
  nn_locators_t *ls;
  unsigned fl1, fldest;
  if (dd->bufsz < sizeof (*a))
    return NN_ERR_INVALID;
  switch (fl)
  {
    case PPTMP_MULTICAST_IPADDRESS:
      a = &dest_tmp->multicast_ipaddress;
      p = NULL; /* don't know which port to use ... */
      fl1 = 0;
      fldest = PP_MULTICAST_LOCATOR;
      ls = &dest->multicast_locators;
      break;
    case PPTMP_DEFAULT_UNICAST_IPADDRESS:
      a = &dest_tmp->default_unicast_ipaddress;
      p = &dest_tmp->default_unicast_port;
      fl1 = PPTMP_DEFAULT_UNICAST_PORT;
      fldest = PP_DEFAULT_UNICAST_LOCATOR;
      ls = &dest->unicast_locators;
      break;
    case PPTMP_METATRAFFIC_UNICAST_IPADDRESS:
      a = &dest_tmp->metatraffic_unicast_ipaddress;
      p = &dest_tmp->metatraffic_unicast_port;
      fl1 = PPTMP_METATRAFFIC_UNICAST_PORT;
      fldest = PP_METATRAFFIC_UNICAST_LOCATOR;
      ls = &dest->metatraffic_unicast_locators;
      break;
    case PPTMP_METATRAFFIC_MULTICAST_IPADDRESS:
      a = &dest_tmp->metatraffic_multicast_ipaddress;
      p = &dest_tmp->metatraffic_multicast_port;
      fl1 = PPTMP_METATRAFFIC_MULTICAST_PORT;
      fldest = PP_METATRAFFIC_MULTICAST_LOCATOR;
      ls = &dest->metatraffic_multicast_locators;
      break;
    default:
      abort ();
  }
  memcpy (a, dd->buf, sizeof (*a));
  dest_tmp->present |= fl;
  if ((dest_tmp->present & (fl | fl1)) == (fl | fl1))
  {
    /* If port already known, add corresponding locator and discard
       both address & port from the set of present plist: this
       allows adding another pair. */
    nn_locator_t loc;
    locator_from_ipv4address_port (&loc, a, p);
    dest_tmp->present &= ~(fl | fl1);
    return add_locator (ls, &dest->present, wanted, fldest, &loc);
  }
  else
  {
    return 0;
  }
}

static int do_port (nn_plist_t *dest, nn_ipaddress_params_tmp_t *dest_tmp, unsigned wanted, unsigned fl, const struct dd *dd)
{
  nn_ipv4address_t *a;
  nn_port_t *p;
  nn_locators_t *ls;
  unsigned fl1, fldest;
  if (dd->bufsz < sizeof (*p))
    return NN_ERR_INVALID;
  switch (fl)
  {
    case PPTMP_DEFAULT_UNICAST_PORT:
      a = &dest_tmp->default_unicast_ipaddress;
      p = &dest_tmp->default_unicast_port;
      fl1 = PPTMP_DEFAULT_UNICAST_IPADDRESS;
      fldest = PP_DEFAULT_UNICAST_LOCATOR;
      ls = &dest->unicast_locators;
      break;
    case PPTMP_METATRAFFIC_UNICAST_PORT:
      a = &dest_tmp->metatraffic_unicast_ipaddress;
      p = &dest_tmp->metatraffic_unicast_port;
      fl1 = PPTMP_METATRAFFIC_UNICAST_IPADDRESS;
      fldest = PP_METATRAFFIC_UNICAST_LOCATOR;
      ls = &dest->metatraffic_unicast_locators;
      break;
    case PPTMP_METATRAFFIC_MULTICAST_PORT:
      a = &dest_tmp->metatraffic_multicast_ipaddress;
      p = &dest_tmp->metatraffic_multicast_port;
      fl1 = PPTMP_METATRAFFIC_MULTICAST_IPADDRESS;
      fldest = PP_METATRAFFIC_MULTICAST_LOCATOR;
      ls = &dest->metatraffic_multicast_locators;
      break;
    default:
      abort ();
  }
  memcpy (p, dd->buf, sizeof (*p));
  if (dd->bswap)
    *p = bswap4u (*p);
  if (*p <= 0 || *p > 65535)
    return NN_ERR_INVALID;
  dest_tmp->present |= fl;
  if ((dest_tmp->present & (fl | fl1)) == (fl | fl1))
  {
    /* If port already known, add corresponding locator and discard
       both address & port from the set of present plist: this
       allows adding another pair. */
    nn_locator_t loc;
    locator_from_ipv4address_port (&loc, a, p);
    dest_tmp->present &= ~(fl | fl1);
    return add_locator (ls, &dest->present, wanted, fldest, &loc);
  }
  else
  {
    return 0;
  }
}

static int valid_participant_guid (const nn_guid_t *g, UNUSED_ARG (const struct dd *dd))
{
  /* All 0 is GUID_UNKNOWN, which is a defined GUID */
  if (g->prefix.u[0] == 0 && g->prefix.u[1] == 0 && g->prefix.u[2] == 0)
    return (g->entityid.u != 0) ? NN_ERR_INVALID : 0;
  else if (g->entityid.u == NN_ENTITYID_PARTICIPANT)
    return 0;
  else
    return NN_ERR_INVALID;
}

static int valid_endpoint_guid (const nn_guid_t *g, const struct dd *dd)
{
  /* All 0 is GUID_UNKNOWN, which is a defined GUID */
  if (g->prefix.u[0] == 0 && g->prefix.u[1] == 0 && g->prefix.u[2] == 0)
    return (g->entityid.u != 0) ? NN_ERR_INVALID : 0;
  switch (g->entityid.u & NN_ENTITYID_SOURCE_MASK)
  {
    case NN_ENTITYID_SOURCE_USER:
      switch (g->entityid.u & NN_ENTITYID_KIND_MASK)
      {
        case NN_ENTITYID_KIND_WRITER_WITH_KEY:
        case NN_ENTITYID_KIND_WRITER_NO_KEY:
        case NN_ENTITYID_KIND_READER_NO_KEY:
        case NN_ENTITYID_KIND_READER_WITH_KEY:
          return 0;
        default:
          return !protocol_version_is_newer (dd->protocol_version) ? NN_ERR_INVALID : 0;
      }
    case NN_ENTITYID_SOURCE_BUILTIN:
      switch (g->entityid.u)
      {
        case NN_ENTITYID_SEDP_BUILTIN_TOPIC_WRITER:
        case NN_ENTITYID_SEDP_BUILTIN_TOPIC_READER:
        case NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER:
        case NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER:
        case NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER:
        case NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER:
        case NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER:
        case NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER:
        case NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER:
        case NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER:
          return 0;
        default:
          return !protocol_version_is_newer (dd->protocol_version) ? NN_ERR_INVALID : 0;
      }
    case NN_ENTITYID_SOURCE_VENDOR:
      /* vendor specific: always ok, unless vendor is PrismTech, 'cos we don't do that yet! */
      return is_own_vendor (dd->vendorid) ? NN_ERR_INVALID : 0;
    default:
      return NN_ERR_INVALID;
  }
}

static int do_guid (nn_guid_t *dst, unsigned *present, unsigned fl, int (*valid) (const nn_guid_t *g, const struct dd *dd), const struct dd *dd)
{
  if (dd->bufsz < sizeof (*dst))
    return NN_ERR_INVALID;
  memcpy (dst, dd->buf, sizeof (*dst));
  *dst = nn_ntoh_guid (*dst);
  if (valid (dst, dd) < 0)
  {
    if (fl == PP_PARTICIPANT_GUID && vendor_is_twinoaks (dd->vendorid) &&
        dst->entityid.u == 0 && ! NN_STRICT_P)
    {
      nn_log (LC_WARNING, "plist(vendor %d.%d): rewriting invalid participant guid %x:%x:%x:%x\n",
              dd->vendorid.id[0], dd->vendorid.id[1],
              dst->prefix.u[0], dst->prefix.u[1], dst->prefix.u[2], dst->entityid.u);
      dst->entityid.u = NN_ENTITYID_PARTICIPANT;
    }
    else
    {
      return NN_ERR_INVALID;
    }
  }
  *present |= fl;
  return 0;
}

static void bswap_prismtech_writer_info (nn_prismtech_writer_info_t *wri)
{
  wri->transactionId = bswap4 (wri->transactionId);
  wri->writerGID.systemId = bswap4 (wri->writerGID.systemId);
  wri->writerGID.localId = bswap4 (wri->writerGID.localId);
  wri->writerGID.serial = bswap4 (wri->writerGID.serial);
  wri->writerInstanceGID.systemId = bswap4 (wri->writerInstanceGID.systemId);
  wri->writerInstanceGID.systemId = bswap4 (wri->writerInstanceGID.localId);
  wri->writerInstanceGID.systemId = bswap4 (wri->writerInstanceGID.serial);
}

static int init_one_parameter (nn_plist_t *dest, nn_ipaddress_params_tmp_t *dest_tmp, unsigned pwanted, unsigned qwanted, unsigned short pid, const struct dd *dd)
{
  int res;
  switch (pid)
  {
    case PID_PAD:
    case PID_SENTINEL:
      return 0;

      /* Extended QoS data: */
#define Q(NAME_, name_) case PID_##NAME_:               \
    if (dd->bufsz < sizeof (nn_##name_##_qospolicy_t))  \
      return NN_ERR_INVALID;                            \
    else                                                \
    {                                                   \
      nn_##name_##_qospolicy_t *q = &dest->qos.name_;   \
      memcpy (q, dd->buf, sizeof (*q));                 \
      if (dd->bswap) bswap_##name_##_qospolicy (q);     \
      if ((res = validate_##name_##_qospolicy (q)) < 0) \
        return res;                                     \
      dest->qos.present |= QP_##NAME_;                  \
    }                                                   \
    return 0
      Q (DURABILITY, durability);
      Q (DURABILITY_SERVICE, durability_service);
      Q (LIVELINESS, liveliness);
      Q (DESTINATION_ORDER, destination_order);
      Q (HISTORY, history);
      Q (RESOURCE_LIMITS, resource_limits);
      Q (OWNERSHIP, ownership);
      Q (OWNERSHIP_STRENGTH, ownership_strength);
      Q (PRESENTATION, presentation);
      Q (TRANSPORT_PRIORITY, transport_priority);
#undef Q

      /* PID_RELIABILITY handled differently because it (formally, for
         static typing reasons) has a different type on the network
         than internally, with the transformation between the two
         dependent on wheter we are being pedantic.  If that weren't
         the case, it would've been an ordinary Q (RELIABILITY,
         reliability). */
    case PID_RELIABILITY:
      if (dd->bufsz < sizeof (nn_external_reliability_qospolicy_t))
        return NN_ERR_INVALID;
      else
      {
        nn_reliability_qospolicy_t *q = &dest->qos.reliability;
        nn_external_reliability_qospolicy_t qext;
        memcpy (&qext, dd->buf, sizeof (qext));
        if (dd->bswap)
          bswap_external_reliability_qospolicy (&qext);
        if ((res = validate_xform_reliability_qospolicy (q, &qext)) < 0)
          return res;
        dest->qos.present |= QP_RELIABILITY;
      }
      return 0;

    case PID_TOPIC_NAME:
      return do_string (&dest->qos.topic_name, &dest->qos.present, &dest->qos.aliased, qwanted, QP_TOPIC_NAME, dd);
    case PID_TYPE_NAME:
      return do_string (&dest->qos.type_name, &dest->qos.present, &dest->qos.aliased, qwanted, QP_TYPE_NAME, dd);

    case PID_USER_DATA:
      return do_octetseq (&dest->qos.user_data, &dest->qos.present, &dest->qos.aliased, qwanted, QP_USER_DATA, dd);
    case PID_GROUP_DATA:
      return do_octetseq (&dest->qos.group_data, &dest->qos.present, &dest->qos.aliased, qwanted, QP_GROUP_DATA, dd);
    case PID_TOPIC_DATA:
      return do_octetseq (&dest->qos.topic_data, &dest->qos.present, &dest->qos.aliased, qwanted, QP_TOPIC_DATA, dd);

    case PID_DEADLINE:
      return do_duration (&dest->qos.deadline.deadline, &dest->qos.present, QP_DEADLINE, dd);
    case PID_LATENCY_BUDGET:
      return do_duration (&dest->qos.latency_budget.duration, &dest->qos.present, QP_LATENCY_BUDGET, dd);
    case PID_LIFESPAN:
      return do_duration (&dest->qos.lifespan.duration, &dest->qos.present, QP_LIFESPAN, dd);
    case PID_TIME_BASED_FILTER:
      return do_duration (&dest->qos.time_based_filter.minimum_separation, &dest->qos.present, QP_TIME_BASED_FILTER, dd);

    case PID_PARTITION:
      return do_stringseq (&dest->qos.partition, &dest->qos.present, &dest->qos.aliased, qwanted, QP_PARTITION, dd);

    case PID_READER_DATA_LIFECYCLE: /* PrismTech specific */
      if (dd->bufsz < sizeof (dest->qos.reader_data_lifecycle))
        return NN_ERR_INVALID;
      else
      {
        nn_reader_data_lifecycle_qospolicy_t *q = &dest->qos.reader_data_lifecycle;
        memcpy (q, dd->buf, sizeof (*q));
        if (dd->bswap)
        {
          bswap_duration (&q->autopurge_nowriter_samples_delay);
          bswap_duration (&q->autopurge_disposed_samples_delay);
        }
        if (validate_duration (&q->autopurge_nowriter_samples_delay) < 0 ||
            validate_duration (&q->autopurge_disposed_samples_delay) < 0)
          return NN_ERR_INVALID;
        dest->qos.present |= QP_READER_DATA_LIFECYCLE;
        return 0;
      }
    case PID_WRITER_DATA_LIFECYCLE: /* PrismTech specific */
      if (dd->bufsz < sizeof (dest->qos.writer_data_lifecycle))
        return NN_ERR_INVALID;
      else
      {
        nn_writer_data_lifecycle_qospolicy_t *q = &dest->qos.writer_data_lifecycle;
        memcpy (q, dd->buf, sizeof (*q));
        if (q->autodispose_unregistered_instances & ~1)
          return NN_ERR_INVALID;
        dest->qos.present |= QP_WRITER_DATA_LIFECYCLE;
        return 0;
      }

      /* Other plist */
    case PID_PROTOCOL_VERSION:
      if (dd->bufsz < sizeof (nn_protocol_version_t))
        return NN_ERR_INVALID;
      memcpy (&dest->protocol_version, dd->buf, sizeof (dest->protocol_version));
      if (NN_STRICT_P &&
          (dest->protocol_version.major != dd->protocol_version.major ||
           dest->protocol_version.minor != dd->protocol_version.minor))
        /* Not accepting a submessage advertising a protocol version
           other than that advertised by the message header, unless I
           have good reason to, at least not when being strict. */
        return NN_ERR_INVALID;
      dest->present |= PP_PROTOCOL_VERSION;
      return 0;

    case PID_VENDORID:
      if (dd->bufsz < sizeof (nn_vendorid_t))
        return NN_ERR_INVALID;
      memcpy (&dest->vendorid, dd->buf, sizeof (dest->vendorid));
      if (NN_STRICT_P &&
          (dest->vendorid.id[0] != dd->vendorid.id[0] ||
           dest->vendorid.id[1] != dd->vendorid.id[1]))
        /* see PID_PROTOCOL_VERSION */
        return NN_ERR_INVALID;
      dest->present |= PP_VENDORID;
      return 0;

      /* Locators: there may be lists, so we have to allocate memory for them */
#define XL(NAME_, name_) case PID_##NAME_##_LOCATOR: return do_locator (&dest->name_##_locators, &dest->present, pwanted, PP_##NAME_##_LOCATOR, dd)
      XL (UNICAST, unicast);
      XL (MULTICAST, multicast);
      XL (DEFAULT_UNICAST, default_unicast);
      XL (DEFAULT_MULTICAST, default_multicast);
      XL (METATRAFFIC_UNICAST, metatraffic_unicast);
      XL (METATRAFFIC_MULTICAST, metatraffic_multicast);
#undef XL

      /* IPADDRESS + PORT entries are a nuisance ... I'd prefer
         converting them to locators right away, so that the rest of
         the code only has to deal with locators, but that is
         impossible because the locators require both the address &
         the port to be known.

         The wireshark dissector suggests IPvAdress_t is just the 32
         bits of the IP address but it doesn't say so anywhere
         ... Similarly for ports, but contrary to the expections they
         seem to be 32-bits, too. Apparently in host-endianness.

         And, to be honest, I have no idea what port to use for
         MULTICAST_IPADDRESS ... */
#define XA(NAME_) case PID_##NAME_##_IPADDRESS: return do_ipv4address (dest, dest_tmp, pwanted, PPTMP_##NAME_##_IPADDRESS, dd)
#define XP(NAME_) case PID_##NAME_##_PORT: return do_port (dest, dest_tmp, pwanted, PPTMP_##NAME_##_PORT, dd)
      XA (MULTICAST);
      XA (DEFAULT_UNICAST);
      XP (DEFAULT_UNICAST);
      XA (METATRAFFIC_UNICAST);
      XP (METATRAFFIC_UNICAST);
      XA (METATRAFFIC_MULTICAST);
      XP (METATRAFFIC_MULTICAST);
#undef XP
#undef XA

    case PID_EXPECTS_INLINE_QOS:
      if (dd->bufsz < sizeof (dest->expects_inline_qos))
        return NN_ERR_INVALID;
      dest->expects_inline_qos = dd->buf[0];
      /* boolean: only lsb may be set */
      if (dest->expects_inline_qos & ~1)
        return NN_ERR_INVALID;
      dest->present |= PP_EXPECTS_INLINE_QOS;
      return 0;

    case PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT:
      /* Spec'd as "incremented monotonically" (DDSI 2.1, table 8.13),
         but 32 bits signed is not such a smart choice for that. We'll
         simply accept any value. */
      if (dd->bufsz < sizeof (dest->participant_manual_liveliness_count))
        return NN_ERR_INVALID;
      memcpy (&dest->participant_manual_liveliness_count, dd->buf, sizeof (dest->participant_manual_liveliness_count));
      if (dd->bswap)
        dest->participant_manual_liveliness_count = bswap4 (dest->participant_manual_liveliness_count);
      dest->present |= PP_PARTICIPANT_MANUAL_LIVELINESS_COUNT;
      return 0;

    case PID_PARTICIPANT_LEASE_DURATION:
      return do_duration (&dest->participant_lease_duration, &dest->present, PP_PARTICIPANT_LEASE_DURATION, dd);

    case PID_CONTENT_FILTER_PROPERTY:
      /* FIXME */
      return 0;

    case PID_PARTICIPANT_GUID:
      return do_guid (&dest->participant_guid, &dest->present, PP_PARTICIPANT_GUID, valid_participant_guid, dd);

    case PID_PARTICIPANT_ENTITYID:
    case PID_GROUP_GUID:
    case PID_GROUP_ENTITYID:
      /* DDSI 2.1 table 9.13: reserved for future use */
      return 0;

    case PID_PARTICIPANT_BUILTIN_ENDPOINTS:
      /* FIXME: I assume it is the same as the BUILTIN_ENDPOINT_SET,
         which is the set that DDSI2 has been using so far. */
      /* FALLS THROUGH */
    case PID_BUILTIN_ENDPOINT_SET:
      if (dd->bufsz < sizeof (dest->builtin_endpoint_set))
        return NN_ERR_INVALID;
      memcpy (&dest->builtin_endpoint_set, dd->buf, sizeof (dest->builtin_endpoint_set));
      if (dd->bswap)
        dest->builtin_endpoint_set = bswap4u (dest->builtin_endpoint_set);
      if (NN_STRICT_P && !protocol_version_is_newer (dd->protocol_version) &&
          (dest->builtin_endpoint_set & ~(NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER |
                                          NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR |
                                          NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER |
                                          NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR |
                                          NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER |
                                          NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR |
                                          /* undefined ones: */
                                          NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_ANNOUNCER |
                                          NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_DETECTOR |
                                          NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_ANNOUNCER |
                                          NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_DETECTOR |
                                          /* defined ones again: */
                                          NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER |
                                          NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER)) != 0)
        return NN_ERR_INVALID;
      dest->present |= PP_BUILTIN_ENDPOINT_SET;
      return 0;

    case PID_PROPERTY_LIST:
    case PID_TYPE_MAX_SIZE_SERIALIZED:
      /* FIXME */
      return 0;

    case PID_ENTITY_NAME:
      return do_string (&dest->entity_name, &dest->present, &dest->aliased, pwanted, PP_ENTITY_NAME, dd);

    case PID_KEYHASH:
      if (dd->bufsz < sizeof (dest->keyhash))
        return NN_ERR_INVALID;
      memcpy (&dest->keyhash, dd->buf, sizeof (dest->keyhash));
      dest->present |= PP_KEYHASH;
      return 0;

    case PID_STATUSINFO:
      if (dd->bufsz < sizeof (dest->statusinfo))
        return NN_ERR_INVALID;
      memcpy (&dest->statusinfo, dd->buf, sizeof (dest->statusinfo));
      dest->statusinfo = fromBE4u (dest->statusinfo);
      if (NN_STRICT_P && !protocol_version_is_newer (dd->protocol_version) &&
          (dest->statusinfo & ~(NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER)))
      {
        /* Spec says I may not interpret the reserved bits. But no-one
           may use them in this version of the specification */
        return NN_ERR_INVALID;
      }
      dest->present |= PP_STATUSINFO;
      return 0;

    case PID_CONTENT_FILTER_INFO:
    case PID_COHERENT_SET:
    case PID_DIRECTED_WRITE:
    case PID_ORIGINAL_WRITER_INFO:
      /* FIXME */
      return 0;

    case PID_ENDPOINT_GUID:
      if (NN_PEDANTIC_P && !protocol_version_is_newer (dd->protocol_version))
      {
        /* ENDPOINT_GUID is not specified in the 2.1 standard, so
           reject it: in (really) strict mode we do not accept
           undefined things, even though we are -arguably- supposed to
           ignore it. */
        return NN_ERR_INVALID;
      }
      return do_guid (&dest->endpoint_guid, &dest->present, PP_ENDPOINT_GUID, valid_endpoint_guid, dd);

    case PID_PRISMTECH_ENDPOINT_GUID:
      /* PrismTech specific variant of ENDPOINT_GUID, for strict compliancy */
      if (!vendor_is_prismtech (dd->vendorid))
        return 0;
      else
        return do_guid (&dest->endpoint_guid, &dest->present, PP_ENDPOINT_GUID, valid_endpoint_guid, dd);

    case PID_PRISMTECH_WRITER_INFO: /* PrismTech specific */
      if (!vendor_is_prismtech (dd->vendorid))
        return 0;
      else if (dd->bufsz < sizeof (dest->prismtech_writer_info))
        return NN_ERR_INVALID;
      else
      {
        nn_prismtech_writer_info_t *wri = &dest->prismtech_writer_info;
        memcpy (wri, dd->buf, sizeof (*wri));
        if (dd->bswap)
          bswap_prismtech_writer_info (wri);
        dest->present |= PP_PRISMTECH_WRITER_INFO;
        return 0;
      }

      /* Deprecated ones (used by RTI, but not relevant to DDSI) */
    case PID_PERSISTENCE:
    case PID_TYPE_CHECKSUM:
    case PID_TYPE2_NAME:
    case PID_TYPE2_CHECKSUM:
    case PID_EXPECTS_ACK:
    case PID_MANAGER_KEY:
    case PID_SEND_QUEUE_SIZE:
    case PID_RELIABILITY_ENABLED:
    case PID_VARGAPPS_SEQUENCE_NUMBER_LAST:
    case PID_RECV_QUEUE_SIZE:
    case PID_RELIABILITY_OFFERED:
      return 0;

    default:
      /* Ignore unrecognised parameters (disregarding vendor-specific
         ones, of course) if the protocol version is newer than the
         one implemented, and fail it if it isn't. I know all RFPs say
         to be tolerant in what is accepted, but that is where the
         bugs & the buffer overflows originate! */
      if (pid & PID_UNRECOGNIZED_INCOMPATIBLE_FLAG)
        dest->present |= PP_INCOMPATIBLE;
      else if (pid & PID_VENDORSPECIFIC_FLAG)
        return 0;
      else if (!protocol_version_is_newer (dd->protocol_version) && NN_STRICT_P)
        return NN_ERR_INVALID;
      else
        return 0;
  }

  assert (0);
  return NN_ERR_INVALID;
}

static void default_resource_limits (nn_resource_limits_qospolicy_t *q)
{
  q->max_instances = NN_DDS_LENGTH_UNLIMITED;
  q->max_samples = NN_DDS_LENGTH_UNLIMITED;
  q->max_samples_per_instance = NN_DDS_LENGTH_UNLIMITED;
}

static void default_history (nn_history_qospolicy_t *q)
{
  q->kind = NN_KEEP_LAST_HISTORY_QOS;
  q->depth = 1;
}

void nn_plist_init_empty (nn_plist_t *dest)
{
#ifndef NDEBUG
  memset (dest, 0, sizeof (*dest));
#endif
  dest->present = dest->aliased = 0;
  nn_xqos_init_empty (&dest->qos);
}

static int final_validation (nn_plist_t *dest)
{
    /* Resource limits & history are related, so if only one is given,
     set the other to the default, claim it has been provided &
     validate the combination. They can't be changed afterward, so
     this is a reasonable interpretation. */
  if ((dest->qos.present & QP_HISTORY) && !(dest->qos.present & QP_RESOURCE_LIMITS))
  {
    default_resource_limits (&dest->qos.resource_limits);
    dest->qos.present |= QP_RESOURCE_LIMITS;
  }
  if (!(dest->qos.present & QP_HISTORY) && (dest->qos.present & QP_RESOURCE_LIMITS))
  {
    default_history (&dest->qos.history);
    dest->qos.present |= QP_HISTORY;
  }
  if (dest->qos.present & (QP_HISTORY | QP_RESOURCE_LIMITS))
  {
    int res;
    assert ((dest->qos.present & (QP_HISTORY | QP_RESOURCE_LIMITS)) == (QP_HISTORY | QP_RESOURCE_LIMITS));
    if ((res = validate_history_and_resource_limits (&dest->qos.history, &dest->qos.resource_limits)) < 0)
      return res;
  }
  return 0;
}

int nn_plist_init_frommsg (nn_plist_t *dest, char **nextafterplist, unsigned pwanted, unsigned qwanted, const nn_plist_src_t *src)
{
  const char *pl;
  struct dd dd;
  nn_ipaddress_params_tmp_t dest_tmp;

#ifndef NDEBUG
  memset (dest, 0, sizeof (*dest));
#endif

  assert (src->bufsz >= 0);

  if (nextafterplist)
    *nextafterplist = NULL;
  dd.protocol_version = src->protocol_version;
  dd.vendorid = src->vendorid;
  switch (src->encoding)
  {
    case PL_CDR_LE:
#ifdef PA_LITTLE_ENDIAN
      dd.bswap = 0;
#else
      dd.bswap = 1;
#endif
      break;
    case PL_CDR_BE:
#ifdef PA_LITTLE_ENDIAN
      dd.bswap = 1;
#else
      dd.bswap = 0;
#endif
      break;
    default:
      nn_log (LC_WARNING, "plist(vendor %d.%d): unknown encoding (%d)\n",
              src->vendorid.id[0], src->vendorid.id[1], src->encoding);
      return NN_ERR_INVALID;
  }
  nn_plist_init_empty (dest);
  dest->unalias_needs_bswap = dd.bswap;
  dest_tmp.present = 0;

#if ENABLE_LOGGING
  nn_log (LC_TRACE, "NN_PLIST_INIT (bswap %d)\n", dd.bswap);
#endif

  pl = src->buf;
  while (pl + sizeof (nn_parameter_t) <= src->buf + src->bufsz)
  {
    nn_parameter_t *par = (nn_parameter_t *) pl;
    unsigned short pid, length;
    int res;
    /* swapping header partially based on wireshark dissector
       output, partially on intuition, and in a small part based on
       the spec */
    pid = dd.bswap ? bswap2u (par->parameterid) : par->parameterid;
    length = dd.bswap ? bswap2u (par->length) : par->length;
    if (pid == PID_SENTINEL)
    {
      /* Sentinel terminates list, the length is ignored, DDSI
         9.4.2.11. */
      /*nn_log (LC_TRACE, "%4x PID %x\n", (unsigned) (pl - (char *) hdr), pid);*/
      if ((res = final_validation (dest)) < 0)
      {
        nn_plist_fini (dest);
        return NN_ERR_INVALID;
      }
      else
      {
        pl += sizeof (*par);
        if (nextafterplist)
          *nextafterplist = (char *) pl;
        return 0;
      }
    }
    if (length > src->bufsz - sizeof (*par) - (pl - src->buf))
    {
      nn_log (LC_WARNING, "plist(vendor %d.%d): parameter length %d out of bounds\n",
              src->vendorid.id[0], src->vendorid.id[1], length);
      nn_plist_fini (dest);
      return NN_ERR_INVALID;
    }
    if ((length % 4) != 0) /* DDSI 9.4.2.11 */
    {
      nn_log (LC_WARNING, "plist(vendor %d.%d): parameter length %d mod 4 != 0\n",
              src->vendorid.id[0], src->vendorid.id[1], length);
      nn_plist_fini (dest);
      return NN_ERR_INVALID;
    }

    dd.buf = (const char *) (par + 1);
    dd.bufsz = (unsigned) length;
    if ((res = init_one_parameter (dest, &dest_tmp, pwanted, qwanted, pid, &dd)) < 0)
    {
#if ENABLE_LOGGING
      nn_log (LC_TRACE, "NN_INIT pid %x error\n", pid);
#endif
      nn_plist_fini (dest);
      return res;
    }
    /*nn_log (LC_TRACE, "%4x PID %x len %d\n", (unsigned) (pl - (char *) hdr), pid, length);*/
    pl += sizeof (*par) + length;
  }
  /* If we get here, that means we reached the end of the message
     without encountering a sentinel. That is an error */
  nn_log (LC_WARNING, "plist(vendor %d.%d): invalid parameter list: sentinel missing\n",
          src->vendorid.id[0], src->vendorid.id[1]);
  nn_plist_fini (dest);
  return NN_ERR_INVALID;
}

char *nn_plist_quickscan (struct nn_rsample_info *dest, const struct nn_rmsg *rmsg, const nn_plist_src_t *src)
{
  /* Sets a few fields in dest, returns address of first byte
     following parameter list, or NULL on error.  Most errors will go
     undetected, unlike nn_plist_init_frommsg(). */
  const char *pl;
  assert (src->bufsz >= 0);

  dest->statusinfo = 0;
  dest->pt_wr_info_zoff = NN_OFF_TO_ZOFF (0);
  dest->complex_qos = 0;
  switch (src->encoding)
  {
    case PL_CDR_LE:
#ifdef PA_LITTLE_ENDIAN
      dest->bswap = 0;
#else
      dest->bswap = 1;
#endif
      break;
    case PL_CDR_BE:
#ifdef PA_LITTLE_ENDIAN
      dest->bswap = 1;
#else
      dest->bswap = 0;
#endif
      break;
    default:
      nn_log (LC_WARNING, "plist(vendor %d.%d): quickscan: unknown encoding (%d)\n",
              src->vendorid.id[0], src->vendorid.id[1], src->encoding);
      return NULL;
  }
#if ENABLE_LOGGING
  nn_log (LC_TRACE, "NN_PLIST_QUICKSCAN (bswap %d)\n", dest->bswap);
#endif
  pl = src->buf;
  while (pl + sizeof (nn_parameter_t) <= src->buf + src->bufsz)
  {
    nn_parameter_t *par = (nn_parameter_t *) pl;
    os_ushort pid, length;
    pid = dest->bswap ? bswap2u (par->parameterid) : par->parameterid;
    length = dest->bswap ? bswap2u (par->length) : par->length;
    pl += sizeof (*par);
    if (pid == PID_SENTINEL)
      return (char *) pl;
    if (length > src->bufsz - (pl - src->buf))
    {
      nn_log (LC_WARNING, "plist(vendor %d.%d): quickscan: parameter length %d out of bounds\n",
              src->vendorid.id[0], src->vendorid.id[1], length);
      return NULL;
    }
    if ((length % 4) != 0) /* DDSI 9.4.2.11 */
    {
      nn_log (LC_WARNING, "plist(vendor %d.%d): quickscan: parameter length %d mod 4 != 0\n",
              src->vendorid.id[0], src->vendorid.id[1], length);
      return NULL;
    }
    switch (pid)
    {
      case PID_PAD:
        break;
      case PID_KEYHASH:
        /* couldn't care less about the key hash */
        break;
      case PID_STATUSINFO:
        if (length < 4)
          return NULL;
        else
        {
          unsigned stinfo = fromBE4u (*((unsigned *) pl));
#if (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER) != 3
#error "expected dispose/unregister to be in lowest 2 bits"
#endif
          dest->statusinfo = stinfo & 3;
          if (stinfo & ~3)
            dest->complex_qos = 1;
        }
        break;
      case PID_PRISMTECH_WRITER_INFO:
        if (length < sizeof (nn_prismtech_writer_info_t))
          return NULL;
        dest->pt_wr_info_zoff = NN_OFF_TO_ZOFF (pl - NN_RMSG_PAYLOAD (rmsg));
        break;
      default:
        dest->complex_qos = 1;
        break;
    }
    pl += length;
  }
  /* If we get here, that means we reached the end of the message
     without encountering a sentinel. That is an error */
  nn_log (LC_WARNING, "plist(vendor %d.%d): quickscan: invalid parameter list: sentinel missing\n",
          src->vendorid.id[0], src->vendorid.id[1]);
  return NULL;
}

void nn_plist_extract_wrinfo (nn_prismtech_writer_info_t *wri, const struct nn_rsample_info *sampleinfo, const struct nn_rdata *rdata)
{
  int wrinfo_off = NN_SAMPLEINFO_WRINFO_OFF (sampleinfo);
  assert (wrinfo_off > NN_RDATA_SUBMSG_OFF (rdata));
  assert (wrinfo_off + sizeof (*wri) <= NN_RDATA_PAYLOAD_OFF (rdata));
  memcpy (wri, NN_RMSG_PAYLOADOFF (rdata->rmsg, wrinfo_off), sizeof (*wri));
  if (sampleinfo->bswap)
    bswap_prismtech_writer_info (wri);
}

void nn_xqos_init_empty (nn_xqos_t *dest)
{
#ifndef NDEBUG
  memset (dest, 0, sizeof (*dest));
#endif
  dest->present = dest->aliased = 0;
}

static int xqos_init_default_common (nn_xqos_t *xqos)
{
  nn_xqos_init_empty (xqos);

  xqos->present |= QP_PARTITION;
  xqos->partition.n = 0;
  xqos->partition.strs = NULL;

  xqos->present |= QP_PRESENTATION;
  xqos->presentation.access_scope = NN_INSTANCE_PRESENTATION_QOS;
  xqos->presentation.coherent_access = 0;
  xqos->presentation.ordered_access = 0;

  xqos->present |= QP_DURABILITY;
  xqos->durability.kind = NN_VOLATILE_DURABILITY_QOS;

  xqos->present |= QP_DEADLINE;
  xqos->deadline.deadline = nn_to_ddsi_duration (T_NEVER);

  xqos->present |= QP_LATENCY_BUDGET;
  xqos->latency_budget.duration = nn_to_ddsi_duration (0);

  xqos->present |= QP_LIVELINESS;
  xqos->liveliness.kind = NN_AUTOMATIC_LIVELINESS_QOS;
  xqos->liveliness.lease_duration = nn_to_ddsi_duration (T_NEVER);

  xqos->present |= QP_DESTINATION_ORDER;
  xqos->destination_order.kind = NN_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;

  xqos->present |= QP_HISTORY;
  xqos->history.kind = NN_KEEP_LAST_HISTORY_QOS;
  xqos->history.depth = 1;

  xqos->present |= QP_RESOURCE_LIMITS;
  xqos->resource_limits.max_samples = NN_DDS_LENGTH_UNLIMITED;
  xqos->resource_limits.max_instances = NN_DDS_LENGTH_UNLIMITED;
  xqos->resource_limits.max_samples_per_instance = NN_DDS_LENGTH_UNLIMITED;

  xqos->present |= QP_TRANSPORT_PRIORITY;
  xqos->transport_priority.value = 0;

  xqos->present |= QP_OWNERSHIP;
  xqos->ownership.kind = NN_SHARED_OWNERSHIP_QOS;

  return 0;
}

int nn_xqos_init_default_reader (nn_xqos_t *xqos)
{
  int res;
  if ((res = xqos_init_default_common (xqos)) < 0)
    return res;

  xqos->present |= QP_RELIABILITY;
  xqos->reliability.kind = NN_BEST_EFFORT_RELIABILITY_QOS;

  xqos->present |= QP_TIME_BASED_FILTER;
  xqos->time_based_filter.minimum_separation = nn_to_ddsi_duration (0);

  xqos->present |= QP_READER_DATA_LIFECYCLE;
  xqos->reader_data_lifecycle.autopurge_nowriter_samples_delay = nn_to_ddsi_duration (T_NEVER);
  xqos->reader_data_lifecycle.autopurge_disposed_samples_delay = nn_to_ddsi_duration (T_NEVER);
  return 0;
}

int nn_xqos_init_default_writer (nn_xqos_t *xqos)
{
  int res;
  if ((res = xqos_init_default_common (xqos)) < 0)
    return res;

  xqos->present |= QP_DURABILITY_SERVICE;
  xqos->durability_service.service_cleanup_delay = nn_to_ddsi_duration (0);
  xqos->durability_service.history.kind = NN_KEEP_LAST_HISTORY_QOS;
  xqos->durability_service.history.depth = 1;
  xqos->durability_service.resource_limits.max_samples = NN_DDS_LENGTH_UNLIMITED;
  xqos->durability_service.resource_limits.max_instances = NN_DDS_LENGTH_UNLIMITED;
  xqos->durability_service.resource_limits.max_samples_per_instance = NN_DDS_LENGTH_UNLIMITED;

  xqos->present |= QP_RELIABILITY;
  xqos->reliability.kind = NN_RELIABLE_RELIABILITY_QOS;
  xqos->reliability.max_blocking_time = nn_to_ddsi_duration (100 * T_MILLISECOND);

  xqos->present |= QP_OWNERSHIP_STRENGTH;
  xqos->ownership_strength.value = 0;

  xqos->present |= QP_TRANSPORT_PRIORITY;
  xqos->transport_priority.value = 0;

  xqos->present |= QP_LIFESPAN;
  xqos->lifespan.duration = nn_to_ddsi_duration (T_NEVER);

  xqos->present |= QP_WRITER_DATA_LIFECYCLE;
  xqos->writer_data_lifecycle.autodispose_unregistered_instances = 1;
  return 0;
}

int nn_xqos_mergein_missing (nn_xqos_t *a, const nn_xqos_t *b)
{
  /* Adds QoS's from B to A (duplicating memory) (only those not
     present in A, obviously) */
  int res;

  /* Simple ones (that don't need memory): everything but topic, type,
     partition, {group,topic|user} data */
#define CQ(fl_, name_) do {                                     \
    if (!(a->present & QP_##fl_) && (b->present & QP_##fl_)) {  \
      a->name_ = b->name_;                                      \
      a->present |= QP_##fl_;                                   \
    }                                                           \
  } while (0)
  CQ (PRESENTATION, presentation);
  CQ (DURABILITY, durability);
  CQ (DURABILITY_SERVICE, durability_service);
  CQ (DEADLINE, deadline);
  CQ (LATENCY_BUDGET, latency_budget);
  CQ (LIVELINESS, liveliness);
  CQ (RELIABILITY, reliability);
  CQ (DESTINATION_ORDER, destination_order);
  CQ (HISTORY, history);
  CQ (RESOURCE_LIMITS, resource_limits);
  CQ (TRANSPORT_PRIORITY, transport_priority);
  CQ (LIFESPAN, lifespan);
  CQ (OWNERSHIP, ownership);
  CQ (OWNERSHIP_STRENGTH, ownership_strength);
  CQ (TIME_BASED_FILTER, time_based_filter);
  CQ (READER_DATA_LIFECYCLE, reader_data_lifecycle);
  CQ (WRITER_DATA_LIFECYCLE, writer_data_lifecycle);
#undef CQ

  /* For allocated ones it is Not strictly necessary to use tmp, as
     a->name_ may only be interpreted if the present flag is set, but
     this keeps a clean on failure and may thereby save us from a
     nasty surprise. */
#define CQ(fl_, name_, type_, tmp_type_) do {                   \
    if (!(a->present & QP_##fl_) && (b->present & QP_##fl_)) {  \
      tmp_type_ tmp = b->name_;                                 \
      if ((res = unalias_##type_ (&tmp, -1)) < 0)               \
        return res;                                             \
      a->name_ = tmp;                                           \
      a->present |= QP_##fl_;                                   \
    }                                                           \
  } while (0)
  CQ (GROUP_DATA, group_data, octetseq, nn_octetseq_t);
  CQ (TOPIC_DATA, topic_data, octetseq, nn_octetseq_t);
  CQ (USER_DATA, user_data, octetseq, nn_octetseq_t);
  CQ (TOPIC_NAME, topic_name, string, char *);
  CQ (TYPE_NAME, type_name, string, char *);
#undef CQ
  if (!(a->present & QP_PARTITION) && (b->present & QP_PARTITION))
  {
    if ((res = duplicate_stringseq (&a->partition, &b->partition)) < 0)
      return res;
    a->present |= QP_PARTITION;
  }
  return 0;
}

int nn_xqos_copy (nn_xqos_t *dst, const nn_xqos_t *src)
{
  nn_xqos_init_empty (dst);
  return nn_xqos_mergein_missing (dst, src);
}

int nn_xqos_unalias (nn_xqos_t *xqos)
{
  int res;
#if ENABLE_LOGGING
  nn_log (LC_TRACE, "NN_XQOS_UNALIAS\n");
#endif
#define Q(name_, func_, field_) do {                                    \
    if ((xqos->present & QP_##name_) && (xqos->aliased & QP_##name_)) { \
      if ((res = unalias_##func_ (&xqos->field_, -1)) < 0)              \
        goto fail;                                                      \
      xqos->aliased &= ~QP_##name_;                                     \
    }                                                                   \
  } while (0)
  Q (GROUP_DATA, octetseq, group_data);
  Q (TOPIC_DATA, octetseq, topic_data);
  Q (USER_DATA, octetseq, user_data);
  Q (TOPIC_NAME, string, topic_name);
  Q (TYPE_NAME, string, type_name);
  Q (PARTITION, stringseq, partition);
#undef Q
  assert (xqos->aliased == 0);
  return 0;
 fail:
  nn_xqos_fini (xqos);
  return res;
}

void nn_xqos_fini (nn_xqos_t *xqos)
{
  struct t { unsigned fl; size_t off; };
  static const struct t qos_simple[] = {
    { QP_GROUP_DATA, offsetof (nn_xqos_t, group_data.value) },
    { QP_TOPIC_DATA, offsetof (nn_xqos_t, topic_data.value) },
    { QP_USER_DATA, offsetof (nn_xqos_t, user_data.value) },
    { QP_TOPIC_NAME, offsetof (nn_xqos_t, topic_name) },
    { QP_TYPE_NAME, offsetof (nn_xqos_t, type_name) },
  };
  int i;
#if ENABLE_LOGGING
  nn_log (LC_TRACE, "NN_XQOS_FINI\n");
#endif
  for (i = 0; i < (int) (sizeof (qos_simple) / sizeof (*qos_simple)); i++)
  {
    if ((xqos->present & qos_simple[i].fl) && !(xqos->aliased & qos_simple[i].fl))
    {
      void **pp = (void **) ((char *) xqos + qos_simple[i].off);
#if ENABLE_LOGGING
      nn_log (LC_TRACE, "NN_XQOS_FINI free %p\n", *pp);
#endif
      os_free (*pp);
    }
  }
  if (xqos->present & QP_PARTITION)
  {
    if (!(xqos->aliased & QP_PARTITION))
      free_stringseq (&xqos->partition);
    else
    {
      /* until proper message buffers arrive */
#if ENABLE_LOGGING
      nn_log (LC_TRACE, "NN_PLIST_FINI free %p\n", xqos->partition.strs);
#endif
      os_free (xqos->partition.strs);
    }
  }
  xqos->present = 0;
}

static int octetseqs_differ (const nn_octetseq_t *a, const nn_octetseq_t *b)
{
  return (a->length != b->length || memcmp (a->value, b->value, a->length) != 0);
}

static int durations_differ (const nn_duration_t *a, const nn_duration_t *b)
{
#if DDSI_DURATION_ACCORDING_TO_SPEC
  return (a->sec != b->sec || a->nanosec != b->nanosec);
#else
  return (a->seconds != b->seconds || a->fraction != b->fraction);
#endif
}

static int histories_differ (const nn_history_qospolicy_t *a, const nn_history_qospolicy_t *b)
{
  return (a->kind != b->kind || (a->kind == NN_KEEP_LAST_HISTORY_QOS && a->depth != b->depth));
}

static int resource_limits_differ (const nn_resource_limits_qospolicy_t *a, const nn_resource_limits_qospolicy_t *b)
{
  return (a->max_samples != b->max_samples || a->max_instances != b->max_instances ||
          a->max_samples_per_instance != b->max_samples_per_instance);
}

static int partition_is_default (const nn_partition_qospolicy_t *a)
{
  int i;
  for (i = 0; i < a->n; i++)
    if (strcmp (a->strs[i], "") != 0)
      return 0;
  return 1;
}

static int partitions_equal (const nn_partition_qospolicy_t *a, const nn_partition_qospolicy_t *b)
{
  /* Return true iff (the set a->strs) equals (the set b->strs); that
     is, order doesn't matter. One could argue that "**" and "*" are
     equal, but we're not that precise here. */
  char *statictab[8];
  char **tab;
  int equal = 1;
  int b_is_def;
  if (a->n == 1 && b->n == 1)
    return (strcmp (a->strs[0], b->strs[0]) == 0);
  /* not the trivial case */
  b_is_def = partition_is_default (b);
  if (partition_is_default (a))
    return b_is_def;
  else if (b_is_def)
    return 0;
  /* neither is default, go the expensive route */
  if (a->n < (int) (sizeof (statictab) / sizeof (*statictab)))
    tab = statictab;
  else if ((tab = os_malloc (a->n * sizeof (*tab))) == NULL)
    return -1;
  else
  {
    int i;
    for (i = 0; i < a->n; i++)
      tab[i] = a->strs[i];
    qsort (tab, a->n, sizeof (*tab), (int (*) (const void *, const void *)) strcmp);
    for (i = 0; i < b->n; i++)
      if (bsearch (b->strs[i], tab, a->n, sizeof (*tab), (int (*) (const void *, const void *)) strcmp) == NULL)
      {
        equal = 0;
        goto out;
      }
  }
 out:
  if (tab != statictab)
    os_free (tab);
  return equal;
}

unsigned nn_xqos_delta (const nn_xqos_t *a, const nn_xqos_t *b, unsigned mask)
{
  /* Returns QP_... set for RxO settings where a differs from b; if
     present in a but not in b (or in b but not in a) it counts as a
     difference. */
  unsigned delta = (a->present ^ b->present) & mask;
  unsigned check = (a->present & b->present) & mask;
  if (check & QP_TOPIC_NAME) {
    if (strcmp (a->topic_name, b->topic_name))
      delta |= QP_TOPIC_NAME;
  }
  if (check & QP_TYPE_NAME) {
    if (strcmp (a->type_name, b->type_name))
      delta |= QP_TYPE_NAME;
  }
  if (check & QP_PRESENTATION) {
    if (a->presentation.access_scope != b->presentation.access_scope ||
        a->presentation.coherent_access != b->presentation.coherent_access ||
        a->presentation.ordered_access != b->presentation.ordered_access)
      delta |= QP_PRESENTATION;
  }
  if (check & QP_PARTITION) {
    if (!partitions_equal (&a->partition, &b->partition))
      delta |= QP_PARTITION;
  }
  if (check & QP_GROUP_DATA) {
    if (octetseqs_differ (&a->group_data, &b->group_data))
      delta |= QP_GROUP_DATA;
  }
  if (check & QP_TOPIC_DATA) {
    if (octetseqs_differ (&a->group_data, &b->group_data))
      delta |= QP_TOPIC_DATA;
  }
  if (check & QP_DURABILITY) {
    if (a->durability.kind != b->durability.kind)
      delta |= QP_DURABILITY;
  }
  if (check & QP_DURABILITY_SERVICE)
  {
    const nn_durability_service_qospolicy_t *qa = &a->durability_service;
    const nn_durability_service_qospolicy_t *qb = &b->durability_service;
    if (durations_differ (&qa->service_cleanup_delay, &qb->service_cleanup_delay) ||
        histories_differ (&qa->history, &qb->history) ||
        resource_limits_differ (&qa->resource_limits, &qb->resource_limits))
      delta |= QP_DURABILITY_SERVICE;
  }
  if (check & QP_DEADLINE) {
    if (durations_differ (&a->deadline.deadline, &b->deadline.deadline))
      delta |= QP_DEADLINE;
  }
  if (check & QP_LATENCY_BUDGET) {
    if (durations_differ (&a->latency_budget.duration, &b->latency_budget.duration))
      delta |= QP_LATENCY_BUDGET;
  }
  if (check & QP_LIVELINESS) {
    if (a->liveliness.kind != b->liveliness.kind ||
        durations_differ (&a->liveliness.lease_duration, &b->liveliness.lease_duration))
      delta |= QP_LIVELINESS;
  }
  if (check & QP_RELIABILITY) {
    if (a->reliability.kind != b->reliability.kind ||
        (a->reliability.kind == NN_RELIABLE_RELIABILITY_QOS &&
         durations_differ (&a->reliability.max_blocking_time, &b->reliability.max_blocking_time)))
      delta |= QP_RELIABILITY;
  }
  if (check & QP_DESTINATION_ORDER) {
    if (a->destination_order.kind != b->destination_order.kind)
      delta |= QP_DESTINATION_ORDER;
  }
  if (check & QP_HISTORY) {
    if (histories_differ (&a->history, &b->history))
      delta |= QP_HISTORY;
  }
  if (check & QP_RESOURCE_LIMITS) {
    if (resource_limits_differ (&a->resource_limits, &b->resource_limits))
      delta |= QP_RESOURCE_LIMITS;
  }
  if (check & QP_TRANSPORT_PRIORITY) {
    if (a->transport_priority.value != b->transport_priority.value)
      delta |= QP_TRANSPORT_PRIORITY;
  }
  if (check & QP_LIFESPAN) {
    if (durations_differ (&a->lifespan.duration, &b->lifespan.duration))
      delta |= QP_LIFESPAN;
  }
  if (check & QP_USER_DATA) {
    if (octetseqs_differ (&a->group_data, &b->group_data))
      delta |= QP_USER_DATA;
  }
  if (check & QP_OWNERSHIP) {
    if (a->ownership.kind != b->ownership.kind)
      delta |= QP_OWNERSHIP;
  }
  if (check & QP_OWNERSHIP_STRENGTH) {
    if (a->ownership_strength.value != b->ownership_strength.value)
      delta |= QP_OWNERSHIP_STRENGTH;
  }
  if (check & QP_TIME_BASED_FILTER) {
    if (durations_differ (&a->time_based_filter.minimum_separation, &b->time_based_filter.minimum_separation))
      delta |= QP_TIME_BASED_FILTER;
  }
  if (check & QP_READER_DATA_LIFECYCLE) {
    if (durations_differ (&a->reader_data_lifecycle.autopurge_disposed_samples_delay,
                          &b->reader_data_lifecycle.autopurge_disposed_samples_delay) ||
        durations_differ (&a->reader_data_lifecycle.autopurge_nowriter_samples_delay,
                          &b->reader_data_lifecycle.autopurge_nowriter_samples_delay))
      delta |= QP_READER_DATA_LIFECYCLE;
  }
  if (check & QP_WRITER_DATA_LIFECYCLE) {
    if (a->writer_data_lifecycle.autodispose_unregistered_instances !=
        b->writer_data_lifecycle.autodispose_unregistered_instances)
      delta |= QP_WRITER_DATA_LIFECYCLE;
  }
  return delta;
}

/*************************/

int nn_xqos_addtomsg (struct nn_xmsg *m, const nn_xqos_t *xqos, unsigned wanted)
{
  /* Returns new nn_xmsg pointer (currently, reallocs may happen), or NULL
     on out-of-memory. (In which case the original nn_xmsg is freed, cos
     that is then required anyway */
  unsigned w = xqos->present & wanted;
  char *tmp;
  int res;
#define SIMPLE(name_, field_) do {                                      \
    if (w & QP_##name_) {                                               \
      if ((tmp = nn_xmsg_addpar (m, PID_##name_, sizeof (xqos->field_))) == NULL) \
        return NN_ERR_OUT_OF_MEMORY;                                    \
      *((nn_##field_##_qospolicy_t *) tmp) = xqos->field_;              \
    }                                                                   \
  } while (0)
#define FUNC_GENERIC(name_, field_, func_, ref_) do {                   \
    if (w & QP_##name_) {                                               \
      if ((res = nn_xmsg_addpar_##func_ (m, PID_##name_, ref_ xqos->field_)) < 0) \
        return res;                                                     \
    }                                                                   \
  } while (0)
#define FUNC_BY_VAL(name_, field_, func_) FUNC_GENERIC (name_, field_, func_,)
#define FUNC_BY_REF(name_, field_, func_) FUNC_GENERIC (name_, field_, func_, &)
  FUNC_BY_VAL (TOPIC_NAME, topic_name, string);
  FUNC_BY_VAL (TYPE_NAME, type_name, string);
  SIMPLE (PRESENTATION, presentation);
  FUNC_BY_REF (PARTITION, partition, stringseq);
  FUNC_BY_REF (GROUP_DATA, group_data, octetseq);
  FUNC_BY_REF (TOPIC_DATA, topic_data, octetseq);
  SIMPLE (DURABILITY, durability);
  SIMPLE (DURABILITY_SERVICE, durability_service);
  SIMPLE (DEADLINE, deadline);
  SIMPLE (LATENCY_BUDGET, latency_budget);
  SIMPLE (LIVELINESS, liveliness);
  FUNC_BY_REF (RELIABILITY, reliability, reliability);
  SIMPLE (DESTINATION_ORDER, destination_order);
  SIMPLE (HISTORY, history);
  SIMPLE (RESOURCE_LIMITS, resource_limits);
  SIMPLE (TRANSPORT_PRIORITY, transport_priority);
  SIMPLE (LIFESPAN, lifespan);
  FUNC_BY_REF (USER_DATA, user_data, octetseq);
  SIMPLE (OWNERSHIP, ownership);
  SIMPLE (OWNERSHIP_STRENGTH, ownership_strength);
  SIMPLE (TIME_BASED_FILTER, time_based_filter);
  SIMPLE (READER_DATA_LIFECYCLE, reader_data_lifecycle);
  SIMPLE (WRITER_DATA_LIFECYCLE, writer_data_lifecycle);
#undef FUNC_BY_REF
#undef FUNC_BY_VAL
#undef FUNC_GENERIC
#undef SIMPLE
  return 0;
}

static int add_locators (struct nn_xmsg *m, unsigned present, unsigned flag, const nn_locators_t *ls, int pid)
{
  const struct nn_locators_one *l;
  if (!(present & flag))
    return 0;
  for (l = ls->first; l != NULL; l = l->next)
  {
    char *tmp;
    if ((tmp = nn_xmsg_addpar (m, pid, sizeof (nn_locator_t))) == NULL)
      return NN_ERR_OUT_OF_MEMORY;
    memcpy (tmp, &l->loc, sizeof (nn_locator_t));
  }
  return 0;
}

int nn_plist_addtomsg (struct nn_xmsg *m, const nn_plist_t *ps, unsigned pwanted, unsigned qwanted)
{
  /* Returns new nn_xmsg pointer (currently, reallocs may happen), or NULL
     on out-of-memory. (In which case the original nn_xmsg is freed, cos
     that is then required anyway */
  unsigned w = ps->present & pwanted;
  char *tmp;
  int res;
#define SIMPLE_TYPE(name_, field_, type_) do {                          \
    if (w & PP_##name_) {                                               \
      if ((tmp = nn_xmsg_addpar (m, PID_##name_, sizeof (ps->field_))) == NULL) \
        return NN_ERR_OUT_OF_MEMORY;                                    \
      *((type_ *) tmp) = ps->field_;                                    \
    }                                                                   \
  } while (0)
#define FUNC_GENERIC(name_, field_, func_, ref_) do {                   \
    if (w & PP_##name_) {                                               \
      if ((res = nn_xmsg_addpar_##func_ (m, PID_##name_, ref_ ps->field_)) < 0) \
        return res;                                                     \
    }                                                                   \
  } while (0)
#define FUNC_BY_VAL(name_, field_, func_) FUNC_GENERIC (name_, field_, func_,)
#define FUNC_BY_REF(name_, field_, func_) FUNC_GENERIC (name_, field_, func_, &)
  if ((res = nn_xqos_addtomsg (m, &ps->qos, qwanted)) < 0)
    return res;
  SIMPLE_TYPE (PROTOCOL_VERSION, protocol_version, nn_protocol_version_t);
  SIMPLE_TYPE (VENDORID, vendorid, nn_vendorid_t);

  if ((res = add_locators (m, ps->present, PP_UNICAST_LOCATOR, &ps->unicast_locators, PID_UNICAST_LOCATOR)) < 0)
    return res;
  if ((res = add_locators (m, ps->present, PP_MULTICAST_LOCATOR, &ps->multicast_locators, PID_MULTICAST_LOCATOR)) < 0)
    return res;
  if ((res = add_locators (m, ps->present, PP_DEFAULT_UNICAST_LOCATOR, &ps->default_unicast_locators, PID_DEFAULT_UNICAST_LOCATOR)) < 0)
    return res;
  if ((res = add_locators (m, ps->present, PP_DEFAULT_MULTICAST_LOCATOR, &ps->default_multicast_locators, PID_DEFAULT_MULTICAST_LOCATOR)) < 0)
    return res;
  if ((res = add_locators (m, ps->present, PP_METATRAFFIC_UNICAST_LOCATOR, &ps->metatraffic_unicast_locators, PID_METATRAFFIC_UNICAST_LOCATOR)) < 0)
    return res;
  if ((res = add_locators (m, ps->present, PP_METATRAFFIC_MULTICAST_LOCATOR, &ps->metatraffic_multicast_locators, PID_METATRAFFIC_MULTICAST_LOCATOR)) < 0)
    return res;

  SIMPLE_TYPE (EXPECTS_INLINE_QOS, expects_inline_qos, char);
  SIMPLE_TYPE (PARTICIPANT_LEASE_DURATION, participant_lease_duration, nn_duration_t);
  FUNC_BY_REF (PARTICIPANT_GUID, participant_guid, guid);
  SIMPLE_TYPE (BUILTIN_ENDPOINT_SET, builtin_endpoint_set, unsigned);
  SIMPLE_TYPE (KEYHASH, keyhash, nn_keyhash_t);
  FUNC_BY_VAL (STATUSINFO, statusinfo, BE4u);
  if (! NN_PEDANTIC_P)
    FUNC_BY_REF (ENDPOINT_GUID, endpoint_guid, guid);
  else
  {
    if (w & PP_ENDPOINT_GUID)
    {
      if ((res = nn_xmsg_addpar_guid (m, PID_PRISMTECH_ENDPOINT_GUID, &ps->endpoint_guid)) < 0)
        return res;
    }
  }
  SIMPLE_TYPE (PRISMTECH_WRITER_INFO, prismtech_writer_info, nn_prismtech_writer_info_t);
#undef FUNC_BY_REF
#undef FUNC_BY_VAL
#undef FUNC_GENERIC
#undef SIMPLE
  return 0;
}

/*************************/

void nn_logb_xqos (logbuf_t lb, logcat_t cat, const nn_xqos_t *xqos)
{
  unsigned p = xqos->present;
  const char *prefix = "";
#define LOGB0(fmt_) nn_logb (lb, cat, "%s" fmt_, prefix)
#define LOGB1(fmt_, arg0_) nn_logb (lb, cat, "%s" fmt_, prefix, arg0_)
#define LOGB2(fmt_, arg0_, arg1_) nn_logb (lb, cat, "%s" fmt_, prefix, arg0_, arg1_)
#define LOGB3(fmt_, arg0_, arg1_, arg2_) nn_logb (lb, cat, "%s" fmt_, prefix, arg0_, arg1_, arg2_)
#define DO(name_, body_) do { if (p & QP_##name_) { { body_ } prefix = ","; } } while (0)

#if DDSI_DURATION_ACCORDING_TO_SPEC
#define FMT_DUR "%d.%09d"
#define PRINTARG_DUR(d) (d).sec, (d).nanosec
#else
#define FMT_DUR "%d.%09d"
#define PRINTARG_DUR(d) (d).seconds, (int) ((d).fraction/4.294967296)
#endif

  DO (TOPIC_NAME, { LOGB1 ("topic=%s", xqos->topic_name); });
  DO (TYPE_NAME, { LOGB1 ("type=%s", xqos->type_name); });
  DO (PRESENTATION, { LOGB3 ("presentation=%d:%d:%d", xqos->presentation.access_scope, xqos->presentation.coherent_access, xqos->presentation.ordered_access); });
  DO (PARTITION, {
      int i;
      LOGB0 ("partition={");
      for (i = 0; i < xqos->partition.n; i++) {
        nn_logb (lb, cat, "%s%s", (i == 0) ? "" : ",", xqos->partition.strs[i]);
      }
      nn_logb (lb, cat, "}");
    });
  DO (GROUP_DATA, { LOGB1 ("group_data=%d<...>", xqos->group_data.length); });
  DO (TOPIC_DATA, { LOGB1 ("topic_data=%d<...>", xqos->topic_data.length); });
  DO (DURABILITY, { LOGB1 ("durability=%d", xqos->durability.kind); });
  DO (DURABILITY_SERVICE, {
      LOGB0 ("durability_service=");
      nn_logb (lb, cat, FMT_DUR, PRINTARG_DUR (xqos->durability_service.service_cleanup_delay));
      nn_logb (lb, cat, ":{%d:%d}", xqos->durability_service.history.kind, xqos->durability_service.history.depth);
      nn_logb (lb, cat, ":{%d:%d:%d}", xqos->durability_service.resource_limits.max_samples, xqos->durability_service.resource_limits.max_instances, xqos->durability_service.resource_limits.max_samples_per_instance);
    });
  DO (DEADLINE, { LOGB1 ("deadline="FMT_DUR, PRINTARG_DUR (xqos->deadline.deadline)); });
  DO (LATENCY_BUDGET, { LOGB1 ("latency_budget="FMT_DUR, PRINTARG_DUR (xqos->latency_budget.duration)); });
  DO (LIVELINESS, { LOGB2 ("liveliness=%d:"FMT_DUR, xqos->liveliness.kind, PRINTARG_DUR (xqos->liveliness.lease_duration)); });
  DO (RELIABILITY, { LOGB2 ("reliability=%d:"FMT_DUR, xqos->reliability.kind, PRINTARG_DUR (xqos->reliability.max_blocking_time)); });
  DO (DESTINATION_ORDER, { LOGB1 ("destination_order=%d", xqos->destination_order.kind); });
  DO (HISTORY, { LOGB2 ("history=%d:%d", xqos->history.kind, xqos->history.depth); });
  DO (RESOURCE_LIMITS, { LOGB3 ("resource_limits=%d:%d:%d", xqos->resource_limits.max_samples, xqos->resource_limits.max_instances, xqos->resource_limits.max_samples_per_instance); });
  DO (TRANSPORT_PRIORITY, { LOGB1 ("transport_priority=%d", xqos->transport_priority.value); });
  DO (LIFESPAN, { LOGB1 ("lifespan="FMT_DUR, PRINTARG_DUR (xqos->lifespan.duration)); });
  DO (USER_DATA, { LOGB1 ("user_data=%d<...>", xqos->user_data.length); });
  DO (OWNERSHIP, { LOGB1 ("ownership=%d", xqos->ownership.kind); });
  DO (OWNERSHIP_STRENGTH, { LOGB1 ("ownership_strength=%d", xqos->ownership_strength.value); });
  DO (TIME_BASED_FILTER, { LOGB1 ("time_based_filter="FMT_DUR, PRINTARG_DUR (xqos->time_based_filter.minimum_separation)); });
  DO (READER_DATA_LIFECYCLE, { LOGB2 ("reader_data_lifecycle="FMT_DUR":"FMT_DUR, PRINTARG_DUR (xqos->reader_data_lifecycle.autopurge_nowriter_samples_delay), PRINTARG_DUR (xqos->reader_data_lifecycle.autopurge_disposed_samples_delay)); });
  DO (WRITER_DATA_LIFECYCLE, { LOGB1 ("writer_data_lifecycle=%d", xqos->writer_data_lifecycle.autodispose_unregistered_instances); });

#undef PRINTARG_DUR
#undef FMT_DUR
#undef DO
#undef LOGB3
#undef LOGB2
#undef LOGB1
#undef LOGB0
}
