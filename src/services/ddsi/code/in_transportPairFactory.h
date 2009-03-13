/*
 * in_transportPairFactory.h
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

#ifndef IN_TRANSPORTPAIRFACTORY_H_
#define IN_TRANSPORTPAIRFACTORY_H_

#include "in_transportPair.h"
#include "in__configChannel.h"
/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif

in_transportPair
in_transportPairFactoryCreate(const in_configChannel configChannel);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif



#endif /* IN_TRANSPORTPAIRFACTORY_H_ */
