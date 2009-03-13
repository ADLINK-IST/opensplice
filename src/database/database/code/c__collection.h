
#ifndef C__COLLECTION_H
#define C__COLLECTION_H

#include "c_collection.h"

#if defined (__cplusplus)
extern "C" {
#endif

extern const c_long c_listSize;
extern const c_long c_setSize;
extern const c_long c_bagSize;
extern const c_long c_tableSize;
extern const c_long c_querySize;

c_array c_keyList        (c_table c);
void    c_collectionInit (c_base base);

void    c_clear(c_collection c);

#if defined (__cplusplus)
}
#endif

#endif /* C__COLLECTION_H */
