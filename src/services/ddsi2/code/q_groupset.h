#ifndef NN_GROUPSET_H
#define NN_GROUPSET_H

struct nn_groupset;

typedef int (*nn_groupset_foreach_t) (v_group g, void *arg);

int nn_groupset_add_group (struct nn_groupset *gs, v_group g);

/* groupset_add locks ADD, then calls groupset_add_group, which locks
   GS. Don't do groupset_add(a,b) PAR groupset_add(b,a). */
int nn_groupset_add (struct nn_groupset *gs, const struct nn_groupset *add);

void nn_groupset_free (struct nn_groupset *gs);
int nn_groupset_fromqos (struct nn_groupset *gs, v_kernel kernel, const nn_xqos_t *qos);
struct nn_groupset *nn_groupset_new (void);

/* groupset_foreach locks GS, and keeps it locked across calls to f() */
int nn_groupset_foreach (const struct nn_groupset *gs, nn_groupset_foreach_t f, void *arg);
int nn_groupset_empty (const struct nn_groupset *gs);

#endif /* NN_GROUPSET_H */

/* SHA1 not available (unoffical build.) */
