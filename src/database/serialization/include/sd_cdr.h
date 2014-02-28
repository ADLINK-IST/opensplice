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
#ifndef SD_CDR_H
#define SD_CDR_H

#include "os_defs.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define SD_CDR_OK 0
#define SD_CDR_INVALID -1
#define SD_CDR_OUT_OF_MEMORY -2

struct sd_cdrSerdata;
struct sd_cdrInfo;
struct c_type_s;

OS_API struct sd_cdrInfo *sd_cdrInfoNew (const struct c_type_s *type);
OS_API void sd_cdrInfoClearPadding (struct sd_cdrInfo *ci);
OS_API void sd_cdrInfoFree (struct sd_cdrInfo *ci);

/* sd_cdrNoteCatsStac: typestack[0 .. n-1] gives the path to the
   current type, with typestack[0] the top-level type and
   typestack[n-1] the leaf.

   sd_cdrNoteCatsStac + sd_cdrCompile currently assume that the
   order of calls to sd_cdrNoteCatsStac corresponds to an in-order
   walk of the type. */
OS_API int sd_cdrNoteCatsStac (struct sd_cdrInfo *ci, unsigned n, struct c_type_s const * const *typestack);

OS_API int sd_cdrCompile (struct sd_cdrInfo *ci);
OS_API struct sd_cdrSerdata *sd_cdrSerialize (const struct sd_cdrInfo *ci, const void * data);
OS_API int sd_cdrDeserializeRaw (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src);
OS_API int sd_cdrDeserializeObject (void **dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src);
OS_API void sd_cdrSerdataFree (struct sd_cdrSerdata *serdata);

/* SerdataBlob returns a size of blob, address of blob in *blob, has
   ownership of blob and frees it with sd_cdrSerdataFree. */
OS_API os_uint32 sd_cdrSerdataBlob (const void **blob, struct sd_cdrSerdata *serdata);

#undef OS_API

#endif /* SD_CDR_H */
