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

#include "d__filter.h"
#include "d__element.h"
#include "d__misc.h"
#include "os_report.h"
#include "ut_xmlparser.h"

static c_bool
d_filterAddPartitionTopic(
    d_filter filter,
    const char *name,
    const char *partition,
    const char *topic)
{
    d_element element;
    c_bool added = FALSE;

    assert(d_filterIsValid(filter));

    element = d_elementNew(name, partition, topic);
    if (element) {
        c_iterAppend(filter->elements, element);
        added = TRUE;
    }
    return added;
}


d_filter
d_filterNew (
    const char *xmlExpression)
{
    d_filter filter;
    /* Allocate filter object */
    filter = d_filter(os_malloc(C_SIZEOF(d_filter)));
    if (filter != NULL){
        /* Call super-init */
        d_objectInit(d_object(filter), D_FILTER,
                     (d_objectDeinitFunc)d_filterDeinit);
        /* Initialize filter object */
        if (xmlExpression != NULL) {
            filter->xmlExpression = os_strdup(xmlExpression);
        } else {
            filter->xmlExpression = NULL;
        }
        filter->elements = NULL;
        filter->sqlExpression = NULL;
        if (filter->xmlExpression != NULL) {
            size_t n;
            q_expr expr;
            /* Construct a sqlExpression from the filter.
             * The xmlxpression may contain escapes such as
             * &lt;, &gt;, &amp;, &apos;, &quot;, but the
             * sql expression may not
             */
            if ((filter->sqlExpression = os_strdup(xmlExpression)) == NULL) {
                goto err_allocFilterSqlExpression;
            }
            n = strlen(xmlExpression);
            if (ut_xmlUnescapeInsitu(filter->sqlExpression, &n) < 0) {
                /* Parse &lt;, &gt;, &amp;, &apos;, &quot; failed */
                OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                "Content attribute ' %s' in <Filter>-element contains invalid escape sequences, terminate the durability service", filter->xmlExpression);
                goto err_invalidUnescape;
            }
            assert(n<=strlen(xmlExpression));
            filter->sqlExpression[n] = '\0';
            if ((expr = (q_expr)q_parse(filter->sqlExpression)) == NULL) {
                /* A filter with an invalid syntax is detected. This
                 * also covers the case where the content attribute
                 * is empty.
                 */
                OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                "The <Filter>-element with content attribute '%s' has invalid sql syntax '%s', terminate the durability service", filter->xmlExpression, filter->sqlExpression);
                goto err_invalidSqlExpression;
            }
            if (expr) {
                q_dispose(expr);
            }
        }
        filter->elements = c_iterNew(NULL);
    }
    return filter;

err_invalidSqlExpression:
err_invalidUnescape:
err_allocFilterSqlExpression:
   d_filterFree(filter);
   return NULL;
}


void
d_filterDeinit (
        d_filter filter)
{
    assert(d_filterIsValid(filter));

    if (filter->xmlExpression) {
        os_free(filter->xmlExpression);
        filter->xmlExpression = NULL;
    }
    if (filter->sqlExpression) {
        os_free(filter->sqlExpression);
        filter->sqlExpression = NULL;
    }
    if (filter->elements) {
        d_element element;

        element = d_element(c_iterTakeFirst(filter->elements));
        while (element) {
            d_elementFree(element);
            element = d_element(c_iterTakeFirst(filter->elements));
        }
        c_iterFree(filter->elements);
        filter->elements = NULL;
    }
    /* Call super-deinit */
    d_objectDeinit(d_object(filter));
}


void
d_filterFree (
    d_filter filter)
{
    assert(d_filterIsValid(filter));

    d_objectFree(d_object(filter));
}


c_bool
d_filterAddElement(
    d_filter filter,
    const char *name,
    const char *partitionTopic,
    const char *topicGiven)
{
    c_bool added = FALSE;
    char *partition;
    char *topic;
    os_uint32 len1, len2;

    assert(d_filterIsValid(filter));

    len1 = (os_uint32) (strlen(partitionTopic) + 1);
    if (len1 < D_MAX_STRLEN_NAMESPACE) {
        if (topicGiven) {
            len2 = (os_uint32) (strlen(topicGiven) + 1);
            if (len2 < D_MAX_STRLEN_NAMESPACE) {
                added = d_filterAddPartitionTopic(filter, name, partitionTopic, topicGiven);
            }
        } else {
            /* The topic is not given. If the partitionTopic string contains
             * a '.', then try to split the string in a partition part and a
             * topic part. If there is no topic then use '*' by default.
             */
            partition = os_malloc(len1);
            os_strncpy(partition, partitionTopic, len1);
            /* Make topic point to last character in partition string.
             * partition points to first character and len1
             * includes '\0', so subtract 2 to point to last character.
             */
            topic = partition + (len1-2);
            /* QAC EXPECT 2106,3123; */
            while ((*topic != '.') && (topic != partition)) {
                /* QAC EXPECT 0489; */
                topic--;
            }
            /* QAC EXPECT 2106,3123; */
            if (*topic == '.') {
                *topic = 0;
                /* QAC EXPECT 0489; */
                topic++;
                /* QAC EXPECT 2106; */
                if (*topic != 0) {
                    added = d_filterAddPartitionTopic(filter, name, partition, topic);
                }
            } else {
                /* Though <PartitionTopic> was used in the filter, only
                 * a partition is provided. In that case assume the topic
                 * top be '*'
                 */
                added = d_filterAddPartitionTopic(filter, name, partition, "*");
            }
            os_free(partition);
        }
    }
    return added;
}

