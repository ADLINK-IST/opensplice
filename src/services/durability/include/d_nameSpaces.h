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

#ifndef D_NAMESPACES_H
#define D_NAMESPACES_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_nameSpaces(n)  ((d_nameSpaces)(n))
#define d_nsElement(e)   ((d_nsElement)(e))
#define d_nsPartition(e) ((d_nsPartition)(e))

typedef enum d_nsEquality_s{
    D_LESS, D_GREATER, D_EQUAL, D_INTERSECT, D_DIFFERENT
} d_nsEquality;

d_nameSpaces        d_nameSpacesNew                 (d_admin admin,
                                                     d_nameSpace nameSpace,
                                                     d_quality initialQuality,
                                                     c_ulong total);

void                d_nameSpacesFree                (d_nameSpaces namespaces);

int                 d_nameSpacesCompare             (d_nameSpaces ns1,
                                                     d_nameSpaces ns2);

d_alignmentKind     d_nameSpacesGetAlignmentKind    (d_nameSpaces nameSpaces);

d_durabilityKind    d_nameSpacesGetDurabilityKind   (d_nameSpaces nameSpaces);

c_bool              d_nameSpacesIsAligner           (d_nameSpaces nameSpaces);

c_ulong             d_nameSpacesGetTotal            (d_nameSpaces nameSpaces);

c_char*             d_nameSpacesGetPartitions       (d_nameSpaces nameSpaces);

d_quality           d_nameSpacesGetInitialQuality   (d_nameSpaces nameSpaces);

d_networkAddress    d_nameSpacesGetMaster           (d_nameSpaces nameSpaces);

char*               d_nameSpacesGetName             (d_nameSpaces nameSpaces);

void                d_nameSpacesSetMaster           (d_nameSpaces nameSpaces,
                                                     d_networkAddress master);

void                d_nameSpacesSetTotal            (d_nameSpaces nameSpaces,
                                                     c_ulong total);

#if defined (__cplusplus)
}
#endif

#endif /*D_NAMESPACES_H*/
