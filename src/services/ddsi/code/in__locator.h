/*
 * in__locator.h
 *
 *  Created on: Feb 18, 2009
 *      Author: frehberg
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
