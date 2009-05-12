/*
 * in_ddsiParameter.c
 *
 *  Created on: Mar 9, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in_ddsiParameter.h"
#include "in__ddsiParameter.h"
/* **** implementation headers **** */
#include "in_ddsiElements.h"
#include "in_ddsiDefinitions.h"
#include "in_report.h"

/* **** private functions **** */

/* **** public functions **** */

in_long
in_ddsiParameterHeaderInitFromBuffer(
    in_ddsiParameterHeader _this,
	in_ddsiDeserializer deserializer)
{
	in_long nofOctets = 0;
	in_long total = 0;
	in_long result = -1; /* error */
	os_ushort lowerTwoBits;

	do {
		nofOctets =
			in_ddsiDeserializerAlign(deserializer, IN_DDSI_PARAMETER_HEADER_ALIGNMENT);

		if (nofOctets < 0) break;
		total += nofOctets;
		if (nofOctets > 0) {
			IN_REPORT_WARNING("in_ddsiParameterHeaderInitFromBuffer", "unexpected deserializer alignment");
		}

		nofOctets =
			in_ddsiParameterIdInitFromBuffer(&(_this->id), deserializer);

		assert(nofOctets >= 0);
		if (nofOctets < 0) break;
		total += nofOctets;

		if(_this->id.value != IN_PID_SENTINEL)
		{

            nofOctets =
                in_ddsiDeserializerParseUshort(deserializer, &(_this->octetsToNextParameter));
            assert(nofOctets >= 0);
            if (nofOctets < 0) break;
            total += nofOctets;

            lowerTwoBits =
                (_this->octetsToNextParameter & ((IN_DDSI_PARAMETER_HEADER_ALIGNMENT)-1));

            /* length must be multiple of IN_DDSI_PARAMETER_HEADER_ALIGNMENT */
            if (lowerTwoBits > 0) {
                /* Error, lower 2 bits are not 0, length encoding does not
                 * meet preconditions */
            	assert(FALSE);
                break; /* result = -1;*/
            }
            /* all succeeeded, assign result */
        } else
        {
            nofOctets = in_ddsiDeserializerAlign(deserializer, IN_DDSI_PARAMETER_HEADER_ALIGNMENT);
            assert(nofOctets >= 0);
            if (nofOctets < 0) break;
            total += nofOctets;
        }
		result = total;
	} while (0);

	assert(result != -1);

	return result;
}



