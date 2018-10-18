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

#include "d__element.h"
#include "os_heap.h"

d_element
d_elementNew(
    const char * name,
    const char * partition,
    const char * topic )
{
    d_element element;

    element = os_malloc(C_SIZEOF(d_element));
    element->name = NULL;
    element->partition = NULL;
    element->topic = NULL;
    /* QAC EXPECT 1253; */
    element->strlenName      = (os_uint32) (strlen(name) + 1);
    /* QAC EXPECT 1253; */
    element->strlenPartition = (os_uint32) (strlen(partition) + 1);
    /* QAC EXPECT 1253; */
    element->strlenTopic     = (os_uint32) (strlen(topic) + 1);

    element->name = os_malloc(element->strlenName);
    /* QAC EXPECT 5007; use of strcpy */
    os_strcpy(element->name, name);
    element->partition = os_malloc(element->strlenPartition);
    /* QAC EXPECT 5007; use of strcpy */
    os_strcpy(element->partition, partition);
    element->topic = os_malloc(element->strlenTopic);
    /* QAC EXPECT 5007; use of strcpy */
    os_strcpy(element->topic, topic);
    return element;
}


void
d_elementFree(
    d_element element )
{
    if (element) {
        if (element->name) {
            os_free(element->name);
           element->name = NULL;
        }
        if (element->partition) {
            os_free(element->partition);
            element->partition = NULL;
        }
        if (element->topic) {
            os_free(element->topic);
            element->topic = NULL;
        }
        os_free(element);
        element = NULL;
    }
}


c_string
d_elementGetExpression(
    d_element element)
{
    char *result;
    size_t size;

    size = element->strlenPartition + element->strlenTopic + 1; /* For the '/0'*/
    if (element->topic) {
        size++; /* For the '.' */
    }
    result = os_malloc(size);
    if (element->topic) {
        sprintf(result, "%s.%s", element->partition, element->topic);
    } else {
        sprintf(result, "%s", element->partition);
    }
    return result;
}


int
d_elementCompare(
    c_voidp  object1,
    c_voidp  object2 )
{
    d_element element1, element2;
    int  result;

    result = 0;
    element1 = d_element(object1);
    element2 = d_element(object2);
    if (element1 != element2) {
        result = strncmp(element1->topic, element2->topic, element2->strlenTopic);
        if (result == 0) {
            result = strncmp(element1->partition, element2->partition, element2->strlenPartition);
        }
    }
    return result;
}
