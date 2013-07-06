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
#ifndef NN_MISC_H
#define NN_MISC_H

#include "q_protocol.h"

char *ddsi2_strsep (char **str, const char *sep);

int vendor_is_rti (nn_vendorid_t vendor);
int vendor_is_twinoaks (nn_vendorid_t vendor);
int vendor_is_prismtech (nn_vendorid_t vendor);
int is_own_vendor (nn_vendorid_t vendor);
unsigned char normalize_data_datafrag_flags (const SubmessageHeader_t *smhdr, int datafrag_as_data);

os_int64 fromSN (const nn_sequence_number_t sn);
nn_sequence_number_t toSN (os_int64);


#endif /* NN_MISC_H */

/* SHA1 not available (unoffical build.) */
