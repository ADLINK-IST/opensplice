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
#ifndef IN__LOCATOR_H_
#define IN__LOCATOR_H_

/* interfaces */
#include "in_locator.h"

/* attributes */
#include "in_ddsiElements.h"
#include "in__object.h"

/** \brief Class inherits from  in_object */
OS_STRUCT(in_locator)
{
    /* refcounter and destroy operation */
    OS_EXTENDS(in_object);
	OS_STRUCT(in_ddsiLocator) value;
};


#endif /* IN__LOCATOR_H_ */
