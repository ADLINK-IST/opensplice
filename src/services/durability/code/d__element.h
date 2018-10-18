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

#ifndef D__ELEMENT_H
#define D__ELEMENT_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \brief The d_element cast macro.
 *
 * This macro casts an object to a d_element object.
 */
#define d_element(_this) ((d_element)(_this))

/**
 * The class d_element is used in the definition
 * of namespaces (sets of partition/topics) and
 * filters.  In both partition and topic a '*'
 * matches for any number of characters and a '?'
 * matches one character.
 */
C_STRUCT(d_element) {
    d_partition partition;       /**< partition name          */
    d_topic     topic;           /**< topic name              */
    d_name      name;            /**< element name            */
    os_uint32   strlenPartition; /**< partition name's length */
    os_uint32   strlenTopic;     /**< topic name's length     */
    os_uint32   strlenName;      /**< name's length           */
};

d_element           d_elementNew                        (const char *name,
                                                         const char *partition,
                                                         const char *topic);

void                d_elementFree                       (d_element element);

d_name              d_elementGetExpression              (d_element element);

int                 d_elementCompare                    (c_voidp object1,
                                                         c_voidp object2);

#if defined (__cplusplus)
}
#endif

#endif /* D__ELEMENT_H */
