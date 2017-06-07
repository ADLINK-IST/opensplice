#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "dds.h"


void
dds_free (
    void * ptr)
{
    DDS_free(ptr);
}

char *
dds_string_alloc (
    size_t size)
{
    return DDS_string_alloc(size);
}

char *
dds_string_dup (
    const char * str)
{
    return DDS_string_dup(str);
}

void
dds_string_free (
    char * str)
{
    DDS_free(str);
}

void
dds_sample_free(
    void *sample,
    const struct dds_topic_descriptor * desc,
    dds_free_op_t op)
{
    if (sample) {
        if (op & DDS_FREE_ALL_BIT) {
            dds_free (sample);
        } else {
            if (desc->destructor) {
                (void)desc->destructor(sample);
            }
            if (desc->size > 0) {
                memset(sample, 0, desc->size);
            }
        }
    }
}
