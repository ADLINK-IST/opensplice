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

#ifndef D__FILTER_H
#define D__FILTER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_filter validity.
 * Because d_filter is a concrete class typechecking is required.
 */
#define             d_filterIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_FILTER)

/**
 * \brief The d_filter cast macro.
 *
 * This macro casts an object to a d_filter object.
 */
#define d_filter(_this) ((d_filter)(_this))

C_STRUCT(d_filter) {
    C_EXTENDS(d_object);
    c_char *xmlExpression;          /* The xml expression associated with this filter */
    c_char *sqlExpression;          /* The sql expression associated with this filter.
                                     * This is not necessarily equal to the xml expression
                                     * because the expression may contain &lt;, &gt; etc.
                                     * but the sqlExpression may not.
                                     */
    c_iter elements;                /* The <PartitionTopic> and <Partition> elements of this filter */
};


d_filter            d_filterNew                 (const char* xmlExpression);

void                d_filterDeinit              (d_filter filter);

void                d_filterFree                (d_filter filter);

c_bool              d_filterAddElement          (d_filter filter,
                                                 const char *name,
                                                 const char *partitionTopic,
                                                 const char *topicGiven);

#if defined (__cplusplus)
}
#endif

#endif /* D__FILTER_H */
