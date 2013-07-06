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
#include "gapi_common.h"
#include "gapi_map.h"

#include "os_heap.h"

C_CLASS(gapi_mapEntry);

C_STRUCT(gapi_mapEntry) {
    gapi_mapEntry prev;
    gapi_mapEntry next;
    gapi_object key;
    gapi_object object;
};

C_STRUCT(gapi_map) {
    gapi_mapEntry first;
    gapi_mapEntry last;
    gapi_long count;
    gapi_equality (*compare)();
    gapi_boolean free_key;
    gapi_boolean free_object;
};

C_STRUCT(gapi_mapIter) {
    gapi_map map;
    gapi_mapEntry cursor;
};

gapi_mapEntry
gapi_mapEntryNew (
    gapi_object key,
    gapi_object object)
{
    gapi_mapEntry newEntry = os_malloc (C_SIZEOF(gapi_mapEntry));

    if (newEntry != NULL) {
        newEntry->key = key;
        newEntry->object = object;
        newEntry->next = NULL;
        newEntry->prev = NULL;
    }
    return newEntry;
}

void
gapi_mapEntryFree (
    gapi_map map,
    gapi_mapEntry entry)
{
    if (map->free_key) {
        os_free (entry->key);
    }
    if (map->free_object) {
        os_free (entry->object);
    }
    os_free (entry);
}

gapi_map
gapi_mapNew (
    gapi_equality (*compare)(),
    gapi_boolean free_key,
    gapi_boolean free_object)
{
    gapi_map newMap = os_malloc (C_SIZEOF(gapi_map));

    if (newMap != NULL) {
        newMap->free_key = free_key;
        newMap->free_object = free_object;
        newMap->first = NULL;
        newMap->last = NULL;
        newMap->count = 0;
        newMap->compare = compare;
    }
    return newMap;
}

void
gapi_mapFree (
    gapi_map map)
{
    gapi_mapEntry entry;

    entry = map->first;
    while (entry != NULL) {
        map->first = entry->next;
        gapi_mapEntryFree (map, entry);
        map->count--;
        entry = map->first;
    }
    assert(map->count == 0);
    os_free (map);
}

gapi_returnCode_t
gapi_mapAdd (
    gapi_map map,
    gapi_object key,
    gapi_object object)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_mapEntry newEntry = gapi_mapEntryNew (key, object);

    if (newEntry != NULL) {
        if (map->first) {
            assert(map->first->prev == NULL);
            map->first->prev = newEntry;
            newEntry->next = map->first;
        } else {
            assert(map->last == NULL);
            map->last = newEntry;
        }
        map->first = newEntry;
        map->count++;
    } else {
        result = GAPI_RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

void
gapi_mapRemove (
    gapi_map map,
    gapi_object key)
{
    gapi_mapEntry entry, next;

    entry = map->first;
    while (entry != NULL) {
        next = entry->next;
        if (map->compare (entry->key, key) == GAPI_EQ) {
            if (entry->prev) {
                entry->prev->next = entry->next;
            } else {
                map->first = entry->next;
            }
            if (entry->next) {
                entry->next->prev = entry->prev;
            } else {
                map->last = entry->prev;
            }
            entry->prev = NULL;
            entry->next = NULL;
            gapi_mapEntryFree (map, entry);
            map->count--;
            return;
        }
        entry = next;
    }
    return;
}

c_long
gapi_mapLength(
    gapi_map map)
{
    return map->count;
}

gapi_boolean
gapi_mapIsEmpty (
    gapi_map map)
{
    return (gapi_boolean)(map->count == 0);
}

gapi_mapIter
gapi_mapFind (
    gapi_map map,
    const gapi_object key)
{
    gapi_mapIter mapIter = NULL;
    gapi_mapEntry entry;

    if (map) {
        mapIter = (gapi_mapIter)os_malloc(C_SIZEOF(gapi_mapIter));
        if (mapIter) {
            mapIter->map = map;
            mapIter->cursor = NULL;
            entry = map->first;
            while (entry != NULL) {
                if (map->compare (entry->key, key) == GAPI_EQ) {
                    mapIter->cursor = entry;
                    return mapIter;
                }
                entry = entry->next;
            }
        }
    }
    return mapIter;
}

gapi_mapIter
gapi_mapFirst (
    gapi_map map)
{
    gapi_mapIter mapIter = NULL;

    if (map) {
        mapIter = (gapi_mapIter)os_malloc(C_SIZEOF(gapi_mapIter));

        if (mapIter) {
            mapIter->map = map;
            mapIter->cursor = map->first;
        }
    }
    return mapIter;
}

gapi_mapIter
gapi_mapLast (
    gapi_map map)
{
    gapi_mapIter mapIter = NULL;

    if (map) {
        mapIter = (gapi_mapIter)os_malloc(C_SIZEOF(gapi_mapIter));

        if (mapIter) {
            mapIter->map = map;
            mapIter->cursor = map->last;
        }
    }
    return mapIter;
}

void
gapi_mapIterFree (
    gapi_mapIter mapIter)
{
    os_free (mapIter);
}

gapi_long
gapi_mapIterNext (
    gapi_mapIter mapIter)
{
    gapi_long result = 0;

    if (mapIter) {
        if (mapIter->cursor) {
            mapIter->cursor = mapIter->cursor->next;
            if (mapIter->cursor) {
                result = 1;
            }
        }
    }
    return result;
}

gapi_long
gapi_mapIterPrev (
    gapi_mapIter mapIter)
{
    gapi_long result = 0;
    
    if (mapIter) {
        if (mapIter->cursor) {
            mapIter->cursor = mapIter->cursor->prev;
            if (mapIter->cursor) {
                result = 1;
            }
        }
    }
    return result;
}

gapi_long
gapi_mapIterRemove (
    gapi_mapIter mapIter)
{
    gapi_mapEntry entry, next;
    gapi_map map;
    gapi_long result = 1;

    map = mapIter->map;
    entry = map->first;
    while (entry != NULL) {
        next = entry->next;
        if (map->compare (entry->key, mapIter->cursor->key) == GAPI_EQ) {
            if (entry == mapIter->cursor) {
                mapIter->cursor = entry->next;
            }
            if (entry->prev) {
                entry->prev->next = entry->next;
                entry->prev = NULL;
            } else {
                map->first = entry->next;
            }
            if (entry->next) {
                entry->next->prev = entry->prev;
                entry->next = NULL;
            } else {
                map->last = entry->prev;
            }
            gapi_mapEntryFree (map, entry);
            map->count--;
            next = NULL; /* Found entry so break out of loop */
        }
        entry = next;
    }
    if (mapIter->cursor == NULL) {
        result = 0;
    }
    return result;
}

gapi_object
gapi_mapIterKey (
    gapi_mapIter mapIter)
{
    if (mapIter) {
        if (mapIter->cursor) {
            return mapIter->cursor->key;
        }
    }
    return NULL;
}

gapi_object
gapi_mapIterObject (
    gapi_mapIter mapIter)
{
    if (mapIter) {
        if (mapIter->cursor) {
            return mapIter->cursor->object;
        }
    }
    return NULL;
}

gapi_long
gapi_mapIterSize (
    gapi_mapIter mapIter)
{
    if (mapIter == NULL) {
        return 0;
    }
    return mapIter->map->count;
}
