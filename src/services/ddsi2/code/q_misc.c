/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <string.h>

#include "q_misc.h"

char *ddsi2_strsep (char **str, const char *sep)
{
  char *ret;
  if (**str == '\0')
    return 0;
  ret = *str;
  while (**str && strchr (sep, **str) == 0)
    (*str)++;
  if (**str != '\0')
  {
    **str = '\0';
    (*str)++;
  }
  return ret;
}

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

int vendor_is_prismtech (nn_vendorid_t vendor)
{
  const nn_vendorid_t prismtech = NN_VENDORID_PRISMTECH;
  return vendor.id[0] == prismtech.id[0] && vendor.id[1] == prismtech.id[1];
}

int is_own_vendor (nn_vendorid_t vendor)
{
  const nn_vendorid_t ownid = MY_VENDOR_ID;
  return vendor.id[0] == ownid.id[0] && vendor.id[1] == ownid.id[1];
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



/* SHA1 not available (unoffical build.) */
