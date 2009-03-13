/** \file services/serialization/code/sd_deepwalk.h
 *  \brief Prototypes for the deepwalk functionality, to be used by
 *         \b serializer descendants.
 */

#ifndef SD_DEEPWALK_H
#define SD_DEEPWALK_H

#include "c_typebase.h"
#include "c_metabase.h"

typedef void sd_deepwalkFunc(c_type type, c_object *object, void *arg);

void         sd_deepwalk(c_type type, c_object *objectPtr,
                         sd_deepwalkFunc action, void *actionArg);

#endif  /* SD_DEEPWALK_H */
