/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "gapi_common.h"
#include "gapi_set.h"

#include "os_heap.h"

C_CLASS(gapi_setEntry);

C_STRUCT(gapi_setEntry) {
    gapi_setEntry prev;
    gapi_setEntry next;
    gapi_object object;
};

C_STRUCT(gapi_set) {
    gapi_setEntry first;
    gapi_setEntry last;
    gapi_long count;
    gapi_equality (*compare)();
};

C_STRUCT(gapi_setIter) {
    gapi_set set;
    gapi_setEntry cursor;
};

gapi_setEntry
gapi_setEntryNew (
    gapi_object object)
{
    gapi_setEntry newEntry = os_malloc (C_SIZEOF(gapi_setEntry));

    if (newEntry != NULL) {
        newEntry->object = object;
        newEntry->next = NULL;
        newEntry->prev = NULL;
    }
    return newEntry;
}

void
gapi_setEntryFree (
    gapi_set set,
    gapi_setEntry entry)
{
    os_free (entry);
}

gapi_set
gapi_setNew (
    gapi_equality (*compare)())
{
    gapi_set newSet = (gapi_set)os_malloc (C_SIZEOF(gapi_set));

    if (newSet != NULL) {
	newSet->first = NULL;
	newSet->last = NULL;
	newSet->count = 0;
	newSet->compare = compare;
    }
    return newSet;
}

void
gapi_setFree (
    gapi_set set)
{
    gapi_setEntry entry;

    if (set) {
        entry = set->first;
        while (entry != NULL) {
            set->first = entry->next;
            gapi_setEntryFree(set, entry);
            set->count--;
            entry = set->first;
        }
        assert(set->count == 0);
        os_free (set);
    }
}

gapi_returnCode_t
gapi_setAdd (
    gapi_set set,
    gapi_object object)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    gapi_setEntry newEntry = gapi_setEntryNew (object);

    if (newEntry != NULL) {
        if (set->first) {
            assert(set->first->prev == NULL);
            set->first->prev = newEntry;
            newEntry->next = set->first;
        } else {
            assert(set->last == NULL);
            set->last = newEntry;
        }
        set->first = newEntry;
        set->count++;
    } else {
        result = GAPI_RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

void
gapi_setRemove (
    gapi_set set,
    gapi_object object)
{
    gapi_setEntry entry, next;

    entry = set->first;
    while (entry != NULL) {
        next = entry->next;
        if (entry->object == object) {
            if (entry->prev) {
                entry->prev->next = entry->next;
            } else {
                set->first = entry->next;
            }
            if (entry->next) {
                entry->next->prev = entry->prev;
            } else {
                set->last = entry->prev;
            }
            entry->prev = NULL;
            entry->next = NULL;
            gapi_setEntryFree (set, entry);
            set->count--;
            return;
        }
        entry = next;
    }
}

gapi_boolean
gapi_setIsEmpty (
    gapi_set set)
{
    return (gapi_boolean)(set->count == 0);
}

gapi_setIter
gapi_setFind (
    gapi_set set,
    gapi_object object)
{
    gapi_setIter setIter = NULL;
    gapi_setEntry entry;

    if (set) {
        setIter = (gapi_setIter)os_malloc(C_SIZEOF(gapi_setIter));
        if (setIter) {
            setIter->set = set;
            setIter->cursor = NULL;
            entry = set->first;
            while (entry != NULL) {
                if (set->compare (entry->object, object) == GAPI_EQ) {
                    setIter->cursor = entry;
                    return setIter;
                }
                entry = entry->next;
            }
        }
    }
    return setIter;
}

gapi_setIter
gapi_setFirst (
    gapi_set set)
{
    gapi_setIter setIter = NULL;

    if (set) {
        setIter = (gapi_setIter)os_malloc(C_SIZEOF(gapi_setIter));

        if (setIter) {
            setIter->set = set;
            setIter->cursor = set->first;
        }
    }
    return setIter;
}

gapi_setIter
gapi_setLast (
    gapi_set set)
{
    gapi_setIter setIter = NULL;

    if (set) {
        setIter = (gapi_setIter)os_malloc(C_SIZEOF(gapi_setIter));

        if (setIter) {
            setIter->set = set;
            setIter->cursor = set->last;
        }
    }
    return setIter;
}

void
gapi_setIterFree (
    gapi_setIter setIter)
{
    os_free (setIter);
}

gapi_long
gapi_setIterNext (
    gapi_setIter setIter)
{
    gapi_long result = 0;

    if (setIter) {
        if (setIter->cursor) {
            setIter->cursor = setIter->cursor->next;
            if (setIter->cursor) {
                result = 1;
            }
        }
    }
    return result;
}

gapi_long
gapi_setIterPrev (
    gapi_setIter setIter)
{
    gapi_long result = 0;

    if (setIter) {
        if (setIter->cursor) {
            setIter->cursor = setIter->cursor->prev;
            if (setIter->cursor) {
                result = 1;
            }
        }
    }
    return result;
}

gapi_long
gapi_setIterRemove (
    gapi_setIter setIter)
{
    gapi_setEntry cursor;
    gapi_set set;
    gapi_long result = 1;

    cursor = setIter->cursor;

    if( cursor != NULL ) {
        set = setIter->set;
        setIter->cursor = cursor->next;
        if (cursor->prev) {
            cursor->prev->next = cursor->next;
        } else {
            set->first = cursor->next;
        }
        if (cursor->next) {
            cursor->next->prev = cursor->prev;
        } else {
            set->last = cursor->prev;
        }
        cursor->prev = NULL;
        cursor->next = NULL;
        gapi_setEntryFree (set, cursor);
        set->count--;
    }
    if (setIter->cursor == NULL) {
        result = 0;
    }
    return result;
}

gapi_object
gapi_setIterObject (
    gapi_setIter setIter)
{
    if (setIter == NULL) {
	return NULL;
    } else if (setIter->cursor == NULL) {
        return NULL;
    } else {
        return setIter->cursor->object;
    }
}

gapi_long
gapi_setIterSize (
    gapi_setIter setIter)
{
    if (setIter == NULL) {
	return 0;
    }
    return setIter->set->count;
}
