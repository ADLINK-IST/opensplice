#ifndef Q_FIBHEAP_H
#define Q_FIBHEAP_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct fibheap_node {
  struct fibheap_node *parent, *children;
  struct fibheap_node *prev, *next;
  unsigned mark: 1;
  unsigned degree: 31;
};

struct fibheap {
  struct fibheap_node *roots; /* points to root with min key value */
  os_address offset;
  int (*cmp) (const void *va, const void *vb);
};

void fh_init (struct fibheap *fh, os_address offset, int (*cmp) (const void *va, const void *vb));
void *fh_min (const struct fibheap *fh);
void fh_merge (struct fibheap *a, struct fibheap *b);
void fh_insert (struct fibheap *fh, const void *vnode);
void fh_delete (struct fibheap *fh, const void *vnode);
void *fh_extractmin (struct fibheap *fh);
void fh_decreasekey (struct fibheap *fh, const void *vnode); /* to be called AFTER decreasing the key */

#if defined (__cplusplus)
}
#endif

#endif /* Q_FIBHEAP_H */

/* SHA1 not available (unoffical build.) */
