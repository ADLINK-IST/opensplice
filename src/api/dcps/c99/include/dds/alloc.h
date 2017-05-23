#ifndef DDS_ALLOC_H
#define DDS_ALLOC_H

#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#undef DDS_EXPORT
#ifdef OSPL_BUILD_DCPSC99
#define DDS_EXPORT OS_API_EXPORT
#else
#define DDS_EXPORT OS_API_IMPORT
#endif

#define DDS_FREE_KEY_BIT 0x01
#define DDS_FREE_CONTENTS_BIT 0x02
#define DDS_FREE_ALL_BIT 0x04

typedef enum
{
  DDS_FREE_ALL = DDS_FREE_KEY_BIT | DDS_FREE_CONTENTS_BIT | DDS_FREE_ALL_BIT,
  DDS_FREE_CONTENTS = DDS_FREE_KEY_BIT | DDS_FREE_CONTENTS_BIT,
  DDS_FREE_KEY = DDS_FREE_KEY_BIT
} dds_free_op_t;

DDS_EXPORT void dds_free (void * ptr);

DDS_EXPORT char * dds_string_alloc (size_t size);
DDS_EXPORT char * dds_string_dup (const char * str);
DDS_EXPORT void dds_string_free (char * str);
DDS_EXPORT void dds_sample_free(void *sample, const struct dds_topic_descriptor * desc, dds_free_op_t op);

#undef DDS_EXPORT

#if defined (__cplusplus)
}
#endif
#endif
