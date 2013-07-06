#include <stddef.h>
#include <limits.h>
#include <assert.h>

#include "q_fibheap.h"

/* max degree: n >= F_{d+2} >= \phi^d ==> d <= log_\phi n, where \phi
   (as usual) is the golden ratio ~= 1.618.  We know n <= (size of
   address space) / sizeof (fh_node), log_\phi 2 ~= 1.44, sizeof
   (fh_node) >= 4, therefore max degree < log_2 (size of address
   space). */
#define MAX_DEGREE ((unsigned) (sizeof (void *) * CHAR_BIT - 1))

static int cmp (struct fibheap *fh, const struct fibheap_node *a, const struct fibheap_node *b)
{
  return fh->cmp ((const char *) a - fh->offset, (const char *) b - fh->offset);
}

void fh_init (struct fibheap *fh, os_address offset, int (*cmp) (const void *va, const void *vb))
{
  fh->roots = NULL;
  fh->offset = offset;
  fh->cmp = cmp;
}

void *fh_min (const struct fibheap *fh)
{
  if (fh->roots)
    return (void *) ((char *) fh->roots - fh->offset);
  else
    return NULL;
}

static void fh_merge_nonempty_list (struct fibheap_node **markptr, struct fibheap_node *list)
{
  assert (list != NULL);

  if (*markptr == NULL)
    *markptr = list;
  else
  {
    struct fibheap_node * const mark = *markptr;
    struct fibheap_node * const old_mark_next = mark->next;
    struct fibheap_node * const old_list_prev = list->prev;
    mark->next = list;
    old_mark_next->prev = old_list_prev;
    list->prev = mark;
    old_list_prev->next = old_mark_next;
  }
}

static void fh_merge_into (struct fibheap *a, struct fibheap_node * const br)
{
  if (br == NULL)
    return;
  else if (a->roots == NULL)
    a->roots = br;
  else
  {
    const int c = cmp (a, br, a->roots);
    fh_merge_nonempty_list (&a->roots, br);
    if (c < 0)
      a->roots = br;
  }
}

void fh_merge (struct fibheap *a, struct fibheap *b)
{
  /* merges nodes from b into a, thereafter, b is empty */
  assert (a->offset == b->offset);
  assert (a->cmp == b->cmp);
  fh_merge_into (a, b->roots);
  b->roots = NULL;
}

void fh_insert (struct fibheap *fh, const void *vnode)
{
  /* fibheap node is opaque => nothing in node changes as far as
     caller is concerned => declare as const argument, then drop the
     const qualifier */
  struct fibheap_node *node = (struct fibheap_node *) ((char *) vnode + fh->offset);

  /* new heap of degree 0 (i.e., only containing NODE) */
  node->parent = node->children = NULL;
  node->prev = node->next = node;
  node->mark = 0;
  node->degree = 0;

  /* then merge it in */
  fh_merge_into (fh, node);
}

static void fh_add_as_child (struct fibheap_node *parent, struct fibheap_node *child)
{
  parent->degree++;
  child->parent = parent;
  child->prev = child->next = child;
  fh_merge_nonempty_list (&parent->children, child);
}

static void fh_delete_one_from_list (struct fibheap_node **markptr, struct fibheap_node *node)
{
  if (node->next == node)
    *markptr = NULL;
  else
  {
    struct fibheap_node * const node_prev = node->prev;
    struct fibheap_node * const node_next = node->next;
    node_prev->next = node_next;
    node_next->prev = node_prev;
    if (*markptr == node)
      *markptr = node_next;
  }
}

void *fh_extractmin (struct fibheap *fh)
{
  struct fibheap_node *roots[MAX_DEGREE + 1];
  struct fibheap_node * const min = fh->roots;
  unsigned min_degree_noninit = 0;

  /* empty heap => return that, alternative would be to require the
     heap to contain at least one element, but this is probably nicer
     in practice */
  if (min == NULL)
    return NULL;

  /* singleton heap => no work remaining */
  if (min->next == min && min->children == NULL)
  {
    fh->roots = NULL;
    return (void *) ((char *) min - fh->offset);
  }

  /* remove min from fh->roots */
  fh_delete_one_from_list (&fh->roots, min);

  /* FIXME: can speed up by combining a few things & improving
     locality of reference by scanning lists only once */

  /* insert min'schildren as new roots -- must fix parent pointers,
     and reset marks because roots are always unmarked */
  if (min->children)
  {
    struct fibheap_node * const mark = min->children;
    struct fibheap_node *n = mark;
    do {
      n->parent = NULL;
      n->mark = 0;
      n = n->next;
    } while (n != mark);

    fh_merge_nonempty_list (&fh->roots, min->children);
  }

  /* iteratively merge roots of equal degree, completely messing up
     fh->roots, ... */
  {
    struct fibheap_node *const mark = fh->roots;
    struct fibheap_node *n = mark;
    do {
      struct fibheap_node * const n1 = n->next;

      /* if n is first root with this high a degree, there's certainly
         not going to be another root to merge with yet */
      while (n->degree < min_degree_noninit && roots[n->degree])
      {
        unsigned const degree = n->degree;
        struct fibheap_node *u, *v;

        if (cmp (fh, roots[degree], n) < 0) {
          u = roots[degree]; v = n;
        } else {
          u = n; v = roots[degree];
        }
        roots[degree] = NULL;
        fh_add_as_child (u, v);
        n = u;
      }

      /* n may have changed, hence need to retest whether or not
         enough of roots has been initialised -- note that
         initialising roots[n->degree] is unnecessary, but easier */
      assert (n->degree <= MAX_DEGREE);
      while (min_degree_noninit <= n->degree)
        roots[min_degree_noninit++] = NULL;

      roots[n->degree] = n;
      n = n1;
    } while (n != mark);
  }

  /* ... but we don't mind because we have roots[], we can scan linear
     memory at an astonishing rate, and we need to compare the root
     keys anyway to find the minimum */
  {
    struct fibheap_node *mark, *cursor, *newmin;
    unsigned i;
    for (i = 0; roots[i] == NULL; i++)
      assert (i+1 < min_degree_noninit);
    newmin = roots[i];
    assert (newmin != NULL);
    mark = cursor = roots[i];
    for (++i; i < min_degree_noninit; i++)
      if (roots[i])
      {
        struct fibheap_node * const r = roots[i];
        if (cmp (fh, r, newmin) < 0)
          newmin = r;
        r->prev = cursor;
        cursor->next = r;
        cursor = r;
      }
    mark->prev = cursor;
    cursor->next = mark;

    fh->roots = newmin;
  }

  return (void *) ((char *) min - fh->offset);
}

static void fh_cutnode (struct fibheap *fh, struct fibheap_node *node)
{
  /* by marking the node, we ensure it gets cut */
  node->mark = 1;

  /* traverse towards the root, cutting marked nodes on the way */
  while (node->parent && node->mark)
  {
    struct fibheap_node *parent = node->parent;

    assert (parent->degree > 0);
    fh_delete_one_from_list (&parent->children, node);
    parent->degree--;

    node->mark = 0;
    node->parent = NULL;
    node->next = node->prev = node;

    /* we assume heap properties haven't been violated, and therefore
       none of the nodes we cut can become the new minimum */
    fh_merge_nonempty_list (&fh->roots, node);

    node = parent;
  }

  /* if we stopped because we hit an unmarked interior node, we must
     mark it */
  if (node->parent)
    node->mark = 1;
}

void fh_decreasekey (struct fibheap *fh, const void *vnode)
{
  /* fibheap node is opaque => nothing in node changes as far as
     caller is concerned => declare as const argument, then drop the
     const qualifier */
  struct fibheap_node *node = (struct fibheap_node *) ((char *) vnode + fh->offset);

  if (node->parent && cmp (fh, node->parent, node) <= 0)
  {
    /* heap property not violated, do nothing */
  }
  else
  {
    if (node->parent)
    {
      /* heap property violated by decreasing the key, but we cut it
         pretending nothing has happened yet, then fix up the minimum if
         this node is the new minimum */
      fh_cutnode (fh, node);
    }
    if (cmp (fh, node, fh->roots) < 0)
    {
      fh->roots = node;
    }
  }
}

void fh_delete (struct fibheap *fh, const void *vnode)
{
  /* fibheap node is opaque => nothing in node changes as far as
     caller is concerned => declare as const argument, then drop the
     const qualifier */
  struct fibheap_node *node = (struct fibheap_node *) ((char *) vnode + fh->offset);

  /* essentially decreasekey(node);extractmin while pretending the
     node key is -infinity.  That means we can't directly call
     decreasekey, because it considers the actual value of the key. */
  if (node->parent != NULL)
    fh_cutnode (fh, node);
  fh->roots = node;
  fh_extractmin (fh);
}

/* SHA1 not available (unoffical build.) */
