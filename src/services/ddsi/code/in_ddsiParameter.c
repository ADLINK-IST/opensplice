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
in_ddsiParameterHeaderInitFromBuffer(in_ddsiParameterHeader _this,
		in_ddsiDeserializer deserializer)
{
	in_long nofOctets = 0;
	in_long total = 0;
	in_long result = -1; /* error */

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
		if (nofOctets < 0) break;
		total += nofOctets;

		nofOctets =
			in_ddsiDeserializerParseUshort(deserializer, &(_this->octetsToNextParameter));
		if (nofOctets < 0) break;
		total += nofOctets;

		/* length must be multiple of IN_DDSI_PARAMETER_HEADER_ALIGNMENT */
		if (_this->octetsToNextParameter & ((IN_DDSI_PARAMETER_HEADER_ALIGNMENT)-1) > 0) {
			/* Errior, lower 2 bits are not 0, length encoding does not
			 * meet preconditions */
			break; /* result = -1;*/
		}
		/* all succeeeded, assign result */
		result = total;
	} while (0);

	return result;
}



