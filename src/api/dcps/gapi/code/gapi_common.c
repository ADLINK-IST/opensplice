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
#include <ctype.h>

#include "gapi_common.h"

#include "os_heap.h"
#include "os_stdlib.h"

gapi_equality
gapi_stringCompare (
    char *str1,
    char *str2
    )
{
    gapi_long cr;

    cr = strcmp (str1, str2);
    if (cr == 0) {
	return GAPI_EQ;
    } else if (cr < 0) {
	return GAPI_LT;
    }
    return GAPI_GT;
}

gapi_equality
gapi_objectRefCompare (
    gapi_object obj1,
    gapi_object obj2
    )
{
    if (obj1 == obj2) {
	return GAPI_EQ;
    } else if (obj1 < obj2) {
	return GAPI_LT;
    }
    return GAPI_GT;
}

char *
gapi_strdup (
    const char *src
    )
{
    gapi_long len;
    char *dup = NULL;

    if ( src ) {
        len = strlen(src) + 1;

        dup = os_malloc(len);
        if ( dup ) {
            os_strncpy(dup, src, len);
        }
    }

    return dup;
}

typedef struct _Sequence {
    gapi_unsigned_long  _maximum;
    gapi_unsigned_long  _length;
    void               *_buffer;
    gapi_boolean        _release;
} _Sequence;

gapi_boolean
gapi_sequence_is_valid (
    const void *_seq
    )
{
    _Sequence    *seq   = (_Sequence *) _seq;
    gapi_boolean  valid = TRUE;

    if ( seq ) {
        if ( seq->_maximum > 0 && seq->_buffer == NULL ) {
            valid = FALSE;
        }

        if ( seq->_maximum == 0 && seq->_buffer != NULL ) {
            valid = FALSE;
        }

        if ( seq->_length > seq->_maximum ) {
            valid = FALSE;
        }
    } else {
        valid = FALSE;
    }

    return valid;
}

gapi_boolean
gapi_validDuration(const gapi_duration_t *duration)
{
    gapi_boolean valid = FALSE;
    
    /* Duration is valid, only when range below 1 billion, or when both fields are equal to DURATION_INFINITE.*/
    if ( duration ) {
        if ( (duration->sec == GAPI_DURATION_INFINITE_SEC &&
              duration->nanosec == GAPI_DURATION_INFINITE_NSEC) || 
             (duration->nanosec < 1000000000ULL) ) {
            valid = TRUE;
        }
    }

    return valid;
}

gapi_boolean
gapi_validTime(const gapi_time_t *time)
{
    gapi_boolean valid = FALSE;

    if ( time ) {
        if ( (time->sec >= 0) && (time->nanosec < 1000000000ULL) ) {
            valid = TRUE;
        }
    }

    return valid;
}

gapi_boolean
gapi_stringSeqValid (
    const gapi_stringSeq *seq
    )
{
    gapi_boolean valid = TRUE;

    if ( seq != NULL ) {
        if ( gapi_sequence_is_valid((void *)seq) ) {
            gapi_unsigned_long i;

            for ( i = 0; valid && (i < seq->_length); i++ ) {
                if ( !seq->_buffer[i] ) {
                    valid = FALSE;
                }
            }
        } else {
            valid = FALSE;
        }
    } else {
        valid = FALSE;
    }

    return valid;
}
        
gapi_boolean
gapi_sampleStateMaskValid (
    gapi_sampleStateMask mask
    )
{
    gapi_boolean valid = FALSE;
    gapi_sampleStateMask flags = GAPI_READ_SAMPLE_STATE | GAPI_NOT_READ_SAMPLE_STATE;

    if ( mask != GAPI_ANY_SAMPLE_STATE ) {
        if ( (mask | flags) == flags ) {
            valid = TRUE;
        }
    } else {
        valid = TRUE;
    }

    return valid;
}

gapi_boolean
gapi_viewStateMaskValid (
    gapi_viewStateMask mask
    )
{
    gapi_boolean valid = FALSE;
    gapi_viewStateMask flags = GAPI_NEW_VIEW_STATE | GAPI_NOT_NEW_VIEW_STATE;

    if ( mask != GAPI_ANY_VIEW_STATE ) {
        if ( (mask | flags) == flags ) {
            valid = TRUE;
        }
    } else {
        valid = TRUE;
    }
    
    return valid;
}
    
gapi_boolean
gapi_instanceStateMaskValid (
    gapi_instanceStateMask mask
    )
{
    gapi_boolean valid = FALSE;
    gapi_instanceStateMask flags = GAPI_ALIVE_INSTANCE_STATE |
                                   GAPI_NOT_ALIVE_DISPOSED_INSTANCE_STATE |
                                   GAPI_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;

    if ( mask != GAPI_ANY_INSTANCE_STATE ) {
        if ( (mask | flags) == flags ) {
            valid = TRUE;
        }
    } else {
        valid = TRUE;
    }
    
    return valid;
}
       
gapi_boolean
gapi_stateMasksValid (
    gapi_sampleStateMask   sampleMask,
    gapi_viewStateMask     viewMask,
    gapi_instanceStateMask instanceMask
    )
{
    gapi_boolean valid = FALSE;

    if ( gapi_sampleStateMaskValid(sampleMask) &&
         gapi_viewStateMaskValid(viewMask)     &&
         gapi_instanceStateMaskValid(instanceMask) ) {
        valid = TRUE;
    }

    return valid;
}


gapi_boolean
gapi_stringToLongLong (
    const gapi_char *str,
    gapi_long_long *retval)
{
    gapi_long_long n = 1;
    gapi_char *ptr = (gapi_char *)str;
    gapi_long_long result = 0;
    gapi_long base = 10;
    gapi_boolean valid = TRUE;
    gapi_boolean positive = TRUE;
    gapi_boolean trailingSpace = FALSE;


    while ( (*ptr != '\0') && ((*ptr == ' ') || (*ptr == '\t')) ) {
        ptr++;
    }

    if ( *ptr != '\0' ) {
        if ( (*ptr == '0') && ((*(ptr+1) == 'x') || (*(ptr+1) == 'X')) ) {
            ptr += 2;
            base = 16;
        } else {
            if ( *ptr == '+' ) {
                positive = TRUE;
                ptr++;
            } else if ( *ptr == '-' ) {
                positive = FALSE;
                ptr++;
            } else {
                positive = TRUE;
            }
        }

        if ( *ptr == '\0' ) {
            valid = FALSE;
        }

        while ( valid && (*ptr != '\0') ) {
            if ( (*ptr == ' ') || (*ptr == '\t') ) {
                trailingSpace = TRUE;
                ptr++;
            } else {
                if ( !trailingSpace ) {
                    gapi_long_long digit = -1;
                    if ( (*ptr >= '0') && (*ptr <= '9') ) {
                        digit = *ptr - '0';
                    } else if ( base == 16 ) {
                        if ( (*ptr >= 'a') && (*ptr <= 'f') ) {
                            digit = *ptr - 'a' + 10;
                        } else if ( (*ptr >= 'A') && (*ptr <= 'F') ) {
                            digit = *ptr - 'A' + 10;
                        } else {
                            valid = FALSE;
                        }
                    } else {
                        valid = FALSE;
                    }
                    if ( valid ) {
                        result = (result * n) + digit;
                        ptr++;
                        n = base;
                    }
                } else {
                    valid = FALSE;
                }
            }   
        }
    } else {
        valid = FALSE;
    }
    
    if ( positive ) {
        *retval = result;
    } else {
        *retval = result * -1;
    }

    return valid;
}

