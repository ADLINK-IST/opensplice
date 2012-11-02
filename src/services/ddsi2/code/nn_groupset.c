#include <ctype.h>
#include <stddef.h>

#include "os_defs.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_thread.h"
#include "os_socket.h"
#include "os_if.h"

#include "v_groupSet.h"
#include "kernelModule.h"

#include "nn_avl.h"
#include "nn_log.h"
#include "nn_xqos.h"
#include "nn_groupset.h"
#include "nn_error.h"

#include "nn_osplser.h"

struct nn_groupset_entry {
  STRUCT_AVLNODE (nn_groupset_entry_avlnode, struct nn_groupset_entry *) avlnode;
  v_group group;
};

struct nn_groupset {
  STRUCT_AVLTREE (nn_groupset_avltree, struct nn_groupset_entry *) grouptree;
};

static int compare_group_ptr (const void *va, const void *vb)
{
  C_STRUCT (v_group) const * const *a = va;
  C_STRUCT (v_group) const * const *b = vb;
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

static int add_group (struct nn_groupset *gs, v_group g)
{
  struct nn_groupset_entry *e;
  avlparent_t parent;
  if ((e = avl_lookup (&gs->grouptree, &g, &parent)) == NULL)
  {
    if ((e = os_malloc (sizeof (*e))) == NULL)
      return NN_ERR_OUT_OF_MEMORY;
    avl_init_node (&e->avlnode, parent);
    e->group = g;
    avl_insert (&gs->grouptree, e);
  }
  return 0;
}

static int add_group_by_name (struct nn_groupset *gs, v_kernel kernel, const char *topic_name, const char *partition)
{
  v_group g;
  if (strchr (partition, '?') || strchr (partition, '*'))
    return 0;
  else if ((g = v_groupSetGet (kernel->groupSet, partition, topic_name)) != NULL)
  {
    int res;
    res = add_group (gs, g);
    c_free (g);
    return res;
  }
  else
  {
    nn_log (LC_TRACE, "add_group_by_name: no kernel group for %s.%s?\n", partition, topic_name);
    return 0;
  }
}

struct nn_groupset *nn_groupset_new (void)
{
  struct nn_groupset *gs;
  if ((gs = os_malloc (sizeof (*gs))) == NULL)
    return NULL;
  avl_init (&gs->grouptree, offsetof (struct nn_groupset_entry, avlnode), offsetof (struct nn_groupset_entry, group), compare_group_ptr, 0, os_free);
  return gs;
}

int nn_groupset_fromqos (struct nn_groupset *gs, v_kernel kernel, const nn_xqos_t *qos)
{
  int res;
  assert (qos->present & QP_TOPIC_NAME);
  if (!(qos->present & QP_PARTITION) || qos->partition.n == 0)
  {
    if ((res = add_group_by_name (gs, kernel, qos->topic_name, "")) < 0)
      return res;
  }
  else
  {
    int i;
    for (i = 0; i < qos->partition.n; i++)
    {
      if ((res = add_group_by_name (gs, kernel, qos->topic_name, qos->partition.strs[i])) < 0)
        return res;
    }
  }
  return 0;
}

void nn_groupset_free (struct nn_groupset *gs)
{
  avl_free (&gs->grouptree);
  os_free (gs);
}

struct nn_groupset_foreach_arg {
  nn_groupset_foreach_t f;
  void *arg;
  int result;
};

static int nn_groupset_foreach_helper (void *vnode, void *varg)
{
  struct nn_groupset_foreach_arg *arg = varg;
  struct nn_groupset_entry *e = vnode;
  if ((arg->result = arg->f (e->group, arg->arg)) < 0)
    return AVLWALK_ABORT;
  else
    return AVLWALK_CONTINUE;
}

int nn_groupset_foreach (struct nn_groupset *gs, nn_groupset_foreach_t f, void *arg)
{
  struct nn_groupset_foreach_arg arg1;
  arg1.f = f;
  arg1.arg = arg;
  arg1.result = 0;
  avl_walk (&gs->grouptree, nn_groupset_foreach_helper, &arg1);
  return arg1.result;
}

static int nn_groupset_add_helper (v_group g, void *varg)
{
  struct nn_groupset *gs = varg;
  return add_group (gs, g);
}

int nn_groupset_add (struct nn_groupset *gs, const struct nn_groupset *add)
{
  return nn_groupset_foreach ((struct nn_groupset *) add, nn_groupset_add_helper, gs);
}

int nn_groupset_add_group (struct nn_groupset *gs, v_group g)
{
  return add_group (gs, g);
}

int nn_groupset_empty (const struct nn_groupset *gs)
{
  return avl_empty (&gs->grouptree);
}
