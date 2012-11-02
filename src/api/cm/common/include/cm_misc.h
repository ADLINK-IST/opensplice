/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/**@file api/cm/common/include/cm_misc.h
 * 
 * @brief Contains additional shared functions that do not belang in the cm_api.
*/
#ifndef CM_MISC_H
#define CM_MISC_H

#include "cm_typebase.h"
#include "u_user.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**@brief Converts user result to control & monitoring result.
 * 
 * @param ur The user result to convert.
 * @return The cm_result representation of the user result.
 */
cm_result       cm_convertResult    (u_result ur);

/**@brief Converts a control & monitoring result to an integer.
 * 
 * @param cmr The cm_result to convert.
 * @result The integer representation of the cm_result.
 */
int             cm_resultIntValue   (cm_result cmr);

#if defined (__cplusplus)
}
#endif

#endif /* CM_MISC_H */
