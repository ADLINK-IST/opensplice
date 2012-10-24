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

/**@file api/cm/c/include/cmc_typebase.h
 * 
 * @brief Contains the used classes of the C C&M API.
 */
#ifndef CMC_TYPEBASE_H
#define CMC_TYPEBASE_H

#include "cm_typebase.h"
#include "c_metabase.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_CLASS(cmc_entity);
C_CLASS(cmc_entitySet);

/*FUTURE CLASSES

C_CLASS(cmc_qos);
C_CLASS(cmc_status);

C_STRUCT(cmc_qos){
    C_EXTENDS(cm_qos);
    v_qos qos;
};

C_STRUCT(cmc_status){
    C_EXTENDS(cm_status);
    v_status status;
};
*/

#if defined (__cplusplus)
}
#endif

#endif /* CM_TYPEBASE_H */



