/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

/************************************************************************
 * LOGICAL_NAME:    CheckStatus.h
 * FUNCTION:        Implementation of Basic error handling functions for OpenSplice API.
 * MODULE:          Examples for the C programming language.
 * DATE             September 2010.
 ***********************************************************************/
#ifndef __CHECKSTATUS_H__
#define __CHECKSTATUS_H__

#include "dds_dcps.h"
#include <stdio.h>
#include <stdlib.h>

/* Array to hold the names for all ReturnCodes. */
char *RetCodeName[13];

/**
 * Returns the name of an error code.
 **/
char *getErrorName(DDS_ReturnCode_t status);

/**
 * Check the return status for errors. If there is an error, then terminate.
 **/
void checkStatus(DDS_ReturnCode_t status, const char *info);

/**
 * Check whether a valid handle has been returned. If not, then terminate.
 **/
void checkHandle(void *handle, const char *info);

#endif
