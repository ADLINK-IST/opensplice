/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "q_bswap.h"

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

void bswap_sequence_number_set_hdr (nn_sequence_number_set_t *snset)
{
  bswapSN (&snset->bitmap_base);
  snset->numbits = bswap4u (snset->numbits);
}

void bswap_sequence_number_set_bitmap (nn_sequence_number_set_t *snset)
{
  unsigned i, n = (snset->numbits + 31) / 32;
  for (i = 0; i < n; i++)
    snset->bits[i] = bswap4u (snset->bits[i]);
}

void bswap_fragment_number_set_hdr (nn_fragment_number_set_t *fnset)
{
  fnset->bitmap_base = bswap4u (fnset->bitmap_base);
  fnset->numbits = bswap4u (fnset->numbits);
}

void bswap_fragment_number_set_bitmap (nn_fragment_number_set_t *fnset)
{
  unsigned i, n = (fnset->numbits + 31) / 32;
  for (i = 0; i < n; i++)
    fnset->bits[i] = bswap4u (fnset->bits[i]);
}

/* SHA1 not available (unoffical build.) */
