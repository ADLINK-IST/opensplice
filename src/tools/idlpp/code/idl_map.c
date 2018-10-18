/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "idl_map.h"

#include "os_iterator.h"
#include "os_heap.h"

typedef enum {
    IDL_OK,
    IDL_ERROR
} IDL_RETCODE;

C_CLASS(idl_mapEntry);

C_STRUCT(idl_mapEntry) {
    void *key;
    void *object;
};

C_STRUCT(idl_map) {
    os_iter map;
    idl_equality (*compare)();
    int free_key;
    int free_object;
};

C_STRUCT(idl_mapIter) {
    idl_map map;
    void *key;
    void *object;
    os_uint32 index;
    os_uint32 length;
};

idl_mapEntry
idl_mapEntryNew (
    void *key,
    void *object
    )
{
    idl_mapEntry newEntry = os_malloc (OS_SIZEOF(idl_mapEntry));

    if (newEntry != NULL) {
        newEntry->key = key;
        newEntry->object = object;
    }
    return newEntry;
}

void
idl_mapEntryFree (
    idl_mapEntry entry,
    int free_key,
    int free_object
    )
{
    if (free_key) {
        os_free (entry->key);
    }
    if (free_object) {
        os_free (entry->object);
    }
    os_free (entry);
}

idl_map
idl_mapNew (
    idl_equality (*compare)(),
    int free_key,
    int free_object
    )
{
    idl_map newMap = os_malloc (OS_SIZEOF(idl_map));

    if (newMap != NULL) {
        newMap->free_key = free_key;
        newMap->free_object = free_object;
        newMap->map = os_iterNew (NULL);
        newMap->compare = compare;
    }
    return newMap;
}

void
idl_mapFree (
    idl_map map
    )
{
    idl_mapEntry entry;

    entry = os_iterTakeFirst (map->map);
    while (entry != NULL) {
        idl_mapEntryFree (entry, map->free_key, map->free_object);
        entry = os_iterTakeFirst (map->map);
    }
    os_iterFree (map->map);
    os_free (map);
}

int
idl_mapAdd (
    idl_map map,
    void *key,
    void *object
    )
{
    int result = IDL_OK;
    idl_mapEntry newEntry = idl_mapEntryNew (key, object);

    if (newEntry != NULL) {
        map->map = os_iterAppend (map->map, newEntry);
    } else {
        result = IDL_ERROR;
    }
    return result;
}

void
idl_mapRemove (
    idl_map map,
    void *key
    )
{
    idl_mapEntry entry;
    os_uint32 index;
    os_uint32 length;

    length = os_iterLength (map->map);
    index  = 0;
    while (index < length) {
        entry = os_iterObject (map->map, index);
        if (map->compare (entry->key, key) == IDL_EQ) {
            os_iterTake (map->map, entry);
            idl_mapEntryFree (entry, map->free_key, map->free_object);
            return;
        }
        index++;
    }
    return;
}

int
idl_mapIsEmpty (
    idl_map map
    )
{
    int result = FALSE;

    if (os_iterLength(map->map) == 0) {
        result = TRUE;
    }
    return result;
}

idl_mapIter
idl_mapFind (
    idl_map map,
    const void *key
    )
{
    idl_mapIter mapIter = idl_mapIterNew();
    idl_mapEntry entry;

    if (mapIter) {
        mapIter->length = os_iterLength (map->map);
        mapIter->map = map;
        while (mapIter->index < mapIter->length) {
            entry = os_iterObject (map->map, mapIter->index);
            if (map->compare (entry->key, key) == IDL_EQ) {
                mapIter->key = entry->key;
                mapIter->object = entry->object;
                return mapIter;
            }
            mapIter->index++;
        }
    }
    return mapIter;
}

idl_mapIter
idl_mapFirst (
    idl_map map
    )
{
    idl_mapIter mapIter = idl_mapIterNew();
    idl_mapEntry entry;

    if (mapIter) {
        mapIter->length = os_iterLength (map->map);
        mapIter->map = map;
        mapIter->index = 0;
        if (os_iterLength (map->map) > 0) {
            entry = os_iterObject (map->map, 0);
            mapIter->key = entry->key;
            mapIter->object = entry->object;
        }
    }

    return mapIter;
}

idl_mapIter
idl_mapLast (
    idl_map map
    )
{
    idl_mapIter mapIter = idl_mapIterNew();
    idl_mapEntry entry;

    if (mapIter) {
        mapIter->length = os_iterLength (map->map);
        mapIter->map = map;
        mapIter->index = 0;
        if (os_iterLength (map->map) > 0) {
            mapIter->index = os_iterLength (map->map)-1;
            entry = os_iterObject (map->map, mapIter->index);
            mapIter->key = entry->key;
            mapIter->object = entry->object;
        }
    }

    return mapIter;
}

idl_mapIter
idl_mapIterNew (
    void
    )
{
    idl_mapIter newMapIter = os_malloc (OS_SIZEOF(idl_mapIter));

    if (newMapIter) {
        newMapIter->index = 0;
        newMapIter->object = NULL;
        newMapIter->key = NULL;
        newMapIter->length = 0;
    }

    return newMapIter;
}

void
idl_mapIterFree (
    idl_mapIter mapIter
    )
{
    os_free (mapIter);
}

int
idl_mapIterNext (
    idl_mapIter mapIter
    )
{
    idl_mapEntry entry;
    int result = 1;

    if (mapIter->index < (mapIter->length-1)) {
        mapIter->index++;
        entry = os_iterObject (mapIter->map->map, mapIter->index);
        mapIter->key = entry->key;
        mapIter->object = entry->object;
    } else {
        mapIter->object = NULL;
        result = 0;
    }
    return result;
}

int
idl_mapIterPrev (
    idl_mapIter mapIter
    )
{
    idl_mapEntry entry;
    int result = 1;

    if (mapIter->index > 0) {
        mapIter->index--;
        entry = os_iterObject (mapIter->map->map, mapIter->index);
        mapIter->key = entry->key;
        mapIter->object = entry->object;
    } else {
        mapIter->object = NULL;
        result = 0;
    }
    return result;
}

int
idl_mapIterRemove (
    idl_mapIter mapIter
    )
{
    idl_mapEntry entry;
    int result = 1;

    idl_mapRemove(mapIter->map,mapIter->key);
    mapIter->length = os_iterLength (mapIter->map->map);

    if (mapIter->index < (mapIter->length)) {
        entry = os_iterObject (mapIter->map->map, mapIter->index);
        mapIter->key = entry->key;
        mapIter->object = entry->object;
    } else {
        mapIter->object = NULL;
        mapIter->key = NULL;
        result = 0;
    }
    return result;
}

void *
idl_mapIterKey (
    idl_mapIter mapIter
    )
{
    if (mapIter == NULL) {
        return NULL;
    }
    return mapIter->key;
}

void *
idl_mapIterObject (
    idl_mapIter mapIter
    )
{
    if (mapIter == NULL) {
        return NULL;
    }
    return mapIter->object;
}

os_uint32
idl_mapIterSize (
    idl_mapIter mapIter
    )
{
    if (mapIter == NULL) {
        return 0;
    }
    return mapIter->length;
}
