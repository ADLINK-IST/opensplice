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
#include "d__types.h"
#include "d_lock.h"

#ifndef D__NAMESPACE_H
#define D__NAMESPACE_H

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \The class d_element is used in the definition
 *  of namespaces (sets of partition/topics)
 * In both partition and topic a '*' matches for any number of
 * characters and a '?' matches one character.
 */
C_STRUCT(d_element) {
    d_partition partition;       /**< partition name          */
    d_topic     topic;           /**< topic name              */
    d_name      name;            /**< element name            */
    os_uint32   strlenPartition; /**< partition name's length */
    os_uint32   strlenTopic;     /**< topic name's length     */
    os_uint32   strlenName;      /**< name's length           */
};

/* used when searching for a match on partition and topic */
C_CLASS(d_nameSpaceSearch);
C_STRUCT(d_nameSpaceSearch) {
    d_partition  partition;
    d_topic      topic;
    c_bool       match;
};


C_STRUCT(d_nameSpace) {
    C_EXTENDS(d_lock);
    d_name           name;
    d_policy         policy;
    d_quality        quality;
    d_networkAddress master;
    d_serviceState 	 masterState;
    c_bool			 masterConfirmed;
    d_table          elements;
    d_mergeState     mergeState;
    d_table          mergedRoleStates;
};

typedef enum d_nameSpaceHelperKind_s {
    D_NS_COUNT, D_NS_COPY
} d_nameSpaceHelperKind;

struct d_nameSpaceHelper{
    d_nameSpaceHelperKind kind;
    os_uint32 count;
    c_char* value;
    d_nameSpace ns;
};

c_bool  d_nameSpaceGetPartitionsAction  (d_element element,
                                         c_voidp args);
c_bool  d_nameSpaceGetPartitionTopicsAction  (d_element element,
                                         c_voidp args);

void    d_nameSpaceDeinit               (d_object object);

#if defined (__cplusplus)
}
#endif

#endif /* D_NAMESPACE_H */
