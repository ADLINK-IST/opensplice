/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef D__GROUPHASH_H
#define D__GROUPHASH_H

#include "c_typebase.h"
#include "c_iterator.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct d_groupHash {
    c_octet flags;
    c_ulong nrSamples;
    c_octet hash[16];
};

void        d_groupHashCalculate    (struct d_groupHash *groupHash, const c_iter list);

c_char*     d_groupHashToString     (struct d_groupHash *groupHash);

c_bool      d_groupHashFromString   (struct d_groupHash *groupHash, c_char *hashString);

c_bool      d_groupHashIsEqual      (struct d_groupHash *groupHash1, struct d_groupHash *groupHash2);


#if defined (__cplusplus)
}
#endif

#endif /* D__GROUPHASH_H */
