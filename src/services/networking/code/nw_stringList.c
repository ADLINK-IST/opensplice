/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
/* Interface */
#include "nw_stringList.h"

/* Implementation */
#include <string.h>
#include "os_heap.h"
#include "os_stdlib.h" /* os_strdup */
#include "nw_report.h"

#include "nw__confidence.h"


struct nw_stringList_s {
    unsigned int size;
    char **items;
};

nw_stringList
nw_stringListNew(
    const char *string,
    const char *separators)
{
    nw_stringList result = NULL;
    unsigned int size;
    char *helperStart;
    char *helperEnd;
    char *walker;
    unsigned int i;


    if (string != NULL) {
        result = (nw_stringList)os_malloc(sizeof(*result));
        if (result) {
            /* First count how many items the list will contain */
            helperStart = os_strdup(string);
            walker = helperStart;
            size = 0;
            while (*walker) {
                /* First skip all separators and fill them with '\0' characters*/
                while(*walker && strchr(separators, *walker)) {
                    *walker = '\0';
                    walker = &walker[1];
                }
                /* If we have not reached the end of the string, then we have a string here */
                if (*walker) {
                    size++;
                }
                /* Now skip all non-separators */
                while(*walker && !strchr(separators, *walker)) {
                    walker = &walker[1];
                }
            }
            helperEnd = walker;

            /* Then create the array and fill it with the items */
            result->size = size;
            result->items = (char **)os_malloc(size*sizeof(*result->items));
            walker = helperStart;
            i = 0;
            while (walker != helperEnd) {
                /* First skip all '\0' characters*/
                while((walker != helperEnd) && !*walker) {
                    walker = &walker[1];
                }
                if (walker != helperEnd) {
                    result->items[i++] = os_strdup(walker);
                }
                while((walker != helperEnd) && *walker) {
                    walker = &walker[1];
                }
            }
            os_free(helperStart);

        }
    }
    return result;
}


void
nw_stringListFree(
    nw_stringList this)
{
    unsigned int index;

    if (this) {
        for (index = 0; index < this->size; index++) {
            if (this->items[index]) {
                os_free(this->items[index]);
            }
        }
        os_free(this);
    }
}

unsigned int
nw_stringListGetSize(
    nw_stringList this)
{
    unsigned int result = 0;
    if (this) {
        result = this->size;
    }
    return result;
}

const char *
nw_stringListGetValue(
    nw_stringList this,
    unsigned int index)
{
    const char *result = NULL;

    if (this && (this->size > index)) {
        result = this->items[index];
    }
    return result;
}
