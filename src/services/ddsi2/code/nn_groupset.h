#ifndef NN_GROUPSET_H
#define NN_GROUPSET_H

struct nn_groupset;

typedef int (*nn_groupset_foreach_t) (v_group g, void *arg);

int nn_groupset_add_group (struct nn_groupset *gs, v_group g);
int nn_groupset_add (struct nn_groupset *gs, const struct nn_groupset *add);
void nn_groupset_free (struct nn_groupset *gs);
int nn_groupset_fromqos (struct nn_groupset *gs, v_kernel kernel, const nn_xqos_t *qos);
struct nn_groupset *nn_groupset_new (void);
int nn_groupset_foreach (struct nn_groupset *gs, nn_groupset_foreach_t f, void *arg);
int nn_groupset_empty (const struct nn_groupset *gs);

#endif /* NN_GROUPSET_H */
