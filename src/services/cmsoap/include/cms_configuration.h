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

/**
 * @file services/cmsoap/include/cms_configuration.h
 * 
 * Represents the configuration of the cmsoap service. 
 */
#ifndef CMS_CONFIGURATION_H
#define CMS_CONFIGURATION_H

#include "cms__typebase.h"
#include "u_cfElement.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define cms_configuration(a) ((cms_configuration)(a))

/**
 * Initializes the configuration for the supplied service by reading 
 * the configuration for the user layer service that is associated with the
 * supplied service. If no configuration can be found, the default is applied.
 * 
 * @param cms The cmsoap service.
 * @return TRUE if the configuration was applied successfully, FALSE otherwise.
 */
c_bool cms_configurationNew (const c_char* name, cms_service cms);

/**
 * Frees the supplied configuration.
 * 
 * @param config The configuration to free.
 */
void    cms_configurationFree   (cms_configuration config);

/**
 * Creates a formatted string representation of the supplied configuration.
 * 
 * @param config The configuration to create a formatted string representation
 *               of.
 * @return The formatted string representation of the supplied configuration.
 */
c_char* cms_configurationFormat (cms_configuration config);

#endif
