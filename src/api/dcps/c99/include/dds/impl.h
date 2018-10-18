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

#ifndef DDS_IMPL_H
#define DDS_IMPL_H

/** @file impl.h
 *  @brief Vortex   defines header
 */

#include <vortex_os.h>
#include <stdbool.h>


#if defined (__cplusplus)
extern "C" {
#endif

#ifndef DDS_LENGTH_UNLIMITED
#define DDS_LENGTH_UNLIMITED -1

#define DDS_READ_SAMPLE_STATE 1u
#define DDS_NOT_READ_SAMPLE_STATE 2u
#define DDS_ANY_SAMPLE_STATE (1u | 2u)

#define DDS_NEW_VIEW_STATE 4u
#define DDS_NOT_NEW_VIEW_STATE 8u
#define DDS_ANY_VIEW_STATE (4u | 8u)

#define DDS_ALIVE_INSTANCE_STATE 16u
#define DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE 32u
#define DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE 64u
#define DDS_ANY_INSTANCE_STATE (16u | 32u | 64u)

#define DDS_ANY_STATE (DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE)

#define DDS_NOT_REJECTED 0
#define DDS_REJECTED_BY_INSTANCES_LIMIT 1
#define DDS_REJECTED_BY_SAMPLES_LIMIT 2
#define DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT 3

#define DDS_DOMAIN_DEFAULT 0x7fffffff
#define DDS_HANDLE_NIL 0
#endif


typedef void * dds_entity_t;
typedef void * dds_condition_t;
typedef void * dds_listener_t;
typedef uint64_t dds_instance_handle_t;
typedef uint32_t dds_domainid_t;
typedef void * dds_waitset_t;

typedef struct dds_condition_seq
{
  uint32_t _length;
  dds_condition_t * _buffer;
  bool _release;
} dds_condition_seq;

typedef int (*dds_topic_type_registration)(dds_entity_t, void *arg);
typedef int (*dds_topic_type_destructor)(void *);

typedef struct dds_topic_descriptor
{
    dds_topic_type_registration register_type;
    char *type_name;
    uint32_t size;
    dds_topic_type_destructor destructor;
    void *arg;
}
dds_topic_descriptor_t;



#if defined (__cplusplus)
}
#endif
#endif
