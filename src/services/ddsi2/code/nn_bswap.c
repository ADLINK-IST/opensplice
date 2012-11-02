#include "nn_bswap.h"

/* If C99 bare "inline" behaviour available, we don't need these three functions. */

#if ! NN_HAVE_C99_INLINE
#include "nn_bswap.template"
#endif

nn_guid_prefix_t nn_hton_guid_prefix (nn_guid_prefix_t p)
{
  int i;
  for (i = 0; i < 3; i++)
    p.u[i] = toBE4u (p.u[i]);
  return p;
}

nn_guid_prefix_t nn_ntoh_guid_prefix (nn_guid_prefix_t p)
{
  int i;
  for (i = 0; i < 3; i++)
    p.u[i] = fromBE4u (p.u[i]);
  return p;
}

nn_entityid_t nn_hton_entityid (nn_entityid_t e)
{
  e.u = toBE4u (e.u);
  return e;
}

nn_entityid_t nn_ntoh_entityid (nn_entityid_t e)
{
  e.u = fromBE4u (e.u);
  return e;
}

nn_guid_t nn_hton_guid (nn_guid_t g)
{
  g.prefix = nn_hton_guid_prefix (g.prefix);
  g.entityid = nn_hton_entityid (g.entityid);
  return g;
}

nn_guid_t nn_ntoh_guid (nn_guid_t g)
{
  g.prefix = nn_ntoh_guid_prefix (g.prefix);
  g.entityid = nn_ntoh_entityid (g.entityid);
  return g;
}
