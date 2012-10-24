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
/*
 * in_transportPairFactory.c
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

/* interfaces */
#include "in_transportPairFactory.h"
#include "in_transportPairIBasic.h"

in_transportPair
in_transportPairFactoryCreate(const in_configChannel configChannel)
{
	in_transportPair result;
	result = in_transportPair(in_transportPairIBasicNew(configChannel));
	return result;
}
