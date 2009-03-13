#include "d__types.h"

#ifndef D_NAMESPACE_H
#define D_NAMESPACE_H

#if defined (__cplusplus)
extern "C" {
#endif

#define             d_element(o)                        ((d_element)o)
#define             d_nameSpace(o)                      ((d_nameSpace)o)

d_nameSpace         d_nameSpaceNew                      (const char * name,
                                                         d_alignmentKind alignmentKind,
                                                         d_durabilityKind durabilityKind);

int                 d_nameSpaceCompare                  (d_nameSpace ns1,
                                                         d_nameSpace ns2);

int                 d_nameSpaceCompatibilityCompare     (d_nameSpace ns1,
                                                         d_nameSpace ns2);

void                d_nameSpaceFree                     (d_nameSpace nameSpace);

void                d_nameSpaceAddElement               (d_nameSpace nameSpace, 
                                                         const char * name,
                                                         const char * partition, 
                                                         const char * topic);
                                         
char *              d_nameSpaceGetName                  (d_nameSpace nameSpace);

c_bool              d_nameSpaceIsEmpty                  (d_nameSpace nameSpace);

c_bool              d_nameSpaceIsIn                     (d_nameSpace nameSpace,
                                                         d_partition partition, 
                                                         d_topic topic);

void                d_nameSpaceElementWalk              (d_nameSpace nameSpace, 
                                                         c_bool ( * action ) (
                                                            d_element element, 
                                                            c_voidp userData), 
                                                         c_voidp args);

c_bool              d_nameSpaceIsAligner                (d_nameSpace nameSpace) ;

c_char*             d_nameSpaceGetPartitions            (d_nameSpace nameSpace);

void                d_nameSpaceSetInitialQuality        (d_nameSpace nameSpace,
                                                         d_quality quality);

d_quality           d_nameSpaceGetInitialQuality        (d_nameSpace nameSpace);

d_nameSpace         d_nameSpaceFromNameSpaces           (d_nameSpaces ns);

d_alignmentKind     d_nameSpaceGetAlignmentKind         (d_nameSpace nameSpace);

d_durabilityKind    d_nameSpaceGetDurabilityKind        (d_nameSpace nameSpace);

void                d_nameSpaceSetMaster                (d_nameSpace nameSpace, 
                                                         d_networkAddress master);

d_networkAddress    d_nameSpaceGetMaster                (d_nameSpace nameSpace);

c_bool              d_nameSpaceMasterIsMe               (d_nameSpace nameSpace,
                                                         d_admin admin);

c_bool              d_nameSpaceIsAlignmentNotInitial    (d_nameSpace nameSpace);

#if defined (__cplusplus)
}
#endif

#endif /* D_NAMESPACE_H */
