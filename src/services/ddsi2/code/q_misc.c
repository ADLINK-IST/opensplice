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
#include <string.h>

#include "kernelModule.h"
#include "q_misc.h"
#include "q_bswap.h"
#include "q_md5.h"

int vendor_is_rti (nn_vendorid_t vendor)
{
  const nn_vendorid_t rti = NN_VENDORID_RTI;
  return vendor.id[0] == rti.id[0] && vendor.id[1] == rti.id[1];
}

int vendor_is_twinoaks (nn_vendorid_t vendor)
{
  const nn_vendorid_t twinoaks = NN_VENDORID_TWINOAKS;
  return vendor.id[0] == twinoaks.id[0] && vendor.id[1] == twinoaks.id[1];
}

int vendor_is_cloud (nn_vendorid_t vendor)
{
  const nn_vendorid_t cloud = NN_VENDORID_PRISMTECH_CLOUD;
  return vendor.id[0] == cloud.id[0] && vendor.id[1] == cloud.id[1];
}

int vendor_is_prismtech (nn_vendorid_t vid)
{
  const nn_vendorid_t pt1 = NN_VENDORID_PRISMTECH_OSPL;
  const nn_vendorid_t pt2 = NN_VENDORID_PRISMTECH_LITE;
  const nn_vendorid_t pt3 = NN_VENDORID_PRISMTECH_GATEWAY;
  const nn_vendorid_t pt4 = NN_VENDORID_PRISMTECH_JAVA;
  const nn_vendorid_t pt5 = NN_VENDORID_PRISMTECH_CLOUD;

  return
    (vid.id[0] == pt1.id[0]) &&
    ((vid.id[1] == pt1.id[1]) || (vid.id[1] == pt2.id[1])
     || (vid.id[1] == pt3.id[1]) || (vid.id[1] == pt4.id[1])
     || (vid.id[1] == pt5.id[1]));
}

int vendor_is_opensplice (nn_vendorid_t vid)
{
  const nn_vendorid_t pt1 = NN_VENDORID_PRISMTECH_OSPL;
  return (vid.id[0] == pt1.id[0] && vid.id[1] == pt1.id[1]);
}

int is_own_vendor (nn_vendorid_t vendor)
{
  const nn_vendorid_t ownid = MY_VENDOR_ID;
  return vendor.id[0] == ownid.id[0] && vendor.id[1] == ownid.id[1];
}

int gid_is_fake (const struct v_gid_s *gid)
{
  /* 0:0:0 is not so much fake as invalid, but here they both mean we
     pretend there is no gid.  For the localId mask, see v_handle.c --
     this is very much an undocumented characteristic of GIDs ... */
  if (gid->systemId == 0 && gid->localId == 0 && gid->serial == 0)
    return 1;
  else if (gid->localId & 0xffc00000)
    return 1;
  else
    return 0;
}

os_int64 fromSN (const nn_sequence_number_t sn)
{
  return ((os_int64) sn.high << 32) | sn.low;
}

nn_sequence_number_t toSN (os_int64 n)
{
  nn_sequence_number_t x;
  x.high = (int) (n >> 32);
  x.low = (unsigned) n;
  return x;
}


int ddsi2_patmatch (const char *pat, const char *str)
{
  c_value p, n, r;
  p.kind = n.kind = V_STRING;
  p.is.String = (char *) pat;
  n.is.String = (char *) str;
  r = c_valueStringMatch (p, n);
  return r.is.Boolean;
}

void nn_guid_to_ospl_gid (v_gid *gid, const nn_guid_t *guid, int guid_has_systemid)
{
  /* Try hard to fake a writer id for OpenSplice based on a GUID. All
   systems I know of have something resembling a host/system id in
   the first 32 bits, so copy that as the system id and copy half of
   an MD5 hash into the remaining 64 bits. Now if only OpenSplice
   would use all 96 bits as a key, we'd be doing reasonably well ... */
  const nn_guid_t nguid = nn_hton_guid (*guid);
  union { os_uint32 u[4]; unsigned char md5[16]; } hp, hg;
  md5_state_t md5st;

  if (guid_has_systemid)
  {
    /* for old OpenSplice versions: modern durability relies on the systemId for
       liveliness of its fellows, so we must not modify it */
    gid->systemId = guid->prefix.u[0];
  }
  else
  {
    md5_init (&md5st);
    md5_append (&md5st, (unsigned char *) &nguid.prefix, sizeof (nguid.prefix));
    md5_finish (&md5st, hp.md5);
    gid->systemId = fromBE4u (hp.u[0]);
  }

  md5_init (&md5st);
  md5_append (&md5st, (unsigned char *) &nguid, sizeof (nguid));
  md5_finish (&md5st, hg.md5);

  /* See also gid_is_fake for the masking of bits in localId */
  gid->localId = fromBE4u (hg.u[0]) & 0x3fffff;
  gid->serial = guid->entityid.u;
}

int version_info_is_6_4_1 (const char *internals)
{
  /* node/version/...; unfortunately, the version is quoted on some platforms */
  const char *p;
  if (internals == NULL || (p = strchr (internals, '/')) == NULL)
    return 0;
  return (strncmp (p + 1, "V6.4.1/", 7) == 0 || strncmp (p + 1, "\"V6.4.1\"/", 9) == 0);
}


/* SHA1 not available (unoffical build.) */
