
#include "dds_dcps.h"

/* Sequence support routines */

typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    void *_buffer;
    DDS_boolean _release;
} DDS_generic_sequence;

void
DDS_sequence_set_release (
    void *sequence,
    DDS_boolean release
    )
{
    DDS_generic_sequence *seq = (DDS_generic_sequence *)sequence;

    seq->_release = release;
}

DDS_boolean
DDS_sequence_get_release (
    void *sequence
    )
{
    DDS_generic_sequence *seq = (DDS_generic_sequence *)sequence;

    return seq->_release;
}
