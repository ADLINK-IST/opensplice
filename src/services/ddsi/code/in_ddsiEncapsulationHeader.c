/*
 * in_ddsiEncapsulationHeader.c
 *
 *  Created on: Mar 24, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in_ddsiEncapsulationHeader.h"

/* **** implementation headers **** */

/* **** private functions **** */

/* **** public functions **** */

/** return scans the header and returns number of parsed octets, otherwise -1*/
in_long
in_ddsiEncapsulationHeaderInit(
        in_ddsiEncapsulationHeader _this,
        in_octet* buffer,
        os_size_t bufferLength)
{
    in_long result = -1;
    in_ddsiCodecId codecId;

    if (bufferLength<IN_DDSI_ENCAPSULATION_HEADER_SIZE) {
        result = -1;
    } else {
        /* IN_DDSI_ENCAPSULATION_HEADER_SIZE == 4, no alignment constraint */
        _this->header[0] = buffer[0];
        _this->header[1] = buffer[1];
        _this->header[2] = buffer[2];
        _this->header[3] = buffer[3];

        codecId = (buffer[0] << 8) | buffer[1];

        switch (codecId) {
        case IN_CODEC_CDR_BE:
        case IN_CODEC_CDR_LE:
        case IN_CODEC_PL_CDR_BE:
        case IN_CODEC_PL_CDR_LE:
            result = IN_DDSI_ENCAPSULATION_HEADER_SIZE;
            break;
        default:
            result = -1;
        }
    }

    return result;
}

