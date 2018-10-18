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
