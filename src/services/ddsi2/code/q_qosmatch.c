#include <string.h>
#include <assert.h>

#include "q_time.h"
#include "q_xqos.h"
#include "q_qosmatch.h"

int is_wildcard_partition (const char *str)
{
  return strchr (str, '*') || strchr (str, '?');
}

static int partition_patmatch_p (const char *pat, const char *name)
{
  /* pat may be a wildcard expression, name must not be */
  if (!is_wildcard_partition (pat))
    /* no wildcard in pat => must equal name */
    return (strcmp (pat, name) == 0);
  else if (is_wildcard_partition (name))
    /* (we know: wildcard in pat) => wildcard in name => no match */
    return 0;
  else
  {
    /* quick hack: pattern matcher blatantly stolen from the kernel */
    const char *nameRef = NULL;
    const char *ptnRef = NULL;
    while (*name != 0 && *pat != 0)
    {
      if (*pat == '*')
      {
        pat++;
        while (*name != 0 && *name != *pat)
          name++;
        if (*name != 0)
        {
          nameRef = name+1;
          ptnRef = pat-1;
        }
      }
      else if (*pat == '?')
      {
        pat++;
        name++;
      }
      else if (*pat++ != *name++)
      {
        if (nameRef == NULL)
          return 0;
        name = nameRef;
        pat = ptnRef;
        nameRef = NULL;
      }
    }
    if (*name != 0)
      return 0;
    else
    {
      while (*pat == '*')
        pat++;
      return (*pat == 0);
    }
  }
}

static int partitions_match_default (const nn_xqos_t *x)
{
  int i;
  if (!(x->present & QP_PARTITION) || x->partition.n == 0)
    return 1;
  for (i = 0; i < x->partition.n; i++)
    if (partition_patmatch_p (x->partition.strs[i], ""))
      return 1;
  return 0;
}

int partitions_match_p (const nn_xqos_t *a, const nn_xqos_t *b)
{
  if (!(a->present & QP_PARTITION) || a->partition.n == 0)
    return partitions_match_default (b);
  else if (!(b->present & QP_PARTITION) || b->partition.n == 0)
    return partitions_match_default (a);
  else
  {
    int i, j;
    for (i = 0; i < a->partition.n; i++)
      for (j = 0; j < b->partition.n; j++)
      {
        if (partition_patmatch_p (a->partition.strs[i], b->partition.strs[j]) ||
            partition_patmatch_p (b->partition.strs[j], a->partition.strs[i]))
          return 1;
      }
    return 0;
  }
}

int partition_match_based_on_wildcard_in_left_operand (const nn_xqos_t *a, const nn_xqos_t *b, const char **realname)
{
  assert (partitions_match_p (a, b));
  if (!(a->present & QP_PARTITION) || a->partition.n == 0)
  {
    return 0;
  }
  else if (!(b->present & QP_PARTITION) || b->partition.n == 0)
  {
    /* Either A explicitly includes the default partition, or it is a
       wildcard that matches it */
    int i;
    for (i = 0; i < a->partition.n; i++)
      if (strcmp (a->partition.strs[i], "") == 0)
        return 0;
    *realname = "";
    return 1;
  }
  else
  {
    int i, j, maybe_yes = 0;
    for (i = 0; i < a->partition.n; i++)
      for (j = 0; j < b->partition.n; j++)
      {
        if (partition_patmatch_p (a->partition.strs[i], b->partition.strs[j]))
        {
          if (!is_wildcard_partition (a->partition.strs[i]))
            return 0;
          else
          {
            *realname = b->partition.strs[j];
            maybe_yes = 1;
          }
        }
      }
    return maybe_yes;
  }
}

static int ddsi_duration_is_lt (nn_duration_t a0, nn_duration_t b0)
{
  /* inf counts as <= inf */
  const os_int64 a = nn_from_ddsi_duration (a0);
  const os_int64 b = nn_from_ddsi_duration (b0);
  if (a == T_NEVER)
    return 0;
  else if (b == T_NEVER)
    return 1;
  else
    return a < b;
}

int qos_match_p (const nn_xqos_t *rd, const nn_xqos_t *wr)
{
#ifndef NDEBUG
  unsigned musthave = (QP_RXO_MASK | QP_PARTITION | QP_TOPIC_NAME | QP_TYPE_NAME);
  assert ((rd->present & musthave) == musthave);
  assert ((wr->present & musthave) == musthave);
#endif
  if (rd->relaxed_qos_matching.value || wr->relaxed_qos_matching.value)
  {
    if (rd->reliability.kind != wr->reliability.kind)
      return 0;
  }
  else
  {
    if (rd->reliability.kind > wr->reliability.kind)
      return 0;
    if (rd->durability.kind > wr->durability.kind)
      return 0;
    if (rd->presentation.access_scope > wr->presentation.access_scope)
      return 0;
    if (rd->presentation.coherent_access > wr->presentation.coherent_access)
      return 0;
    if (rd->presentation.ordered_access > wr->presentation.ordered_access)
      return 0;
    if (ddsi_duration_is_lt (rd->deadline.deadline, wr->deadline.deadline))
      return 0;
    if (ddsi_duration_is_lt (rd->latency_budget.duration, wr->latency_budget.duration))
      return 0;
    if (rd->ownership.kind != wr->ownership.kind)
      return 0;
    if (rd->liveliness.kind > wr->liveliness.kind)
      return 0;
    if (ddsi_duration_is_lt (rd->liveliness.lease_duration, wr->liveliness.lease_duration))
      return 0;
    if (rd->destination_order.kind > wr->destination_order.kind)
      return 0;
  }
  if (strcmp (rd->topic_name, wr->topic_name) != 0)
    return 0;
  if (strcmp (rd->type_name, wr->type_name) != 0)
    return 0;
  if (!partitions_match_p (rd, wr))
    return 0;
  return 1;
}

/* SHA1 not available (unoffical build.) */
