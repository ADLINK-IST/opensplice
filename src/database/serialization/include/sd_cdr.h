/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#define SD_CDR_INVALID_TAG -3

struct sd_cdrInfo;
struct sd_cdrSerdata;
struct c_type_s;

enum sd_cdrTagType {
  SD_CDR_TT_PRIM1,
  SD_CDR_TT_PRIM2,
  SD_CDR_TT_PRIM4,
  SD_CDR_TT_PRIM8,
  SD_CDR_TT_STRING
};

/* Note: tags are limited to 20 bits */

typedef int (*sd_cdrSerdataInit_t) (void *serdata, char **dst, os_uint32 size_hint) __nonnull_all__;
typedef int (*sd_cdrSerdataGrow_t) (void *serdata, char **dst, os_uint32 size_hint) __nonnull_all__;
typedef void (*sd_cdrSerdataFinalize_t) (void *serdata, char *dst) __nonnull_all__;
typedef os_uint32 (*sd_cdrSerdataGetpos_t) (const void *serdata, const char *dst) __nonnull_all__;
typedef int (*sd_cdrTagField_t) (os_uint32 *tag, void *arg, enum sd_cdrTagType type, os_uint32 srcoff) __nonnull((1));
typedef int (*sd_cdrTagProcess_t) (void *arg, void *serdata, os_uint32 tag, os_uint32 cdroff, const void *cdr) __nonnull((2, 5));

struct sd_cdrControl {
  /* init() is called to perform the initial allocation of memory for
     the serialised data, it must allocate some memory with a multiple
     of 8 bytes with 8 byte alignment and return the starting address
     in *DST.  SERDATA is the SERDATA argument to
     sd_cdrSerializeControl().  SIZE_HINT is but a suggestion of the
     amount of memory to allocate.  It must return the number of bytes
     allocated or SD_CDR_OUT_OF_MEMORY. */
  sd_cdrSerdataInit_t init;
  /* grow() is called to allocate additional memory, on input *DST is
     the current position in the input stream and equals the first
     address beyond the last byte of the previously allocated block.
     See init() */
  sd_cdrSerdataGrow_t grow;
  /* finalize() is called once serialization has finished.  SERDATA is
     the SERDATA argument to sd_cdrSerializeControl, DST is the
     address of the first byte following the serialised data. */
  sd_cdrSerdataFinalize_t finalize;
  /* getpos() is called to get the current position in the CDR output
     "stream", and is called by the serialiser before serialising a
     "tagged" field.  SERDATA is the SERDATA argument to
     sd_cdrSerializeControl, DST is the address at which the
     serialised form of the tagged field will start. */
  sd_cdrSerdataGetpos_t getpos;
  /* tag() is called during compilation to allow the caller to "tag"
     fields.  Non-zero return means the field is tagged, in which case
     *TAG must be the tag value to use, which can be an arbitrary 20
     bits number.  ARG is the TAG_ARG field of the control structure
     passed to sd_cdrInfoNewControl, TYPE is the simplified type of
     the field and SRCOFF the source offset relative to the base
     address of objects that will serialised. */
  sd_cdrTagField_t tag;
  void *tag_arg;
  /* process() is called during serialisation to post-process tagged
     fields, and is called with the PROCESS_ARG field of the control
     structure, the SERDATA argument to sd_cdrSerializeControl, TAG is
     the tag associated with the field by the tag() function, and
     CDROFF and CDR are respectively the offset and the address of the
     serialised form.  Return should be SD_CDR_OK or one of the error
     codes defined above. */
  sd_cdrTagProcess_t process;
  void *process_arg;
};

OS_API struct sd_cdrInfo *sd_cdrInfoNew (const struct c_type_s *type) __nonnull_all__ __attribute_warn_unused_result__;

/* note: sd_cdrInfoNewControl copies the CONTROL argument */
OS_API struct sd_cdrInfo *sd_cdrInfoNewControl (const struct c_type_s *type, const struct sd_cdrControl *control) __nonnull_all__;
OS_API void sd_cdrInfoClearPadding (struct sd_cdrInfo *ci) __nonnull_all__;
OS_API void sd_cdrInfoFree (struct sd_cdrInfo *ci) __nonnull_all__;

/* sd_cdrNoteCatsStac: typestack[0 .. n-1] gives the path to the
   current type, with typestack[0] the top-level type and
   typestack[n-1] the leaf.

   sd_cdrNoteCatsStac + sd_cdrCompile currently assume that the
   order of calls to sd_cdrNoteCatsStac corresponds to an in-order
   walk of the type. */
OS_API int sd_cdrNoteCatsStac (struct sd_cdrInfo *ci, unsigned n, struct c_type_s const * const *typestack) __nonnull_all__;
OS_API int sd_cdrNoteQuietRef (struct sd_cdrInfo *ci, unsigned int n, struct c_type_s const * const *typestack) __nonnull_all__;

OS_API int sd_cdrCompile (struct sd_cdrInfo *ci) __nonnull_all__ __attribute_warn_unused_result__;

OS_API struct sd_cdrSerdata *sd_cdrSerialize (const struct sd_cdrInfo *ci, const void * data) __nonnull_all__ __attribute_warn_unused_result__;
OS_API int sd_cdrSerializeControl (const struct sd_cdrInfo *ci, void *serdata, const void * data) __nonnull_all__ __attribute_warn_unused_result__;
OS_API struct sd_cdrSerdata *sd_cdrSerializeBSwap (const struct sd_cdrInfo *ci, const void * data) __nonnull_all__ __attribute_warn_unused_result__;
OS_API int sd_cdrSerializeControlBSwap (const struct sd_cdrInfo *ci, void *serdata, const void * data) __nonnull_all__ __attribute_warn_unused_result__;
OS_API struct sd_cdrSerdata *sd_cdrSerializeBE (const struct sd_cdrInfo *ci, const void * data) __nonnull_all__ __attribute_warn_unused_result__;
OS_API int sd_cdrSerializeControlBE (const struct sd_cdrInfo *ci, void *serdata, const void * data) __nonnull_all__ __attribute_warn_unused_result__;
OS_API struct sd_cdrSerdata *sd_cdrSerializeLE (const struct sd_cdrInfo *ci, const void * data) __nonnull_all__ __attribute_warn_unused_result__;
OS_API int sd_cdrSerializeControlLE (const struct sd_cdrInfo *ci, void *serdata, const void * data) __nonnull_all__ __attribute_warn_unused_result__;

OS_API int sd_cdrDeserializeRaw (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src) __nonnull_all__ __attribute_warn_unused_result__;
OS_API int sd_cdrDeserializeObject (void **dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src) __nonnull_all__ __attribute_warn_unused_result__;
OS_API int sd_cdrDeserializeRawBSwap (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src) __nonnull_all__ __attribute_warn_unused_result__;
OS_API int sd_cdrDeserializeObjectBSwap (void **dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src) __nonnull_all__ __attribute_warn_unused_result__;

OS_API int sd_cdrDeserializeRawBE (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src) __nonnull_all__ __attribute_warn_unused_result__;
OS_API int sd_cdrDeserializeRawLE (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src) __nonnull_all__ __attribute_warn_unused_result__;
OS_API int sd_cdrDeserializeObjectBE (void **dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src) __nonnull_all__ __attribute_warn_unused_result__;

OS_API void sd_cdrSerdataFree (struct sd_cdrSerdata *serdata) __nonnull_all__;

/* SerdataBlob returns a size of blob, address of blob in *blob, has
   ownership of blob and frees it with sd_cdrSerdataFree. */
OS_API os_uint32 sd_cdrSerdataBlob (const void **blob, struct sd_cdrSerdata *serdata) __nonnull_all__ __attribute_warn_unused_result__;

#undef OS_API

#endif /* SD_CDR_H */
