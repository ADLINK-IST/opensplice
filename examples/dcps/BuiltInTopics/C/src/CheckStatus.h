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
void checkHandle(void *handle, char *info);

#endif
